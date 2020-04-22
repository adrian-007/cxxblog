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

#include "CompositeWrapper.h"

#include "models/Session.h"
#include "models/Editor.h"
#include "models/EditorContactDetail.h"

#include <Wt/WTemplate.h>

class EditorContactDetailsWidget
    : public CompositeWrapper<EditorContactDetailsWidget>
    , private Wt::WTemplate
{
public:

private:
    friend class CompositeWrapper<EditorContactDetailsWidget>;

    EditorContactDetailsWidget(Session& session, dbo::ptr<Editor> editor);

    void showContactDetailDialog(Wt::WTemplate* item, dbo::ptr<EditorContactDetail> contactDetail = { });
    void onContactDetailEdit(Wt::WTemplate* item, dbo::ptr<EditorContactDetail> detail);
    void onContactDetailDelete(Wt::WTemplate* item, dbo::ptr<EditorContactDetail> detail);

    Wt::WTemplate* createItem(const dbo::ptr<EditorContactDetail>& detail);
    void updateItem(Wt::WTemplate* item, const dbo::ptr<EditorContactDetail>& detail) const;

    Session& _session;
    dbo::ptr<Editor> _editor;
};