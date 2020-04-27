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

#include "SettingsView.h"
#include "models/Editor.h"
#include "settings/SettingsTabEditor.h"

#include <Wt/WApplication.h>
#include <Wt/WTemplate.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WPushButton.h>
#include <Wt/WTabWidget.h>
#include <Wt/WFormModel.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WRegExpValidator.h>

#include "ValidatorUtils.h"

namespace
{
    struct SiteConfigFormModel : public Wt::WFormModel
    {
        static Field SiteNameField;
        static Field AboutField;
        static Field FooterField;
        static Field DisqusShortnameField;

        SiteConfigFormModel(const SiteConfig& siteConfig) : Wt::WFormModel()
        {
            addField(SiteNameField);
            addField(AboutField);
            addField(FooterField);
            addField(DisqusShortnameField);

            auto siteNameRegex { siteConfig.siteNameRegex() };
            if (!siteNameRegex.empty())
                setValidator(SiteNameField, MakeMandatoryValidator<Wt::WRegExpValidator>(siteNameRegex));

            auto aboutRegex { siteConfig.aboutRegex() };
            if (!aboutRegex.empty())
                setValidator(AboutField, std::make_shared<Wt::WRegExpValidator>(aboutRegex));

            auto footerRegex { siteConfig.footerRegex() };
            if (!footerRegex.empty())
                setValidator(FooterField, std::make_shared<Wt::WRegExpValidator>(footerRegex));

            auto disqusShortnameRegex { siteConfig.disqusShortnameRegex() };
            if (!disqusShortnameRegex.empty())
                setValidator(DisqusShortnameField, std::make_shared<Wt::WRegExpValidator>(disqusShortnameRegex));

            setValue(SiteNameField, siteConfig.siteName());
            setValue(AboutField, siteConfig.about());
            setValue(FooterField, siteConfig.footer());
            setValue(DisqusShortnameField, siteConfig.disqusShortname());
        }
    };

    SiteConfigFormModel::Field SiteConfigFormModel::SiteNameField = "name";
    SiteConfigFormModel::Field SiteConfigFormModel::AboutField = "about";
    SiteConfigFormModel::Field SiteConfigFormModel::FooterField = "footer";
    SiteConfigFormModel::Field SiteConfigFormModel::DisqusShortnameField = "disqusShortname";

    struct SiteConfigFormView : public Wt::WTemplateFormView
    {
        explicit SiteConfigFormView(Session& session)
            : _session(session)
        {
            _model = std::make_shared<SiteConfigFormModel>(session.siteConfig());

            setTemplateText(tr("settings.siteConfigForm"));
            addFunction("id", &Wt::WTemplate::Functions::id);

            setFormWidget(SiteConfigFormModel::SiteNameField, std::make_unique<Wt::WLineEdit>());
            setFormWidget(SiteConfigFormModel::AboutField, std::make_unique<Wt::WTextArea>());
            setFormWidget(SiteConfigFormModel::FooterField, std::make_unique<Wt::WLineEdit>());
            setFormWidget(SiteConfigFormModel::DisqusShortnameField, std::make_unique<Wt::WLineEdit>());

            bindNew<Wt::WPushButton>("saveButton", tr("str.save"))->clicked().connect(this, &SiteConfigFormView::save);

            setStatus();
            updateView(_model.get());
        }

    private:
        void setStatus(const Wt::WString& status = {})
        {
            status.empty() ? bindEmpty("saveResultInfo") : bindString("saveResultInfo", status);
        }

        void save()
        {
            updateModel(_model.get());
            auto isValid = _model->validate();

            try
            {
                if (!isValid)
                    throw std::runtime_error("Form is not valid");

                dbo::Transaction t { _session };
                auto& siteConfig { _session.siteConfig() };

                siteConfig.siteName(_model->valueText(SiteConfigFormModel::SiteNameField).toUTF8());
                siteConfig.about(_model->valueText(SiteConfigFormModel::AboutField).toUTF8());
                siteConfig.footer(_model->valueText(SiteConfigFormModel::FooterField).toUTF8());
                siteConfig.disqusShortname(_model->valueText(SiteConfigFormModel::DisqusShortnameField).toUTF8());

                t.commit();

                setStatus(tr("str.siteConfigurationSuccessfullySaved"));
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                setStatus();
            }
        }

        Session& _session;
        std::shared_ptr<SiteConfigFormModel> _model;
    };
}

SettingsView::SettingsView(Session& session)
    : _session(session)
{
    auto editor = _session.editor();
    assert(editor);

    auto editorRole = editor->role;

    wApp->messageResourceBundle().use(Wt::WApplication::appRoot() + "xml/SettingsView");

    auto tabs = std::make_unique<Wt::WTabWidget>();
    tabs->setMinimumSize(Wt::WLength(), 400);

    tabs->addTab(SettingsTabEditor::createNew(session, std::move(editor)), tr("str.myProfile"));

    if (editorRole == Editor::Role::Admin)
    {
        auto siteConfigTabView = std::make_unique<SiteConfigFormView>(session);
        siteConfigTabView->setMargin(Wt::WLength(16), Wt::Side::Top);
        tabs->addTab(std::move(siteConfigTabView), tr("str.siteConfiguration"));
    }

    setImplementation(std::move(tabs));
}
