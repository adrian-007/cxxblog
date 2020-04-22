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

#include <Wt/WColor.h>
#include <Wt/WFont.h>

#include <vector>
#include <cstdint>

class AvatarGenerator final
{
public:
    explicit AvatarGenerator(double size = 256.0);

    void setBackgroundColor(Wt::WColor color);
    void setTextColor(Wt::WColor color);

    std::vector<uint8_t> generate(const std::string& name);

private:
    std::string nameToLetters(const std::string& str) const;

    double _size;

    Wt::WColor _backgroundColor;
    Wt::WColor _textColor;
};
