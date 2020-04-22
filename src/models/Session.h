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

#pragma once

#include <Wt/Auth/Login.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/AbstractPasswordService.h>

#include <Wt/Dbo/ptr.h>

#include <memory>
#include "BasicSession.h"

#include "SiteConfig.h"
#include "ApplicationExceptions.h"
#include "Editor.h"

namespace dbo = Wt::Dbo;

struct DBConnectionInfo
{
    std::string dbType;
    std::string dbHost;
    std::string dbPort;
    std::string dbUsername;
    std::string dbPassword;
    std::string dbName;
};

class Session
    : public BasicSession
{
public:
    Session(const std::string& basePath, Wt::Dbo::SqlConnectionPool& dbConnectionPool);

    std::string relativePath(const std::string& path) const;
    std::string relativePath(const std::initializer_list<std::string>& parts) const;

    inline const auto& basePath() const { return _basePath; }
    inline auto& siteConfig() { return _siteConfig; }

    inline auto& login() { return _login; }
    inline auto& users() { return _users; }

    dbo::ptr<Editor> editor();

    Wt::Auth::PasswordResult updateCredentials(const Wt::Auth::Login& login, const Wt::WString& currentPassword, const Wt::WString& newUsername, const Wt::WString& newPassword);

    /**
     * Throws AccessDeniedException if user is not logged in as editor.
     */
    void validateEditorLoginState();

    std::unique_ptr<Wt::Auth::AuthWidget> createAuthWidget();

    static std::unique_ptr<Wt::Dbo::SqlConnectionPool> createConnectionPool(DBConnectionInfo info);
    static void initAuthServices();

private:
    static Wt::Auth::AuthService& authService();
    static Wt::Auth::PasswordService& passwordService();

    const std::string& _basePath;
    SiteConfig _siteConfig;

    Wt::Auth::Dbo::UserDatabase<EditorAuthInfo> _users;
    Wt::Auth::Login _login;
};
