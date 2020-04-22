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

#include <Wt/WServer.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include "Application.h"

#include "AvatarResource.h"
#include "AttachmentCache.h"
#include "AttachmentResource.h"
#include "AttachmentIconResource.h"
#include "Markdown.h"

int main(int argc, char **argv)
{
    Markdown::init();

    try
    {
        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

        std::string loginPath;
        if (server.readConfigurationProperty("loginPath", loginPath))
        {
            if (loginPath.size() < 5)
                throw std::runtime_error("Login path is too short");

            for (const auto c : loginPath)
            {
                if (c == '-')
                    continue;

                if (std::isalnum(c))
                    continue;

                throw std::runtime_error("Login path contains forbidden characters. It can only contain alphanumeric characters and must have at least 5 characters.");
            }
        }

        DBConnectionInfo dbConnectionInfo;
        if (!server.readConfigurationProperty("dbType", dbConnectionInfo.dbType) || dbConnectionInfo.dbType.empty())
            dbConnectionInfo.dbType = "sqlite";

        if (dbConnectionInfo.dbType == "sqlite")
        {
            dbConnectionInfo.dbName = server.appRoot() + "database.sq3";
        }
        else
        {
            server.readConfigurationProperty("dbName", dbConnectionInfo.dbName);
            server.readConfigurationProperty("dbUsername", dbConnectionInfo.dbUsername);
            server.readConfigurationProperty("dbPassword", dbConnectionInfo.dbPassword);
            server.readConfigurationProperty("dbHost", dbConnectionInfo.dbHost);
            server.readConfigurationProperty("dbPort", dbConnectionInfo.dbPort);
        }

        AttachmentCache::instance().invalidate();
        Session::initAuthServices();

        auto dbConnectionPool = Session::createConnectionPool(std::move(dbConnectionInfo));
        const std::string basePath = "/";

        // Register avatar stateless resource.
        AvatarResource avatarResource { *dbConnectionPool };
        server.addResource(&avatarResource, basePath + "avatar/${handle}");

        // Register attachment preview stateless resource.
        AttachmentIconResource attachmentPreviewResource { *dbConnectionPool };
        server.addResource(&attachmentPreviewResource, basePath + "attachment-icon/${id}");

        // Register attachment stateless resource.
        AttachmentResource attachmentResource { *dbConnectionPool };
        server.addResource(&attachmentResource, basePath + "attachment/${id}");

        // Register entry point for the application.
        server.addEntryPoint(Wt::EntryPointType::Application, [&](const Wt::WEnvironment& env)
        {
            return std::make_unique<Application>(basePath, env, *dbConnectionPool);
        });

        server.run();
        return 0;
    }
    catch (const Wt::WServer::Exception& e)
    {
        std::cerr << "Server exception: " << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unknown exception: " << e.what() << std::endl;
        return -1;
    }
}
