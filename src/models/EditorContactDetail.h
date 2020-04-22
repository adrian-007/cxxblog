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
#include <Wt/Dbo/SqlTraits.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/StdSqlTraits.h>

namespace dbo = Wt::Dbo;

class Editor;

class EditorContactDetail
    : public Wt::Dbo::Dbo<EditorContactDetail>
{
public:
    enum class Type
    {
        Text,
        Link
    };

    enum class Visibility
    {
        Public,  ///< Visible to anyone, everywhere.
        Resume,  ///< Visible only in resume view.
        Private  ///< Visible only to registered users.
    };

    dbo::ptr<Editor> editor;

    Wt::WString label;
    Wt::WString value;
    Type type;
    Wt::WString iconHint;
    Visibility visibility;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, editor, "editor");

        dbo::field(a, label, "label");
        dbo::field(a, value, "value");
        dbo::field(a, type, "type");
        dbo::field(a, iconHint, "iconHint");
        dbo::field(a, visibility, "visibility");
    }

    [[nodiscard]] static Wt::WString visibilityToString(Visibility visibility);
    [[nodiscard]] static Wt::WString typeToString(Type type);

    [[nodiscard]] inline auto visibilityAsString() const { return visibilityToString(visibility); }
    [[nodiscard]] inline auto typeAsString() const { return typeToString(type); }
};
