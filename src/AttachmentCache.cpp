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

#include "AttachmentCache.h"

#include <Wt/WApplication.h>

#include <iterator>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

AttachmentCache& AttachmentCache::instance()
{
    static AttachmentCache i;
    return i;
}

void AttachmentCache::invalidate() const
{
    auto path { cachePath() };

    if (fs::exists(path))
        fs::remove_all(path);

    fs::create_directories(path);
}

std::string AttachmentCache::cachePath() const
{
    return Wt::WApplication::appRoot() + "cache" + fs::path::preferred_separator + "attachments";
}

void AttachmentCache::set(const std::string& id, const std::vector<uint8_t>& data, const std::string& mimeType)
{
    try
    {
        std::ofstream s { cachePath() + fs::path::preferred_separator + id, std::ios::out | std::ios::binary };
        std::ostream_iterator<uint8_t> it { s };
        std::copy(data.begin(), data.end(), it);

        std::scoped_lock<std::mutex> lock { _mutex };
        _idMimeTypeMap[id] = mimeType;
    }
    catch (const std::exception&)
    {
    }
}

std::pair<std::string, std::vector<uint8_t>> AttachmentCache::get(const std::string& id) const
{
    try
    {
        auto path = cachePath() + fs::path::preferred_separator + id;

        if (fs::is_regular_file(path))
        {
            std::ifstream s { path, std::ios::binary };
            s.unsetf(std::ios::skipws);

            std::vector<uint8_t> data { std::istream_iterator<char> { s }, std::istream_iterator<char> { }};

            std::scoped_lock<std::mutex> lock { _mutex };
            if (auto it = _idMimeTypeMap.find(id); it != _idMimeTypeMap.end())
                return std::make_pair(it->second, std::move(data));
        }
    }
    catch (const std::exception&)
    {
    }

    return std::make_pair(std::string{}, std::vector<uint8_t>{});
}
