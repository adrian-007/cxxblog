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

#include <Wt/WResource.h>
#include <Wt/Dbo/SqlConnectionPool.h>

namespace dbo = Wt::Dbo;

class AttachmentResource
    : public Wt::WResource
{
public:
    explicit AttachmentResource(dbo::SqlConnectionPool& connectionPool);
    ~AttachmentResource() override;

private:
    void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;
    void respondWithData(const Wt::Http::Request& request, Wt::Http::Response& response, const std::string& mimeType, const std::vector<uint8_t>& data) const;

    dbo::SqlConnectionPool& _connectionPool;
};
