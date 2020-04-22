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

#include "EditorContactDetailModels.h"

#include "ValidatorUtils.h"

#include "models/Editor.h"
#include "models/EditorContactDetail.h"

#include <Wt/WLengthValidator.h>

#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>

#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>

EditorContactDetailFormModel::Field EditorContactDetailFormModel::LabelField = "label";
EditorContactDetailFormModel::Field EditorContactDetailFormModel::ValueField = "value";
EditorContactDetailFormModel::Field EditorContactDetailFormModel::TypeField = "type";
EditorContactDetailFormModel::Field EditorContactDetailFormModel::IconHintField = "iconHint";
EditorContactDetailFormModel::Field EditorContactDetailFormModel::VisibilityField = "visibility";

EditorContactDetailFormModel::EditorContactDetailFormModel()
    : Wt::WFormModel()
{
    addField(LabelField);
    addField(ValueField);
    addField(TypeField);
    addField(IconHintField);
    addField(VisibilityField);

    setValidator(LabelField, MakeMandatoryValidator<Wt::WLengthValidator>(1, 40));
    setValidator(ValueField, MakeMandatoryValidator<Wt::WLengthValidator>(1, 300));
}

EditorContactDetailFormView::EditorContactDetailFormView(Session& session, dbo::ptr<Editor>& editor, dbo::ptr<EditorContactDetail> contactDetail /*= {}*/)
    : _session { session }
    , _editor { editor }
    , _contactDetail { std::move(contactDetail) }
    , _model { std::make_unique<EditorContactDetailFormModel>() }
{
    if (_contactDetail)
    {
        _model->setValue(EditorContactDetailFormModel::LabelField, _contactDetail->label);
        _model->setValue(EditorContactDetailFormModel::ValueField, _contactDetail->value);
        _model->setValue(EditorContactDetailFormModel::TypeField, _contactDetail->type);
        _model->setValue(EditorContactDetailFormModel::IconHintField, _contactDetail->iconHint);
        _model->setValue(EditorContactDetailFormModel::VisibilityField, _contactDetail->visibility);
    }
    else
    {
        _model->setValue(EditorContactDetailFormModel::TypeField, EditorContactDetail::Type::Text);
        _model->setValue(EditorContactDetailFormModel::VisibilityField, EditorContactDetail::Visibility::Private);
    }

    setTemplateText(tr("settings.editorForm.contactDetails.edit"));
    addFunction("id", &Wt::WTemplate::Functions::id);
    addFunction("tr", &Wt::WTemplate::Functions::tr);

    auto labelLineEdit = std::make_unique<Wt::WLineEdit>();
    auto labelLineEditPtr = labelLineEdit.get();

    setFormWidget(EditorContactDetailFormModel::LabelField, std::move(labelLineEdit));
    setFormWidget(EditorContactDetailFormModel::ValueField, std::make_unique<Wt::WLineEdit>());
    setFormWidget(EditorContactDetailFormModel::IconHintField, std::make_unique<Wt::WLineEdit>());

    auto addEnumToModel = [](std::shared_ptr<Wt::WStandardItemModel>& model, auto enumValue, auto enumToStringFn)
    {
        auto item = std::make_unique<Wt::WStandardItem>();
        item->setText(enumToStringFn(enumValue));
        item->setData(enumValue, Wt::ItemDataRole::User);
        model->appendRow(std::move(item));
    };

    auto indexFromEnumValue = [](const std::shared_ptr<Wt::WStandardItemModel>& model, auto enumValue) -> int
    {
        for (auto i = 0; i < model->rowCount(); ++i)
        {
            auto item = model->item(i);
            auto itemData = Wt::cpp17::any_cast<decltype(enumValue)>(item->data(Wt::ItemDataRole::User));

            if (itemData == enumValue)
                return i;
        }

        return -1;
    };

    auto typeComboBox = std::make_unique<Wt::WComboBox>();
    auto typeComboBoxPtr = typeComboBox.get();
    auto typeModel = std::make_shared<Wt::WStandardItemModel>();

    addEnumToModel(typeModel, EditorContactDetail::Type::Text, &EditorContactDetail::typeToString);
    addEnumToModel(typeModel, EditorContactDetail::Type::Link, &EditorContactDetail::typeToString);

    typeComboBox->setModel(typeModel);

    auto onTypeUpdateView = [=] {
        auto value = Wt::cpp17::any_cast<EditorContactDetail::Type>(_model->value(EditorContactDetailFormModel::TypeField));
        auto i = indexFromEnumValue(typeModel, value);

        if (i >= 0)
            typeComboBoxPtr->setCurrentIndex(i);
    };

    auto onTypeUpdateModel = [=] {
        auto index = typeComboBoxPtr->currentIndex();
        if (index < typeModel->rowCount())
        {
            auto value = Wt::cpp17::any_cast<EditorContactDetail::Type>(typeModel->data(index, 0, Wt::ItemDataRole::User));
            _model->setValue(EditorContactDetailFormModel::TypeField, value);
        }
    };

    setFormWidget(EditorContactDetailFormModel::TypeField, std::move(typeComboBox), onTypeUpdateView, onTypeUpdateModel);

    auto visibilityComboBox = std::make_unique<Wt::WComboBox>();
    auto visibilityComboBoxPtr = visibilityComboBox.get();
    auto visibilityModel = std::make_shared<Wt::WStandardItemModel>();

    addEnumToModel(visibilityModel, EditorContactDetail::Visibility::Public, &EditorContactDetail::visibilityToString);
    addEnumToModel(visibilityModel, EditorContactDetail::Visibility::Resume, &EditorContactDetail::visibilityToString);
    addEnumToModel(visibilityModel, EditorContactDetail::Visibility::Private, &EditorContactDetail::visibilityToString);

    visibilityComboBox->setModel(visibilityModel);

    auto onVisibilityUpdateView = [=] {
        auto value = Wt::cpp17::any_cast<EditorContactDetail::Visibility>(_model->value(EditorContactDetailFormModel::VisibilityField));
        auto i = indexFromEnumValue(visibilityModel, value);

        if (i >= 0)
            visibilityComboBoxPtr->setCurrentIndex(i);
    };

    auto onVisibilityUpdateModel = [=] {
        auto index = visibilityComboBoxPtr->currentIndex();
        if (index < visibilityModel->rowCount())
        {
            auto value = Wt::cpp17::any_cast<EditorContactDetail::Visibility>(visibilityModel->data(index, 0, Wt::ItemDataRole::User));
            _model->setValue(EditorContactDetailFormModel::VisibilityField, value);
        }
    };

    setFormWidget(EditorContactDetailFormModel::VisibilityField, std::move(visibilityComboBox), onVisibilityUpdateView, onVisibilityUpdateModel);

    updateView(_model.get());

    labelLineEditPtr->setFocus();
}

dbo::ptr<EditorContactDetail> EditorContactDetailFormView::save()
{
    updateModel(_model.get());
    auto isValid = _model->validate();

    try
    {
        if (!isValid)
            throw std::runtime_error("Form is not valid.");

        auto label = _model->valueText(EditorContactDetailFormModel::LabelField).trim();
        auto value = _model->valueText(EditorContactDetailFormModel::ValueField).trim();
        auto type = Wt::cpp17::any_cast<EditorContactDetail::Type>(_model->value(EditorContactDetailFormModel::TypeField));
        auto iconHint = _model->valueText(EditorContactDetailFormModel::IconHintField).trim();
        auto visibility = Wt::cpp17::any_cast<EditorContactDetail::Visibility>(_model->value(EditorContactDetailFormModel::VisibilityField));

        dbo::Transaction t { _session };

        auto contactDetailPdo = _contactDetail ? _contactDetail : _session.addNew<EditorContactDetail>();
        auto contactDetail = contactDetailPdo.modify();

        contactDetail->editor = _editor;
        contactDetail->label = label;
        contactDetail->iconHint = iconHint;
        contactDetail->value = std::move(value);
        contactDetail->type = type;
        contactDetail->visibility = visibility;

        t.commit();

        return contactDetailPdo;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    updateView(_model.get());
    return {};
}
