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

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/SqlTraits.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/StdSqlTraits.h>

#include <Wt/WDate.h>
#include <Wt/WString.h>

namespace dbo = Wt::Dbo;

class Editor;
class EditorEmployment;
class EditorProject;
class EditorEducation;
class Technology;
class EditorTechnologyExperience;

class EditorResume
    : public dbo::Dbo<EditorResume>
{
public:
    Wt::WString resumeName;
    Wt::WString password;

    dbo::ptr<Editor> editor;

    Wt::WString fullname;
    Wt::WString title;
    Wt::WString summary;
    Wt::WString gprdStatement;

    dbo::collection<dbo::ptr<EditorEmployment>> employments;
    dbo::collection<dbo::ptr<EditorProject>> projects;
    dbo::collection<dbo::ptr<EditorEducation>> education;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::field(a, resumeName, "resume_name");
        dbo::field(a, password, "password");

        dbo::belongsTo(a, editor, "editor");

        dbo::field(a, fullname, "full_name");
        dbo::field(a, title, "title");
        dbo::field(a, summary, "summary");
        dbo::field(a, gprdStatement, "gprdStatement");

        dbo::hasMany(a, employments, dbo::ManyToMany);
        dbo::hasMany(a, projects, dbo::ManyToMany);
        dbo::hasMany(a, education, dbo::ManyToMany);
    }
};

class EditorEmployment
    : public dbo::Dbo<EditorEmployment>
{
public:
    dbo::ptr<Editor> editor;

    Wt::WString position;
    Wt::WString company;
    Wt::WString companyUrl;
    Wt::WString location;

    Wt::WDate from;
    Wt::WDate to;

    Wt::WString description;

    dbo::collection<dbo::ptr<Technology>> technologies;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, editor, "editor");

        dbo::field(a, position, "position");
        dbo::field(a, company, "company");
        dbo::field(a, companyUrl, "companyUrl");
        dbo::field(a, location, "location");

        dbo::field(a, from, "from");
        dbo::field(a, to, "to");
        dbo::field(a, description, "description");

        dbo::hasMany(a, technologies, dbo::ManyToMany);
    }
};

class EditorProject
    : public dbo::Dbo<EditorProject>
{
public:
    dbo::ptr<Editor> editor;

    Wt::WString title;
    Wt::WString description;
    Wt::WString url;

    Wt::WDate from;
    Wt::WDate to;

    dbo::collection<dbo::ptr<Technology>> technologies;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, editor, "editor");

        dbo::field(a, title, "title");
        dbo::field(a, description, "description");
        dbo::field(a, url, "url");
        dbo::field(a, from, "from");
        dbo::field(a, to, "to");

        dbo::hasMany(a, technologies, dbo::ManyToMany);
    }
};

class EditorEducation
    : public dbo::Dbo<EditorEducation>
{
public:
    dbo::ptr<Editor> editor;

    Wt::WString title;
    Wt::WString issuer;

    Wt::WDate from;
    Wt::WDate to;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, editor, "editor");

        dbo::field(a, title, "title");
        dbo::field(a, issuer, "issuer");

        dbo::field(a, from, "from");
        dbo::field(a, to, "to");
    }
};

class EditorTechnologyExperience;

class Technology
    : public dbo::Dbo<Technology>
{
public:
    enum class Type
    {
        ProgrammingLanguage,
        OperatingSystem,
        Framework,
        Library,
    };

    Wt::WString name;
    Type type;

    dbo::collection<dbo::ptr<EditorTechnologyExperience>> editorsTechnologyExperience;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::hasMany(a, editorsTechnologyExperience, dbo::ManyToMany);

        dbo::field(a, name, "name");
        dbo::field(a, type, "type");
    }
};

class EditorTechnologyExperience
    : public dbo::Dbo<EditorTechnologyExperience>
{
public:
    enum class Experience
    {
        Beginner,
        Intermediate,
        Professional
    };

    dbo::ptr<Editor> editor;
    dbo::ptr<Technology> technology;

    Experience experience;
    short priority;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, editor, "editor", dbo::NotNull | dbo::OnUpdateCascade | dbo::OnDeleteCascade);
        dbo::belongsTo(a, technology, "technology", dbo::NotNull | dbo::OnUpdateCascade);

        dbo::field(a, experience, "experience");
        dbo::field(a, priority, "priority");
    }
};
