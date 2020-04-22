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

#include <mutex>
#include <map>
#include <vector>
#include <string>

namespace dbo = Wt::Dbo;

class AttachmentIconResource
    : public Wt::WResource
{
public:
    explicit AttachmentIconResource(dbo::SqlConnectionPool& connectionPool);
    ~AttachmentIconResource() override;

private:
    void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) override;

    std::vector<uint8_t> iconDataFromFileType(const std::string& fileExtension, const std::string& mimeType) const;
    const std::vector<uint8_t>& iconDataForFileType(const std::string& fileExtension, const std::string& mimeType);

    dbo::SqlConnectionPool& _connectionPool;

    const std::vector<uint8_t> _unknowFileTypeIconData;
    std::map<std::string, std::vector<uint8_t>> _extensionIconDataMap;
    std::mutex _extensionIconDataMapMutex;
};
