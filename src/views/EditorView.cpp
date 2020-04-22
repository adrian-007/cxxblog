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

#include "EditorView.h"
#include "Markdown.h"

#include "models/Editor.h"
#include "models/EditorContactDetail.h"

#include <Wt/WLink.h>
#include <Wt/WImage.h>
#include <Wt/WAnchor.h>

EditorView::EditorView(Session& session, dbo::ptr<Editor> editor)
    : Wt::WTemplate(tr("editorView"))
    , _session(session)
    , _editor(std::move(editor))
{
    dbo::Transaction t { _session };

    Wt::WLink avatarLink { Wt::LinkType::Url, _session.relativePath("avatar/" + _editor->handle.toUTF8()) };
    bindNew<Wt::WImage>("avatar", avatarLink);

    bindString("name", _editor->name);
    bindString("aboutMe", Markdown(_editor->aboutMe.toUTF8()).renderHTML());

    auto container = bindNew<Wt::WContainerWidget>("contactDetails");

    for (const auto& detail : _editor->contactDetails)
    {
        if (detail->visibility == EditorContactDetail::Visibility::Public || (detail->visibility == EditorContactDetail::Visibility::Private && _session.login().loggedIn()))
        {
            auto iconHint = detail->iconHint;
            if (iconHint.empty())
                iconHint = detail->type == EditorContactDetail::Type::Link ? "fas fa-link" : "d-none";

            auto detailTemplate = container->addNew<Wt::WTemplate>(tr("editorView.contactDetail"));

            switch (detail->type)
            {
                case EditorContactDetail::Type::Text:
                    detailTemplate->bindString("label", detail->label + ": ");
                    detailTemplate->bindString("value", detail->value);
                    break;
                case EditorContactDetail::Type::Link:
                {
                    Wt::WLink link { detail->value.toUTF8() };
                    link.setTarget(Wt::LinkTarget::NewWindow);

                    detailTemplate->bindEmpty("label");
                    detailTemplate->bindNew<Wt::WAnchor>("value", link, detail->label);
                    break;
                }
            }

            detailTemplate->bindString("iconHint", iconHint);
        }
    }
}
