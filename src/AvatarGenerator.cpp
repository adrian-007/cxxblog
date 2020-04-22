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

#include "AvatarGenerator.h"

#include <Wt/WRasterImage.h>
#include <Wt/WPainter.h>

#include <sstream>
#include <cstring>
#include <cassert>

AvatarGenerator::AvatarGenerator(double size)
    : _size(size)
    , _backgroundColor { 100, 120, 140 }
    , _textColor { 190, 190, 190 }
{
    assert(size >= 64.0);
}

void AvatarGenerator::setBackgroundColor(Wt::WColor color)
{
    _backgroundColor = std::move(color);
}

void AvatarGenerator::setTextColor(Wt::WColor color)
{
    _textColor = std::move(color);
}

std::vector<uint8_t> AvatarGenerator::generate(const std::string& name)
{
    auto letters = nameToLetters(name);

    Wt::WRasterImage png("png", _size, _size);

    {
        const auto textPadding = _size * 0.0625;
        const Wt::WRectF textBoundingRect { textPadding, textPadding, _size - (textPadding * 2), _size - (textPadding * 2)};

        Wt::WFont font { Wt::FontFamily::SansSerif };
        font.setSize(_size * 0.55);

        Wt::WPen pen { _textColor };
        pen.setCapStyle(Wt::PenCapStyle::Round);
        pen.setJoinStyle(Wt::PenJoinStyle::Round);

        Wt::WPainter p { &png };
        p.setPen(pen);
        p.setFont(font);

        p.fillRect(0.0, 0.0, _size, _size, Wt::WBrush { _backgroundColor });
        p.drawText(textBoundingRect, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Middle, letters);
    }

    // TODO: I don't like it, it copies data twice. Optimize it.
    std::stringstream ss;
    png.write(ss);

    auto s = ss.str();

    std::vector<uint8_t> buffer;

    buffer.resize(s.size());
    std::memcpy(&buffer[0], &s[0], buffer.size());

    return buffer;
}

std::string AvatarGenerator::nameToLetters(const std::string& str) const
{
    std::string letters;
    letters.reserve(2);

    std::locale loc("");
    using ctype = std::ctype<char>;
    auto& facet = std::use_facet<ctype>(loc);

    for (const auto c : str)
    {
        if (facet.is(ctype::upper, c))
            letters += c;

        if (letters.size() >= 2)
            break;
    }

    if (letters.empty())
        letters = "?";

    return letters;
}
