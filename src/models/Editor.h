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
#include <Wt/WGlobal.h>
#include <Wt/WString.h>
#include <Wt/WDate.h>

#include <Wt/Auth/Dbo/AuthInfo.h>

namespace dbo = Wt::Dbo;

class Editor;
using EditorAuthInfo = Wt::Auth::Dbo::AuthInfo<Editor>;

class Post;
class EditorContactDetail;
class EditorJobOffer;
class EditorResume;
class EditorEmployment;
class EditorProject;
class EditorEducation;
class EditorTechnologyExperience;

class Editor
    : public dbo::Dbo<Editor>
{
public:
    enum class Role
    {
        Admin,
        Writer
    };

    Wt::WString name;
    Wt::WString handle;
    Wt::WString aboutMe;
    std::vector<uint8_t> avatar;
    Role role;

    dbo::ptr<EditorAuthInfo> authInfo;

    dbo::collection<dbo::ptr<Post>> posts;
    dbo::collection<dbo::ptr<EditorContactDetail>> contactDetails;

    dbo::collection<dbo::ptr<EditorJobOffer>> jobOffers;
    dbo::collection<dbo::ptr<EditorResume>> resumes;
    dbo::collection<dbo::ptr<EditorEmployment>> employments;
    dbo::collection<dbo::ptr<EditorProject>> projects;
    dbo::collection<dbo::ptr<EditorEducation>> education;
    dbo::collection<dbo::ptr<EditorTechnologyExperience>> experience;

    template<class Action>
    void persist(Action& a)
    {
        if (avatar.empty())
            avatar = generateDefaultAvatar();

        dbo::field(a, name, "name");
        dbo::field(a, handle, "handle");
        dbo::field(a, aboutMe, "about_me");
        dbo::field(a, avatar, "avatar");
        dbo::field(a, role, "role");

        dbo::belongsTo(a, authInfo, "user");

        dbo::hasMany(a, posts, dbo::ManyToOne, "editor");
        dbo::hasMany(a, contactDetails, dbo::ManyToOne, "editor");
        dbo::hasMany(a, jobOffers, dbo::ManyToOne, "editor");
        dbo::hasMany(a, resumes, dbo::ManyToOne, "editor");
        dbo::hasMany(a, employments, dbo::ManyToOne, "editor");
        dbo::hasMany(a, projects, dbo::ManyToOne, "editor");
        dbo::hasMany(a, education, dbo::ManyToOne, "editor");
        dbo::hasMany(a, experience, dbo::ManyToOne, "editor");
    }

    [[nodiscard]] std::vector<uint8_t> generateDefaultAvatar() const;
    [[nodiscard]] std::string url(std::string basePath = {}) const;
};
