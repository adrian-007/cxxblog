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

#include "EditorPersonalInformationModels.h"
#include "ValidatorUtils.h"
#include "AvatarGenerator.h"

#include <Wt/WLengthValidator.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTextArea.h>
#include <Wt/WImage.h>
#include <Wt/WFileUpload.h>
#include <Wt/WMemoryResource.h>

#include <memory>
#include <fstream>
#include <filesystem>

EditorPersonalInformationFormModel::Field EditorPersonalInformationFormModel::NameField = "name";
EditorPersonalInformationFormModel::Field EditorPersonalInformationFormModel::HandleField = "handle";
EditorPersonalInformationFormModel::Field EditorPersonalInformationFormModel::AvatarField = "avatar";
EditorPersonalInformationFormModel::Field EditorPersonalInformationFormModel::AboutMeField = "aboutMe";

EditorPersonalInformationFormModel::EditorPersonalInformationFormModel()
    : Wt::WFormModel()
{
    addField(NameField);
    addField(HandleField);
    addField(AvatarField);
    addField(AboutMeField);

    setValidator(NameField, MakeMandatoryValidator<Wt::WLengthValidator>(2, 40));
    setValidator(HandleField, MakeMandatoryValidator<Wt::WRegExpValidator>("^[a-z0-9-]{1,40}$"));
    setValidator(AboutMeField, std::make_shared<Wt::WLengthValidator>(0, 5000));
}

EditorPersonalInformationFormView::EditorPersonalInformationFormView(Session& session, dbo::ptr<Editor>& editor)
    : _session { session }
    , _editor { editor }
    , _model { std::make_unique<EditorPersonalInformationFormModel>() }
{
    _model->setValue(EditorPersonalInformationFormModel::NameField, _editor->name);
    _model->setValue(EditorPersonalInformationFormModel::HandleField, _editor->handle);
    _model->setValue(EditorPersonalInformationFormModel::AvatarField, _editor->avatar);
    _model->setValue(EditorPersonalInformationFormModel::AboutMeField, _editor->aboutMe);

    setTemplateText(tr("settings.editorForm.personalInformation"));
    addFunction("id", &Wt::WTemplate::Functions::id);
    addFunction("tr", &Wt::WTemplate::Functions::tr);

    setFormWidget(EditorPersonalInformationFormModel::NameField, std::make_unique<Wt::WLineEdit>());
    setFormWidget(EditorPersonalInformationFormModel::HandleField, std::make_unique<Wt::WLineEdit>());
    setFormWidget(EditorPersonalInformationFormModel::AboutMeField, std::make_unique<Wt::WTextArea>());

    auto avatarWidget = std::make_unique<Wt::WImage>();
    _avatar = avatarWidget.get();

    _avatarUploader = bindNew<Wt::WFileUpload>("avatarUpload");
    _avatarUploader->setDisplayWidget(_avatar);
    _avatarUploader->setFilters("image/png");
    _avatarUploader->setFileTextSize(1024 * 1024);
    _avatarUploader->setMultiple(false);

    _avatarUploader->changed().connect(_avatarUploader, &Wt::WFileUpload::upload);
    _avatarUploader->uploaded().connect(this, &EditorPersonalInformationFormView::onUploadCompleted);
    _avatarUploader->fileTooLarge().connect(this, &EditorPersonalInformationFormView::onUploadTooLarge);

    auto onAvatarUpdateView = [=] {
        try
        {
            auto avatarField = _model->value(EditorPersonalInformationFormModel::AvatarField);
            auto avatarBytes = Wt::cpp17::any_cast<std::vector<uint8_t>>(avatarField);

            if (avatarBytes.empty())
            {
                auto name = _model->valueText(EditorPersonalInformationFormModel::NameField);
                avatarBytes = AvatarGenerator(512.0).generate(name.toUTF8());
            }

            auto pngResource = std::make_shared<Wt::WMemoryResource>("image/png", avatarBytes);
            _avatar->setImageLink(Wt::WLink(pngResource));
            _avatar->setMaximumSize(128.0, 128.0);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    };

    auto onAvatarUpdateModel = [=] {
        const auto& files = _avatarUploader->uploadedFiles();

        if (files.empty())
            return;

        auto& file = files.front();
        const auto& uploadedAvatarPath = file.spoolFileName();

        try
        {
            std::ifstream f { uploadedAvatarPath, std::ios::binary };
            f.unsetf(std::ios::skipws);

            if (!f.is_open())
                throw std::runtime_error("Cannot open file");

            std::vector<uint8_t> bytes { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
            f.close();

            std::filesystem::remove(uploadedAvatarPath);
            file.stealSpoolFile();

            _model->setValue(EditorPersonalInformationFormModel::AvatarField, std::move(bytes));
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    };

    setFormWidget(EditorPersonalInformationFormModel::AvatarField, std::move(avatarWidget), onAvatarUpdateView, onAvatarUpdateModel);

    bindNew<Wt::WPushButton>("saveButton", tr("str.save"))->clicked().connect(this, &EditorPersonalInformationFormView::save);
    bindNew<Wt::WPushButton>("useDefaultAvatar", tr("str.useDefaultAvatar"))->clicked().connect(this, &EditorPersonalInformationFormView::useDefaultAvatar);

    setStatus();

    updateView(_model.get());
}

void EditorPersonalInformationFormView::setStatus(const Wt::WString& status /*= {}*/)
{
    status.empty() ? bindEmpty("saveResultInfo") : bindString("saveResultInfo", status);
}

void EditorPersonalInformationFormView::useDefaultAvatar()
{
    _model->setValue(EditorPersonalInformationFormModel::AvatarField, std::vector<uint8_t>{});
    updateViewField(_model.get(), EditorPersonalInformationFormModel::AvatarField);
}

void EditorPersonalInformationFormView::onUploadTooLarge()
{
    _model->setValidation(EditorPersonalInformationFormModel::AvatarField, Wt::WValidator::Result(Wt::ValidationState::Invalid, tr("str.fileTooLarge")));
    updateView(_model.get());
}

void EditorPersonalInformationFormView::onUploadCompleted()
{
    const auto& files = _avatarUploader->uploadedFiles();

    if (files.empty())
    {
        _model->setValidation(EditorPersonalInformationFormModel::AvatarField, Wt::WValidator::Result(Wt::ValidationState::Invalid, tr("str.fileTooLarge")));
    }
    else
    {
        updateModelField(_model.get(), EditorPersonalInformationFormModel::AvatarField);
    }

    updateView(_model.get());
}

void EditorPersonalInformationFormView::save()
{
    updateModel(_model.get());
    auto isValid = _model->validate();

    try
    {
        if (!isValid)
            throw std::runtime_error("Form is not valid.");

        auto avatarField = _model->value(EditorPersonalInformationFormModel::AvatarField);
        auto avatarBytes = Wt::cpp17::any_cast<std::vector<uint8_t>>(avatarField);

        dbo::Transaction t { _session };
        auto e = _editor.modify();

        e->name = _model->valueText(EditorPersonalInformationFormModel::NameField);
        e->handle = _model->valueText(EditorPersonalInformationFormModel::HandleField);
        e->aboutMe = _model->valueText(EditorPersonalInformationFormModel::AboutMeField);
        e->avatar = std::move(avatarBytes);

        t.commit();

        setStatus(tr("str.personalInfoSuccessfullySaved"));
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        setStatus();
    }

    updateView(_model.get());
}
