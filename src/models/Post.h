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

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <Wt/WString.h>
#include <Wt/WDateTime.h>

#include <vector>

#include "PostDraft.h"

namespace dbo = Wt::Dbo;

class Editor;

class Post
    : public dbo::Dbo<Post>
{
public:
    enum class Visibility
    {
        Hidden,
        Published
    };

    dbo::collection<dbo::ptr<PostDraft>> drafts;

    dbo::ptr<Editor> author;
    Wt::WDateTime created;
    Wt::WDateTime published;

    Visibility visibility;

    Wt::WString title;
    Wt::WString intro;
    Wt::WString content;

    template<class Action>
    void persist(Action& a)
    {
        dbo::hasMany(a, drafts, dbo::ManyToOne, "post");
        dbo::belongsTo(a, author, "editor", dbo::NotNull | dbo::OnDeleteCascade | dbo::OnUpdateCascade);

        dbo::field(a, created, "created");
        dbo::field(a, published, "published");
        dbo::field(a, visibility, "visibility");
        dbo::field(a, title, "title");
        dbo::field(a, intro, "intro");
        dbo::field(a, content, "content");
    }

    [[nodiscard]] std::vector<dbo::ptr<PostDraft>> latestDrafts() const;
    [[nodiscard]] std::string url() const;

private:
    [[nodiscard]] std::string titleAsFriendlyURL() const;
};

DBO_EXTERN_TEMPLATES(Post)
