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

#include "Post.h"
#include "PostDraft.h"
#include "Editor.h"

#include <Wt/Dbo/Dbo.h>
#include <algorithm>
#include <locale>

namespace
{
    const auto g_MaxPostDraftsCount = 3u;
}

DBO_INSTANTIATE_TEMPLATES(Post)

std::vector<dbo::ptr<PostDraft>> Post::latestDrafts() const
{
    std::vector<dbo::ptr<PostDraft>> latestDrafts;

    auto postDrafts = drafts.find().orderBy("id desc").resultList();
    auto it = postDrafts.begin();
    auto n = 0u;

    // Unfortunately, std::advance does not work with Wt iterator, for some reason. Advance it manually.
    while (++n <= g_MaxPostDraftsCount && it != postDrafts.end())
    {
        latestDrafts.emplace_back(std::move(*it));
        ++it;
    }

    // Remove everything past the maximal draft count limit.
    while (it != postDrafts.end())
    {
        auto draft = *it;
        ++it;
        draft.remove();
    }

    return latestDrafts;
}

std::string Post::url() const
{
    return Wt::WString("post/{1}{2}")
        .arg(id())
        .arg(titleAsFriendlyURL())
        .toUTF8();
}

std::string Post::titleAsFriendlyURL() const
{
    auto titleString { title.toUTF16() };
    std::string result;

    std::locale loc("");
    auto& facet = std::use_facet<std::ctype<wchar_t>>(loc);

    bool dashInserted = false;
    for (auto c : titleString)
    {
        if ((c >= u'A' && c <= u'Z') || (c >= u'a' && c <= u'z') || (c >= u'0' && c <= u'9'))
        {
            result.push_back(facet.narrow(facet.tolower(c), '-'));
            dashInserted = false;
        }
        else if (!dashInserted)
        {
            result.push_back('-');
            dashInserted = true;
        }
    }

    while (!result.empty() && result[result.size() - 1] == '-')
        result.erase(result.size() - 1, 1);

    if (!result.empty())
        return "/" + result;

    return result;
}
