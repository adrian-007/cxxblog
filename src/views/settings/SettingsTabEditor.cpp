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

#include "SettingsTabEditor.h"

#include "EditorPersonalInformationModels.h"
#include "EditorChangeCredentialsModels.h"
#include "EditorContactDetailsWidget.h"

#include "models/EditorContactDetail.h"

#include <Wt/WGroupBox.h>
#include <Wt/WText.h>

SettingsTabEditor::SettingsTabEditor(Session& session, dbo::ptr<Editor> editor)
    : _session(session)
    , _editor(std::move(editor))
{
    setMargin(Wt::WLength(16), Wt::Side::Top);

    try
    {
        dbo::Transaction t { session };

        if (!_session.login().loggedIn())
            throw AccessDeniedException();

        auto personalInfoGroupBox = addNew<Wt::WGroupBox>(tr("str.personalInformation"));
        personalInfoGroupBox->setMargin(20, Wt::Side::Bottom);
        personalInfoGroupBox->addNew<EditorPersonalInformationFormView>(session, _editor);

        auto contactInfoGroupBox = addNew<Wt::WGroupBox>(tr("str.contactDetails"));
        contactInfoGroupBox->setMargin(20, Wt::Side::Bottom);
        contactInfoGroupBox->addWidget(EditorContactDetailsWidget::createNew(_session, _editor));

        auto changePasswordGroupBox = addNew<Wt::WGroupBox>(tr("str.changeCredentials"));
        changePasswordGroupBox->setMargin(20, Wt::Side::Bottom);
        changePasswordGroupBox->addNew<EditorChangeCredentialsFormView>(session, _editor);
    }
    catch (const std::exception& e)
    {
        addNew<Wt::WText>(Wt::WString("Fatal exception: {1}").arg(e.what()));
    }
}
