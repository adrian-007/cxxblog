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

namespace dbo = Wt::Dbo;

namespace ConfigKeys
{
    constexpr std::string_view SiteName { "site.name" };
    constexpr std::string_view SiteAbout { "site.about" };
    constexpr std::string_view SiteFooter { "site.footer" };
    constexpr std::string_view SiteDisqusShortname { "site.disqusShortname" };
}

class ConfigStore
{
public:
    ConfigStore() = default;
    ConfigStore(const std::string_view& key, std::string value, std::string valueRegex = {});

    std::string key;
    std::string value;
    std::string valueRegex;

    template<class Action>
    void persist(Action& a)
    {
        dbo::id(a, key, "key", 100);
        dbo::field(a, value, "value");
        dbo::field(a, valueRegex, "value_regex");
    }
};

namespace Wt::Dbo
{
    template<>
    struct dbo_traits<ConfigStore> : public dbo_default_traits
    {
        using IdType = std::string;

        static IdType invalidId() { return IdType{}; }
        static const char* surrogateIdField() { return nullptr; }
        static const char* versionField() { return nullptr; }
    };
}

DBO_EXTERN_TEMPLATES(ConfigStore)
