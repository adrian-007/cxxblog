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

#include "EditorChangeCredentialsModels.h"

#include "ValidatorUtils.h"

#include <Wt/WLengthValidator.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/Auth/PasswordStrengthValidator.h>

#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>

namespace
{
    class OptionalPasswordStrengthValidator
        : public Wt::Auth::PasswordStrengthValidator
    {
    public:
        OptionalPasswordStrengthValidator()
            : Wt::Auth::PasswordStrengthValidator()
        {
            setMandatory(false);
        }

        Wt::WValidator::Result validate(const Wt::WString& password, const Wt::WString& loginName, const std::string& email) const override
        {
            if (!isMandatory() && password.empty())
                return Wt::WValidator::Result(Wt::ValidationState::Valid);

            return Wt::Auth::PasswordStrengthValidator::validate(password, loginName, email);
        }
    };
}

EditorChangeCredentialsFormModel::Field EditorChangeCredentialsFormModel::NewUsername = "newUsername";
EditorChangeCredentialsFormModel::Field EditorChangeCredentialsFormModel::NewPassword = "newPassword";
EditorChangeCredentialsFormModel::Field EditorChangeCredentialsFormModel::ConfirmNewPassword = "confirmNewPassword";
EditorChangeCredentialsFormModel::Field EditorChangeCredentialsFormModel::CurrentPassword = "currentPassword";

EditorChangeCredentialsFormModel::EditorChangeCredentialsFormModel()
    : Wt::WFormModel()
{
    addField(NewUsername);
    addField(NewPassword);
    addField(ConfirmNewPassword);
    addField(CurrentPassword);

    setValidator(NewUsername, std::make_shared<Wt::WRegExpValidator>("^[a-zA-Z0-9-_]{5,200}$"));

    auto passwordValidator = std::make_shared<OptionalPasswordStrengthValidator>();

    setValidator(NewPassword, passwordValidator);
    setValidator(ConfirmNewPassword, passwordValidator);
    setValidator(CurrentPassword, MakeMandatoryValidator<Wt::WLengthValidator>(1, std::numeric_limits<int>::max()));
}

EditorChangeCredentialsFormView::EditorChangeCredentialsFormView(Session& session, dbo::ptr<Editor>& editor)
    : _session { session }
    , _editor { editor }
    , _model { std::make_unique<EditorChangeCredentialsFormModel>() }
{
    setTemplateText(tr("settings.editorForm.changeCredentials"));
    addFunction("id", &Wt::WTemplate::Functions::id);
    addFunction("tr", &Wt::WTemplate::Functions::tr);

    auto createPasswordWidget = []
    {
        auto w = std::make_unique<Wt::WLineEdit>();
        w->setEchoMode(Wt::EchoMode::Password);
        return w;
    };

    setFormWidget(EditorChangeCredentialsFormModel::NewUsername, std::make_unique<Wt::WLineEdit>());
    setFormWidget(EditorChangeCredentialsFormModel::NewPassword, createPasswordWidget());
    setFormWidget(EditorChangeCredentialsFormModel::ConfirmNewPassword, createPasswordWidget());
    setFormWidget(EditorChangeCredentialsFormModel::CurrentPassword, createPasswordWidget());

    bindNew<Wt::WPushButton>("saveButton", tr("str.save"))->clicked().connect(this, &EditorChangeCredentialsFormView::save);

    setStatus();
    updateView(_model.get());
}

void EditorChangeCredentialsFormView::setStatus(const Wt::WString& status /*= {}*/)
{
    status.empty() ? bindEmpty("saveResultInfo") : bindString("saveResultInfo", status);
}

void EditorChangeCredentialsFormView::save()
{
    updateModel(_model.get());

    try
    {
        const auto& login = _session.login();

        if (!login.loggedIn())
            throw std::logic_error("User is not logged in.");

        auto newPassword1 = _model->valueText(EditorChangeCredentialsFormModel::NewPassword);
        auto newPassword2 = _model->valueText(EditorChangeCredentialsFormModel::ConfirmNewPassword);

        if (newPassword1 != newPassword2)
        {
            Wt::WValidator::Result result(Wt::ValidationState::Invalid, tr("str.passwordsMismatch"));

            _model->setValidation(EditorChangeCredentialsFormModel::NewPassword, result);
            _model->setValidation(EditorChangeCredentialsFormModel::ConfirmNewPassword, result);

            throw std::runtime_error("Passwords mismatch");
        }

        if (!_model->validate())
            throw std::runtime_error("Form is not valid.");

        assert(newPassword1 == newPassword2);

        auto newUsername = _model->valueText(EditorChangeCredentialsFormModel::NewUsername);
        auto currentPassword = _model->valueText(EditorChangeCredentialsFormModel::CurrentPassword);

        dbo::Transaction t { _session };

        if (!newUsername.empty() && _session.users().findWithIdentity(Wt::Auth::Identity::LoginName, newUsername).isValid())
        {
            Wt::WValidator::Result result(Wt::ValidationState::Invalid, tr("str.usernameAlreadyExist"));
            _model->setValidation(EditorChangeCredentialsFormModel::NewUsername, result);

            throw std::runtime_error("Username already exist");
        }

        auto result = _session.updateCredentials(login, currentPassword, newUsername, newPassword1);

        switch (result)
        {
            case Wt::Auth::PasswordResult::PasswordInvalid:
                _model->setValidation(EditorChangeCredentialsFormModel::CurrentPassword, Wt::WValidator::Result(Wt::ValidationState::Invalid, tr("str.incorrectPassword")));
                break;
            case Wt::Auth::PasswordResult::LoginThrottling:
                _model->setValidation(EditorChangeCredentialsFormModel::CurrentPassword, Wt::WValidator::Result(Wt::ValidationState::Invalid, tr("str.passwordThrottlingTryLater")));
                break;
            case Wt::Auth::PasswordResult::PasswordValid:
                _model->setValue(EditorChangeCredentialsFormModel::NewUsername, Wt::WString::Empty);
                _model->setValue(EditorChangeCredentialsFormModel::NewPassword, Wt::WString::Empty);
                _model->setValue(EditorChangeCredentialsFormModel::ConfirmNewPassword, Wt::WString::Empty);
                _model->setValue(EditorChangeCredentialsFormModel::CurrentPassword, Wt::WString::Empty);

                setStatus(tr("str.saved"));
                break;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        setStatus();
    }

    updateView(_model.get());
}
