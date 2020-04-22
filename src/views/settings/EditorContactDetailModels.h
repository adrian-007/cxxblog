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

#include "models/Session.h"

#include <Wt/WFormModel.h>
#include <Wt/WTemplateFormView.h>

class Editor;
class EditorContactDetail;

class EditorContactDetailFormModel
    : public Wt::WFormModel
{
public:
    static Field LabelField;
    static Field ValueField;
    static Field TypeField;
    static Field IconHintField;
    static Field VisibilityField;

    EditorContactDetailFormModel();
};

class EditorContactDetailFormView
    : public Wt::WTemplateFormView
{
public:
    EditorContactDetailFormView(Session& session, dbo::ptr<Editor>& editor, dbo::ptr<EditorContactDetail> contactDetail = {});
    dbo::ptr<EditorContactDetail> save();

private:
    Session& _session;
    dbo::ptr<Editor>& _editor;
    dbo::ptr<EditorContactDetail> _contactDetail;

    std::unique_ptr<EditorContactDetailFormModel> _model;
};
