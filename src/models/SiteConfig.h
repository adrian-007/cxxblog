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

#include "ConfigStore.h"

class Session;

struct SiteConfig
{
    SiteConfig(const SiteConfig&) = delete;
    SiteConfig& operator=(const SiteConfig&) = delete;
    SiteConfig& operator=(SiteConfig&&) = delete;

    auto siteName() const { return _siteName->value; }
    void siteName(std::string value) { _siteName.modify()->value = std::move(value); }
    auto siteNameRegex() const { return _siteName->valueRegex; }

    auto about() const { return _about->value; }
    void about(std::string value) { _about.modify()->value = std::move(value); }
    auto aboutRegex() const { return _about->valueRegex; }

    auto footer() const { return _footer->value; }
    void footer(std::string value) { _footer.modify()->value = std::move(value); }
    auto footerRegex() const { return _footer->valueRegex; }

private:
    friend class Session;

    SiteConfig() = default;

    dbo::ptr<ConfigStore> _siteName;
    dbo::ptr<ConfigStore> _about;
    dbo::ptr<ConfigStore> _footer;
};
