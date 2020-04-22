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

#include "AttachmentResource.h"
#include "ApplicationExceptions.h"
#include "AttachmentCache.h"

#include "models/BasicSession.h"
#include "models/Attachment.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

AttachmentResource::AttachmentResource(dbo::SqlConnectionPool& connectionPool)
    : _connectionPool(connectionPool)
{
}

AttachmentResource::~AttachmentResource()
{
    beingDeleted();
}

void AttachmentResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
    try
    {
        auto id = request.urlParam("id");

        if (id.empty() || id.find_first_not_of("0123456789") != std::string::npos)
            throw HTTPStatusException(404);

        auto cacheData { AttachmentCache::instance().get(id) };
        if (!cacheData.first.empty() && !cacheData.second.empty())
        {
            respondWithData(request, response, cacheData.first, cacheData.second);
        }
        else
        {
            BasicSession session { _connectionPool };
            dbo::Transaction t { session };

            auto attachment = session.find<Attachment>().where("id = ?").bind(id).resultValue();

            if (!attachment)
                throw HTTPStatusException(404);

            const auto& data { attachment->data };

            if (data.empty())
                throw HTTPStatusException(500);

            auto mimeType { attachment->mimeType.toUTF8() };

            respondWithData(request, response, attachment->mimeType.toUTF8(), data);

            AttachmentCache::instance().set(id, data, mimeType);
        }
    }
    catch (const HTTPStatusException& e)
    {
        response.setStatus(e.status());
    }
    catch (const std::exception& e)
    {
        response.setStatus(500);
    }
}

void AttachmentResource::respondWithData(const Wt::Http::Request& request, Wt::Http::Response& response, const std::string& mimeType, const std::vector<uint8_t>& data) const
{
    response.setMimeType(mimeType);
    response.setStatus(200);
    response.setContentLength(data.size());
    response.addHeader("Cache-Control", "private, max-age=3600");
    response.out().write(reinterpret_cast<const char*>(&data[0]), data.size());
}
