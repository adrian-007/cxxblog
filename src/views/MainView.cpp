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

#include "MainView.h"

#include "SettingsView.h"
#include "PostsListView.h"
#include "PostView.h"
#include "EditorView.h"
#include "Markdown.h"

#include <Wt/WApplication.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WBorderLayout.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMenu.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WLogger.h>
#include <Wt/WStackedWidget.h>

#define LOG() wApp->log("MainView")

MainView::MainView(Session& session, Wt::Dbo::SqlConnectionPool& dbConnectionPool)
    : _session(session)
{
    auto app = wApp;

    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "xml/views");
    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "xml/strings");

    app->useStyleSheet("assets/css/highlight/default.min.css");
    app->useStyleSheet("assets/css/highlight/vs.min.css");
    app->useStyleSheet("assets/css/cxxblog.css");
    app->useStyleSheet("assets/font-awesome/css/all.min.css");

    app->require("assets/js/highlight.min.js");
    app->require("assets/js/cxxblog.js");

    if (auto disqusShortname = session.siteConfig().disqusShortname(); !disqusShortname.empty())
    {
        // Create a stub function, we'll be using DISQUS.reset anyway.
        app->doJavaScript("var disqus_config = function () { };");
        app->require("//" + disqusShortname + ".disqus.com/embed.js");
    }

    app->internalPathChanged().connect(this, &MainView::onInternalPathChanged);

    auto view = std::make_unique<Wt::WTemplate>(tr("mainView"));

    view->bindEmpty("content");
    view->bindString("footer", Markdown(_session.siteConfig().footer()).renderHTML());

    {
        _navBar = view->bindNew<Wt::WNavigationBar>("navbar");
        _navBar->addStyleClass("navbar-fixed-top");

        _navBar->setTitle(_session.siteConfig().siteName(), Wt::WLink(Wt::LinkType::InternalPath, _session.basePath()));
        _navBar->setResponsive(true);

        _navigationMenu = _navBar->addMenu(createNavigationMenu(false), Wt::AlignmentFlag::Right);
    }

    _view = addWidget(std::move(view));

    _session.login().changed().connect(this, &MainView::onLoginStateChanged);

    // This is a ugly hack to actually get a widget process auth tokens.
    _session.createAuthWidget()->processEnvironment();

    onInternalPathChanged(app->internalPath());
}

void MainView::onLoginStateChanged()
{
    auto app = wApp;

    dbo::Transaction t { _session };

    auto editor = _session.editor();
    auto loggedIn = (bool)editor;

    if (_navigationMenu != nullptr)
        _navigationMenu->removeFromParent();

    _navigationMenu = _navBar->addMenu(createNavigationMenu(loggedIn), Wt::AlignmentFlag::Right);

    bool navigateToBasePath = false;

    if (loggedIn)
    {
        if (loggedIn != _previousLoginState)
        {
            LOG() << "User logged in: " << editor->name;
            app->changeSessionId();
            auto nextPart { app->internalPathNextPart(_session.basePath()) };

            if (nextPart == loginPath())
                navigateToBasePath = true;
        }
    }
    else
    {
        LOG() << "User logged out.";
        app->changeSessionId();
        navigateToBasePath = true;
    }

    if (navigateToBasePath)
    {
        app->setInternalPath(_session.basePath(), false);

        // Force redraw. When internal path is already a base path, signal is not emitted, so simply calling 'setInternalPath' won't work in that case.
        onInternalPathChanged(_session.basePath());
    }

    _previousLoginState = loggedIn;
}

void MainView::onInternalPathChanged(const std::string& internalPath)
{
    assert(_view != nullptr);

    auto app = Wt::WApplication::instance();

    if (!app->internalPathMatches(_session.basePath()))
        return;

    safeCall([&]
    {
        auto path = app->internalPathNextPart(_session.basePath());

        if (path == loginPath())
        {
            bindContent(tr("str.login"), _session.createAuthWidget());
        }
        else if (path == "post")
        {
            path = app->internalPathNextPart(_session.basePath() + path + '/');

            dbo::Transaction t { _session };

            auto query = _session.find<Post>();
            query.where("id = ?").bind(path);

            if (!_session.login().loggedIn())
            {
                query.where("visibility = ?").bind(Post::Visibility::Published);
            }

            auto post = query.resultValue();

            if (!post)
                throw PageNotFoundException();

            if (_navigationMenu != nullptr)
                _navigationMenu->select(nullptr);

            auto title = post->title;
            bindContent(title, PostView::createNew(_session, std::move(post)));
        }
        else if (path == "people")
        {
            auto editorHandle = app->internalPathNextPart(_session.basePath() + path + '/');

            dbo::Transaction t { _session };

            auto editor = _session.find<Editor>().where("handle = ?").bind(editorHandle).resultValue();

            if (!editor)
                throw PageNotFoundException();

            if (_navigationMenu != nullptr)
                _navigationMenu->select(nullptr);

            auto title = editor->name;
            bindContent(title, EditorView::createNew(_session, std::move(editor)));
        }
        else
        {
            if (_navigationMenu != nullptr)
            {
                // WMenu only manages automatic changes when there is a Wt::WStackedWidget attached to it. Otherwise
                // internal path changes does not trigger item (de)activation.

                if (!path.empty())
                {
                    auto currentItem = _navigationMenu->currentItem();
                    for (auto item : _navigationMenu->items())
                    {
                        const auto& link = item->link();
                        if (currentItem != item && item->pathComponent() == path)
                        {
                            item->select();
                            return;
                        }
                    }
                }
                else
                {
                    _navigationMenu->select(nullptr);
                }
            }

            if (path.empty())
                bindContent({}, PostsListView::createNew(_session));
            else
                throw PageNotFoundException(internalPath);
        }
    });
}

#include <Wt/WLoadingIndicator.h>

std::unique_ptr<Wt::WMenu> MainView::createNavigationMenu(bool editor /*= false*/)
{
    auto menu = std::make_unique<Wt::WMenu>();

    // 4.2.2 - internal paths won't work when menu does not have WStackedWidget attached to it
    // but navigation text click won't work without it.
    menu->setInternalPathEnabled(_session.basePath());
    menu->setInternalBasePath(_session.basePath());

    auto addMenuItem = [&menu, this](const auto textId, const std::string& path, auto callback)
    {
        auto item = menu->addItem(tr(textId));

        item->setPathComponent(path);
        item->setInternalPathEnabled(false);

        item->triggered().connect(this, [=]
        {
            safeCall([=]
            {
                wApp->setInternalPath(_session.basePath() + item->pathComponent());
                callback(item);
            });
        });
    };

    addMenuItem("str.about", "about", [=](auto item)
    {
        auto content = Markdown(_session.siteConfig().about()).renderHTML();
        bindStringContent(item->text(), content);
    });

    if (!editor)
    {
        return menu;
    }

    // Those are additional buttons accessible only after successful login.
    addMenuItem("str.createPost", "create-post", [=](auto item)
    {
        dbo::Transaction t { _session };
        _session.validateEditorLoginState();

        bindContent(item->text(), PostView::createNew(_session, dbo::ptr<Post> {}));
    });

#if 0 // TODO
    addMenuItem("str.job", "job", [=](auto item)
    {
        dbo::Transaction t { _session };
        _session.validateEditorLoginState();

        bindContent(item->text(), JobOffersView::createNew(_session));
    });
#endif

    addMenuItem("str.settings", "settings", [=](auto item)
    {
        dbo::Transaction t { _session };
        _session.validateEditorLoginState();

        bindNewContent<SettingsView>(item->text(), _session);
    });

    addMenuItem("str.logout", {}, [=](auto item)
    {
        _session.createAuthWidget()->model()->logout(_session.login());
    });

    return menu;
}

void MainView::setPageMessage(const std::string& message)
{
    assert(_view != nullptr);

    wApp->setTitle(_session.siteConfig().siteName() + " - " + message);
    auto view =_view->bindNew<Wt::WTemplate>("content", tr("mainView.errorMessage"));
    view->bindString("errorMessage", message);
}

void MainView::bindContent(const Wt::WString& title, std::unique_ptr<Wt::WWidget> contentWidget)
{
    wApp->setTitle(title.empty() ? _session.siteConfig().siteName() : tr("str.pageSubtitle").arg(_session.siteConfig().siteName()).arg(title));
    _view->bindWidget("content", std::move(contentWidget));
}

void MainView::bindStringContent(const Wt::WString& title, const Wt::WString& text)
{
    wApp->setTitle(title.empty() ? _session.siteConfig().siteName() : tr("str.pageSubtitle").arg(_session.siteConfig().siteName()).arg(title));
    _view->bindString("content", text);
}

void MainView::safeCall(const std::function<void()>& fn) noexcept
{
    try
    {
        fn();
    }
    catch (const ApplicationException& e)
    {
        setPageMessage(e.what());
    }
    catch (const std::exception& e)
    {
        wApp->log("critical") << "Unexpected error: " << e.what();
        setPageMessage("Unexpected Error.");
    }
}

std::string MainView::loginPath() const
{
    std::string loginPath;
    if (!wApp->readConfigurationProperty("loginPath", loginPath))
        loginPath = "login";

    return loginPath;
}
