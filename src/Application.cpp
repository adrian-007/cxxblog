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

#include "Application.h"

#include <Wt/Auth/Identity.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WServer.h>

#include "views/MainView.h"
#include "views/LoadingWidget.h"

Application::Application(const std::string& basePath, const Wt::WEnvironment& env, Wt::Dbo::SqlConnectionPool& dbConnectionPool)
    : Wt::WApplication(env)
    , _session(basePath, dbConnectionPool)
{
    auto theme = std::make_shared<Wt::WBootstrapTheme>();
    theme->setVersion(Wt::BootstrapVersion::v3);
    setTheme(theme);

    setLoadingIndicator(std::make_unique<LoadingWidget>());

    root()->addStyleClass("container");
    root()->addNew<MainView>(_session, dbConnectionPool);
}
