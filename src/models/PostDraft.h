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

namespace dbo = Wt::Dbo;

class Post;

class PostDraft
    : public dbo::Dbo<PostDraft>
{
public:
    PostDraft();
    explicit PostDraft(dbo::ptr<Post> parent);
    explicit PostDraft(const dbo::ptr<PostDraft>& draft);

    dbo::ptr<Post> post;
    Wt::WDateTime created;

    Wt::WString title;
    Wt::WString intro;
    Wt::WString content;

    template<class Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, post, "post", dbo::NotNull | dbo::OnDeleteCascade | dbo::OnUpdateCascade);
        dbo::field(a, created, "created");

        dbo::field(a, title, "title");
        dbo::field(a, intro, "intro");
        dbo::field(a, content, "content");
    }
};

DBO_EXTERN_TEMPLATES(PostDraft)
