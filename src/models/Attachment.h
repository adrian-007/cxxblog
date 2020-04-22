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

#include <Wt/WString.h>
#include <Wt/WDateTime.h>

#include <vector>
#include <cstdint>

namespace dbo = Wt::Dbo;

class Attachment
{
public:
    Wt::WDateTime created;
    Wt::WString name;
    Wt::WString mimeType;
    std::vector<uint8_t> data;

    template<class Action>
    void persist(Action& a)
    {
        dbo::field(a, created, "created");
        dbo::field(a, name, "name");
        dbo::field(a, mimeType, "mimeType");
        dbo::field(a, data, "data");
    }
};

namespace Wt::Dbo
{
    template<>
    struct dbo_traits<Attachment> : public dbo_default_traits
    {
        static const char* versionField() { return nullptr; }
    };
}

DBO_EXTERN_TEMPLATES(Attachment)
