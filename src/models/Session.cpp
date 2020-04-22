/*
 * Copyright (C) 2020 adrian_007, adrian-007 on o2 point pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "Session.h"

#include <Wt/WApplication.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>
#include <Wt/Auth/PasswordStrengthValidator.h>
#include <Wt/Auth/HashFunction.h>
#include <Wt/Auth/Identity.h>

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>

#if __has_include(<Wt/Dbo/backend/Sqlite3.h>)
#include <Wt/Dbo/backend/Sqlite3.h>
#define WT_DBO_SUPPORTS_SQLITE
#endif

#if __has_include(<Wt/Dbo/backend/Postgres.h>)
#include <Wt/Dbo/backend/Postgres.h>
#define WT_DBO_SUPPORTS_POSTGRES
#endif

#if __has_include(<Wt/Dbo/backend/MySQL.h>)
#include <Wt/Dbo/backend/MySQL.h>
#define WT_DBO_SUPPORTS_MYSQL
#endif

#include <atomic>
#include <numeric>

#include "Post.h"
#include "AvatarGenerator.h"
#include "ConfigStore.h"

namespace
{
    std::atomic<bool> g_bDatabaseInitialized { false };
}

Session::Session(const std::string& basePath, Wt::Dbo::SqlConnectionPool& dbConnectionPool)
    : BasicSession(dbConnectionPool)
    , _basePath { basePath }
    , _users { *this }
{
    auto tryQuery = [this](auto fn)
    {
        try
        {
            dbo::Transaction t { *this };
            fn();
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to try query: " << e.what() << std::endl;
            return false;
        }
    };

    if (!g_bDatabaseInitialized)
    {
        try
        {
            if (tryQuery([=]
            {
                execute("select count(*) from editor").run();
            }))
            {
                std::cerr << "Database already exists, running migrations." << std::endl;

                // Here is a place for running migration scripts. Each migration should be a separate
                // transaction, so it's advised to use tryQuery.

                std::cerr << "Migrations completed." << std::endl;
            }
            else
            {
                dbo::Transaction t { *this };

                createTables();

                // Create unique index on editor table, since there is no option to create unique constraint in Wt::Dbo.
                tryQuery([=]
                {
                    execute("create unique index editor_handle_index on editor(handle);");
                });

                // Register a default user account.
                auto user = _users.registerNew();

                // Set login as identity.
                user.addIdentity(Wt::Auth::Identity::LoginName, "admin");

                // Activate the first user.
                user.setStatus(Wt::Auth::AccountStatus::Normal);
                // Set user password.
                passwordService().updatePassword(user, "admin");

                // Set up Editor object that adds additional information to the user object.
                auto editorDbo = addNew<Editor>();
                auto editor = editorDbo.modify();
                editor->name = "Administrator";
                editor->handle = "administrator";
                editor->avatar = AvatarGenerator(128.0).generate(editor->name.toUTF8());
                editor->role = Editor::Role::Admin;

                // Get a record to authInfo for given user.
                auto userAuthInfoDbo = _users.find(user);
                // Modify given record.
                auto userAuthInfo = userAuthInfoDbo.modify();
                // Associate editor object with user authInfo
                userAuthInfo->setUser(editorDbo);

                // Create first post.
                auto postDbo = addNew<Post>();
                auto post = postDbo.modify();
                post->author = editorDbo;
                post->title = "It's alive!";
                post->intro = "This is introduction to the post.";
                post->content = "Seems like this is a clean instance of your brand new CMS. Go to settings and configure it to your needs.";
                post->created = post->published = Wt::WDateTime::currentDateTime();
                post->visibility = Post::Visibility::Published;
            }
        }
        catch (const Wt::Dbo::Exception& e)
        {
            std::cerr << "Database exception: " << e.what() << std::endl << tableCreationSql() << std::endl;
            std::cerr << "Using existing database." << std::endl;
        }

        g_bDatabaseInitialized = true;
    }

    try
    {
        tryQuery([=] { addNew<ConfigStore>(ConfigKeys::SiteName, "cxxblog", ".+"); });
        tryQuery([=] { addNew<ConfigStore>(ConfigKeys::SiteAbout, "This is a new instance of cxxblog."); });
        tryQuery([=] { addNew<ConfigStore>(ConfigKeys::SiteFooter, "Powered by [cxxblog](https://github.com/adrian-007/cxxblog)"); });

        // Try to create default keys.
        dbo::Transaction t { *this };

        auto results { find<ConfigStore>().where("key like 'site.%'").resultList() };
        for (auto& cs : results)
        {
            if (cs->key == ConfigKeys::SiteName)
            {
                _siteConfig._siteName = std::move(cs);
            }
            else if (cs->key == ConfigKeys::SiteAbout)
            {
                _siteConfig._about = std::move(cs);
            }
            else if (cs->key == ConfigKeys::SiteFooter)
            {
                _siteConfig._footer = std::move(cs);
            }
        }

        assert(_siteConfig._siteName);
        assert(_siteConfig._about);
        assert(_siteConfig._footer);
    }
    catch (const std::exception& e)
    {
        wApp->log("error") << e.what();
    }
}

std::string Session::relativePath(const std::string& path) const
{
    assert(!_basePath.empty() && _basePath[_basePath.size() - 1] == '/');
    assert(path.empty() || path[0] != '/');

    return _basePath + path;
}

std::string Session::relativePath(const std::initializer_list<std::string>& parts) const
{
    assert(!_basePath.empty() && _basePath[_basePath.size() - 1] == '/');
    assert(parts.size() > 0u);

    return std::accumulate(parts.begin() + 1, parts.end(), _basePath + *parts.begin(), [](const auto& current, const auto& part)
    {
        assert(!part.empty() && part[0] != '/' && part[part.size() - 1] != '/');
        return current + '/' + part;
    });
}

dbo::ptr<Editor> Session::editor()
{
    if (_login.loggedIn())
    {
        if (auto editorAuthInfo = _users.find(_login.user()))
        {
            return editorAuthInfo->user();
        }
    }

    return dbo::ptr<Editor>();
}

Wt::Auth::PasswordResult Session::updateCredentials(const Wt::Auth::Login& login, const Wt::WString& currentPassword, const Wt::WString& newUsername, const Wt::WString& newPassword)
{
    if (!login.loggedIn())
        throw AccessDeniedException();

    auto user = _login.user();

    if (!user.isValid())
        throw std::runtime_error("Cannot find user from given auth info.");

    auto result = passwordService().verifyPassword(user, currentPassword);

    if (result == Wt::Auth::PasswordResult::PasswordValid)
    {
        if (!newPassword.empty() && currentPassword != newPassword)
            passwordService().updatePassword(user, newPassword);

        if (!newUsername.empty())
        {
            auto u = users().findWithIdentity(Wt::Auth::Identity::LoginName, newUsername);
            if (u.isValid())
                throw std::runtime_error("Given user already exists.");

            user.setIdentity(Wt::Auth::Identity::LoginName, newUsername);
        }
    }

    return result;
}

void Session::validateEditorLoginState()
{
    if (!_login.loggedIn())
        throw AccessDeniedException();

    dbo::Transaction t { *this };
    auto editorAuthInfo = _users.find(_login.user());

    if (!editorAuthInfo || !editorAuthInfo->user())
        throw AccessDeniedException();
}

std::unique_ptr<Wt::Auth::AuthWidget> Session::createAuthWidget()
{
    auto authWidget = std::make_unique<Wt::Auth::AuthWidget>(authService(), users(), login());
    authWidget->setRegistrationEnabled(false);
    authWidget->model()->addPasswordAuth(&passwordService());

    return authWidget;
}

std::unique_ptr<Wt::Dbo::SqlConnectionPool> Session::createConnectionPool(DBConnectionInfo info)
{
    using namespace Wt::Dbo;
    std::unique_ptr<SqlConnection> connection;

    if (info.dbType == "sqlite")
    {
#ifndef WT_DBO_SUPPORTS_SQLITE
        throw std::runtime_error("Wt::Dbo was not compiled with SQLite support.");
#else
        auto backend { std::make_unique<backend::Sqlite3>(info.dbName) };
        backend->setProperty("show-queries", "true");
        backend->setDateTimeStorage(SqlDateTimeType::DateTime, backend::DateTimeStorage::PseudoISO8601AsText);
        connection = std::move(backend);
#endif
    }
    else if (info.dbType == "postgres")
    {
#ifndef WT_DBO_SUPPORTS_POSTGRES
        throw std::runtime_error("Wt::Dbo was not compiled with Postgres support.");
#else
        if (info.dbName.empty())
            throw std::runtime_error("dbName property is empty");

        auto connectionString { "dbname=" + info.dbName };

        if (!info.dbHost.empty())
            connectionString += " host=" + info.dbHost;

        if (!info.dbPort.empty())
            connectionString += " port=" + info.dbPort;

        if (!info.dbUsername.empty())
            connectionString += " user=" + info.dbUsername;

        if (!info.dbPassword.empty())
            connectionString += " password=" + info.dbPassword;

        auto backend { std::make_unique<backend::Postgres>(connectionString) };
        connection = std::move(backend);
#endif
    }
    else if (info.dbType == "mysql")
    {
#ifndef WT_DBO_SUPPORTS_MYSQL
        throw std::runtime_error("Wt::Dbo was not compiled with MySQL support.");
#else
        if (info.dbName.empty())
            throw std::runtime_error("dbName property is empty");

        if (info.dbUsername.empty())
            info.dbUsername = "root";

        if (info.dbHost.empty())
            info.dbHost = "localhost";

        unsigned int port = (info.dbPort.empty() || info.dbPort.find_first_not_of("0123456789") != std::string::npos) ? 0u : std::stoi(info.dbPort);

        auto backend { std::make_unique<backend::MySQL>(info.dbName, info.dbUsername, info.dbPassword, info.dbHost, port) };
        connection = std::move(backend);
#endif
    }
    else
    {
        throw std::runtime_error("Unknown dbType property value");
    }

    assert(connection != nullptr);
    return std::make_unique<FixedSqlConnectionPool>(std::move(connection), 10);
}

void Session::initAuthServices()
{
    auto& authSrv = authService();
    auto& passwordSrv = passwordService();

    authSrv.setAuthTokensEnabled(true, "loginToken");
    authSrv.setEmailVerificationEnabled(false);
    authSrv.setEmailVerificationRequired(false);

    auto passwordVerifier = std::make_unique<Wt::Auth::PasswordVerifier>();
    passwordVerifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(8));

    passwordSrv.setVerifier(std::move(passwordVerifier));
    passwordSrv.setAttemptThrottlingEnabled(true);
    passwordSrv.setStrengthValidator(std::make_unique<Wt::Auth::PasswordStrengthValidator>());
}

Wt::Auth::AuthService& Session::authService()
{
    static Wt::Auth::AuthService service;
    return service;
}

Wt::Auth::PasswordService& Session::passwordService()
{
    static Wt::Auth::PasswordService service { authService() };
    return service;
}
