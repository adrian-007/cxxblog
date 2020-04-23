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

#include "AvatarResource.h"
#include "AvatarGenerator.h"

#include "models/BasicSession.h"
#include "models/Editor.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

AvatarResource::AvatarResource(dbo::SqlConnectionPool& connectionPool)
    : _connectionPool(connectionPool)
    , _unknownAvatar { AvatarGenerator(128.0).generate("") }
{
}

AvatarResource::~AvatarResource()
{
    beingDeleted();
}

void AvatarResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
    try
    {
        auto handle = request.urlParam("handle");

        if (handle.empty())
            throw std::runtime_error("Missing handle argument");

        BasicSession session { _connectionPool };
        dbo::Transaction t { session };

        auto editor = session.find<Editor>().where("handle = ?").bind(handle).resultValue();

        if (!editor)
            throw std::runtime_error("Editor with given handle doesn't exist");

        const auto& avatarBytes = editor->avatar;

        if (avatarBytes.empty())
            throw std::runtime_error("Editor doesn't have avatar set");

        response.setMimeType("image/png");
        response.setStatus(200);
        response.setContentLength(avatarBytes.size());
        response.addHeader("Cache-Control", "max-age=86400");
        response.out().write(reinterpret_cast<const char*>(&avatarBytes[0]), avatarBytes.size());
    }
    catch (const std::exception& e)
    {
        response.setMimeType("image/png");
        response.setStatus(200);
        response.setContentLength(_unknownAvatar.size());
        response.addHeader("Cache-Control", "max-age=300");
        response.out().write(reinterpret_cast<const char*>(&_unknownAvatar[0]), _unknownAvatar.size());
    }
}
