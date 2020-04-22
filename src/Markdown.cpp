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

#include "Markdown.h"

#include <cmark-gfm-core-extensions.h>
#include <memory>
#include <cassert>

void Markdown::init()
{
    cmark_gfm_core_extensions_ensure_registered();
}

Markdown::Markdown(const std::string& markdown)
{
    auto parser = std::unique_ptr<cmark_parser, void(*)(cmark_parser*)> { cmark_parser_new(CMARK_OPT_DEFAULT | CMARK_OPT_VALIDATE_UTF8), &cmark_parser_free };
    assert(parser != nullptr);

    if (auto extTable = cmark_find_syntax_extension("table"))
    {
        cmark_parser_attach_syntax_extension(parser.get(), extTable);
    }

    if (auto extAutolink = cmark_find_syntax_extension("autolink"))
    {
        cmark_parser_attach_syntax_extension(parser.get(), extAutolink);
    }

    if (auto extStrikethrough = cmark_find_syntax_extension("strikethrough"))
    {
        cmark_parser_attach_syntax_extension(parser.get(), extStrikethrough);
    }

    cmark_parser_feed(parser.get(), markdown.c_str(), markdown.size());

    _root = cmark_parser_finish(parser.get());
}

Markdown::~Markdown()
{
    cmark_node_free(_root);
    _root = nullptr;
}

std::string Markdown::renderHTML() const
{
    std::unique_ptr<char, void(*)(void*)> result = { cmark_render_html(_root, CMARK_OPT_SMART | CMARK_OPT_VALIDATE_UTF8, nullptr), &free };
    return result != nullptr ? std::string { result.get() } : std::string{};
}
