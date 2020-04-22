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

#include "EditorContactDetail.h"

#include <Wt/WLink.h>

Wt::WString EditorContactDetail::visibilityToString(Visibility visibility)
{
    switch (visibility)
    {
        case Visibility::Public:
            return Wt::WString::tr("str.public");
        case Visibility::Resume:
            return Wt::WString::tr("str.resume");
        case Visibility::Private:
            return Wt::WString::tr("str.private");
    }

    assert(false);
}

Wt::WString EditorContactDetail::typeToString(EditorContactDetail::Type type)
{
    switch (type)
    {
        case Type::Text:
            return Wt::WString::tr("str.text");
        case Type::Link:
            return Wt::WString::tr("str.link");
    }

    assert(false);
}
