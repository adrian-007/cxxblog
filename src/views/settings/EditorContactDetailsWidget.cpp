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

#include "EditorContactDetailsWidget.h"
#include "EditorContactDetailModels.h"

#include <Wt/WPushButton.h>
#include <Wt/WDialog.h>

EditorContactDetailsWidget::EditorContactDetailsWidget(Session& session, dbo::ptr<Editor> editor)
    : Wt::WTemplate(tr("settings.editorForm.contactDetails"))
    , _session { session }
    , _editor { std::move(editor) }
{
    addFunction("tr", Functions::tr);

    bindNew<Wt::WContainerWidget>("items");

    auto addContactDetailButton = bindNew<Wt::WPushButton>("addContactDetailButton", tr("str.addContactDetail"));
    addContactDetailButton->clicked().connect(this, std::bind(&EditorContactDetailsWidget::showContactDetailDialog, this, nullptr, dbo::ptr<EditorContactDetail>{}));

    for (const auto& detail : _editor->contactDetails)
    {
        createItem(detail);
    }
}

void EditorContactDetailsWidget::showContactDetailDialog(Wt::WTemplate* item, dbo::ptr<EditorContactDetail> contactDetail /*= {}*/)
{
    auto dialog = addChild(std::make_unique<Wt::WDialog>(tr("str.addContactDetail")));

    dialog->setModal(true);

    auto form = dialog->contents()->addNew<EditorContactDetailFormView>(_session, _editor, std::move(contactDetail));
    auto okButton = dialog->footer()->addNew<Wt::WPushButton>(tr("str.save"));
    auto cancelButton = dialog->footer()->addNew<Wt::WPushButton>(tr("str.cancel"));

    okButton->clicked().connect(dialog, [=]
    {
        auto resultDetail = form->save();
        if (resultDetail)
        {
            item != nullptr ? updateItem(item, resultDetail) : (void)createItem(resultDetail);

            dialog->accept();
        }
    });

    cancelButton->clicked().connect(dialog, &Wt::WDialog::reject);

    dialog->show();
}

void EditorContactDetailsWidget::onContactDetailEdit(Wt::WTemplate* item, dbo::ptr<EditorContactDetail> detail)
{
    showContactDetailDialog(item, std::move(detail));
}

void EditorContactDetailsWidget::onContactDetailDelete(Wt::WTemplate* item, dbo::ptr<EditorContactDetail> detail)
{
    try
    {
        dbo::Transaction t { _session };
        detail.remove();
        t.commit();

        item->removeFromParent();
    }
    catch (const std::exception& e)
    {

    }
}

Wt::WTemplate* EditorContactDetailsWidget::createItem(const dbo::ptr<EditorContactDetail>& detail)
{
    assert(detail);

    auto parent = resolve<Wt::WContainerWidget*>("items");
    assert(parent != nullptr);

    auto item = parent->addNew<Wt::WTemplate>(tr("settings.editorForm.contactDetails.show"));
    updateItem(item, detail);

    auto editButton = item->bindNew<Wt::WPushButton>("editButton", tr("str.edit"));
    editButton->clicked().connect(this, std::bind(&EditorContactDetailsWidget::onContactDetailEdit, this, item, detail));

    auto deleteButton = item->bindNew<Wt::WPushButton>("deleteButton", tr("str.delete"));
    deleteButton->clicked().connect(this, std::bind(&EditorContactDetailsWidget::onContactDetailDelete, this, item, detail));

    return item;
}

void EditorContactDetailsWidget::updateItem(Wt::WTemplate* item, const dbo::ptr<EditorContactDetail>& detail) const
{
    assert(detail);

    item->bindString("label", detail->label);
    item->bindString("value", detail->value);
    item->bindString("type", detail->typeAsString());
    item->bindString("iconHint", detail->iconHint);
    item->bindString("visibility", detail->visibilityAsString());
}
