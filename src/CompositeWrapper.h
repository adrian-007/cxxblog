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

#include <Wt/WCompositeWidget.h>

template<typename T>
struct CompositeWrapper
{
    template<typename...A>
    static std::unique_ptr<Wt::WCompositeWidget> createNew(A&&...a)
    {
        std::unique_ptr<Wt::WWidget> widget { new T(std::forward<A>(a)...) };

        struct Allocator : public Wt::WCompositeWidget
        {
            explicit Allocator(std::unique_ptr<Wt::WWidget> widget)
            {
                setImplementation(std::move(widget));
            }
        };

        return std::make_unique<Allocator>(std::move(widget));
    }
};
