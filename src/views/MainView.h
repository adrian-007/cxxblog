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

#include "models/Session.h"

#include <Wt/WGlobal.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

#include <functional>
#include <vector>
#include <memory>
#include <functional>

class MainView
    : public Wt::WContainerWidget
{
public:
    MainView(Session& session, Wt::Dbo::SqlConnectionPool& dbConnectionPool);

private:
    void onLoginStateChanged();
    void onInternalPathChanged(const std::string& path);
    std::unique_ptr<Wt::WMenu> createNavigationMenu(bool editor = false);

    void setPageMessage(const std::string& message);
    void safeCall(const std::function<void()>& fn) noexcept;

    void bindContent(const Wt::WString& title, std::unique_ptr<Wt::WWidget> contentWidget);
    void bindStringContent(const Wt::WString& title, const Wt::WString& text);

    template<typename T, typename...A>
    T* bindNewContent(const Wt::WString& title, A&&...a)
    {
        auto widget = std::make_unique<T>(std::forward<A>(a)...);
        auto ptr = widget.get();
        bindContent(title, std::move(widget));
        return ptr;
    }

    std::string loginPath() const;

    Session& _session;
    bool _previousLoginState = false;

    Wt::WTemplate* _view = nullptr;
    Wt::WNavigationBar* _navBar = nullptr;
    Wt::WMenu* _navigationMenu = nullptr;
};
