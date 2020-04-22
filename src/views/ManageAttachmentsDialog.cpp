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

#include "ManageAttachmentsDialog.h"

#include "models/Attachment.h"

#include <Wt/WTemplate.h>
#include <Wt/WFileUpload.h>
#include <Wt/WProgressBar.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

ManageAttachmentsDialog::ManageAttachmentsDialog(Session& session)
    : Wt::WDialog(tr("str.manageAttachments"))
    , _session(session)
{
    setClosable(true);
    setResizable(true);
    setModal(false);

    contents()->setMaximumSize(Wt::WLength("100%"), Wt::WLength("100%"));

    auto uploadTemplate = footer()->addNew<Wt::WTemplate>(tr("manageAttachmentsWidgetView.upload"));

    const auto addTemplateFunctions = [](Wt::WTemplate* t)
    {
        t->addFunction("tr", &Wt::WTemplate::Functions::tr);
        t->addFunction("id", &Wt::WTemplate::Functions::id);
    };

    addTemplateFunctions(uploadTemplate);

    auto setStatusText = [=](auto text)
    {
        uploadTemplate->bindString("status", text);
    };

    {
        dbo::Transaction t { _session };
        auto attachments = _session.find<Attachment>().orderBy("created desc").resultList();

        for (const auto& attachment : attachments)
        {
            contents()->addWidget(createItem(attachment));
        }
    }

    auto uploadFileName = uploadTemplate->bindNew<Wt::WLineEdit>("name");
    auto selectFileButton = uploadTemplate->bindNew<Wt::WPushButton>("selectFile", tr("str.selectFile"));
    auto uploadProgressBar = uploadTemplate->bindNew<Wt::WProgressBar>("progressBar");
    auto fileUpload = uploadTemplate->bindNew<Wt::WFileUpload>("file");
    auto saveButton = uploadTemplate->bindNew<Wt::WPushButton>("save", tr("str.save"));

    fileUpload->setMultiple(false);
    fileUpload->setProgressBar(uploadProgressBar);
    fileUpload->setDisplayWidget(selectFileButton);

    uploadFileName->setWidth(250.0);

    fileUpload->changed().connect(fileUpload, [=]
    {
        setStatusText(tr("str.uploading"));
        uploadProgressBar->setValue(0.0);
        uploadFileName->setText(Wt::WString::Empty);
        fileUpload->upload();
    });

    fileUpload->fileTooLarge().connect(this, [=]
    {
        setStatusText(tr("str.fileTooLarge"));
        uploadProgressBar->setValue(0.0);
        uploadFileName->setText(Wt::WString::Empty);
    });

    fileUpload->uploaded().connect(this, [=]
    {
        auto uploadedFiles = fileUpload->uploadedFiles();
        if (uploadedFiles.empty())
            return;

        const auto& file = uploadedFiles.back();
        uploadFileName->setText(file.clientFileName());
        setStatusText(tr("str.uploadFinished"));
    });

    saveButton->clicked().connect(uploadTemplate, [=]
    {
        try
        {
            auto uploadedFiles = fileUpload->uploadedFiles();
            if (uploadedFiles.empty())
                return;

            auto file = uploadedFiles.back();

            std::ifstream f { file.spoolFileName(), std::ios::binary };
            f.unsetf(std::ios::skipws);

            if (!f.is_open())
                throw std::runtime_error("Cannot open file");

            std::vector<uint8_t> bytes { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
            f.close();

            auto name = uploadFileName->text().trim();

            if (name.empty())
                name = file.clientFileName().empty() ? "unknown" : file.clientFileName();

            dbo::Transaction t { _session };

            auto contentSearchResults = _session.find<Attachment>().where("data = ?").bind(bytes).resultList();
            if (!contentSearchResults.empty())
                throw std::runtime_error("Identical file already exists.");

            auto attachmentDbo = _session.addNew<Attachment>();
            auto attachment = attachmentDbo.modify();

            attachment->name = name;
            attachment->mimeType = file.contentType();
            attachment->created = Wt::WDateTime::currentDateTime();
            attachment->data = std::move(bytes);
            t.commit();

            contents()->insertWidget(0, createItem(attachmentDbo));

            setStatusText(tr("str.attachmentSaved"));
            uploadProgressBar->setValue(0.0);
            uploadFileName->setText(Wt::WString::Empty);

            file.stealSpoolFile();
            fs::remove(file.spoolFileName());
        }
        catch (const std::exception& e)
        {
            setStatusText(tr("str.failedToSaveAttachment"));
        }
    });

    setStatusText(tr("str.idle"));

    resize(Wt::WLength("50%"), Wt::WLength("40%"));
}

std::unique_ptr<Wt::WTemplate> ManageAttachmentsDialog::createItem(const dbo::ptr<Attachment>& attachment)
{
    auto item = std::make_unique<Wt::WTemplate>(tr("manageAttachmentsWidgetView.item"));
    item->addFunction("tr", &Wt::WTemplate::Functions::tr);

    onUpdateAttachment(item.get(), attachment);

    auto onDeleteCallback = std::bind(&ManageAttachmentsDialog::onDeleteAttachment, this, item.get(), attachment);
    item->bindNew<Wt::WPushButton>("deleteButton", tr("str.delete"))->clicked().connect(item.get(), std::move(onDeleteCallback));

    return item;
}

void ManageAttachmentsDialog::onUpdateAttachment(Wt::WTemplate* item, const dbo::ptr<Attachment>& attachment) const
{
    auto id = std::to_string(attachment.id());
    auto iconLink = Wt::WLink(_session.relativePath("attachment-icon/" + id));

    auto iconImage = std::make_unique<Wt::WImage>(iconLink);
    iconImage->addStyleClass("img-responsive");

    auto attachmentLink = Wt::WLink(_session.relativePath("attachment/" + id + "/" + attachment->name.toUTF8()));
    attachmentLink.setTarget(Wt::LinkTarget::NewWindow);

    item->bindNew<Wt::WAnchor>("icon", attachmentLink, std::move(iconImage));
    item->bindString("itemId", id);
    item->bindString("created", attachment->created.toString("yyyy-MM-dd HH:MM:ss"));
    item->bindString("name", attachment->name);
    item->bindString("mimeType", attachment->mimeType);
    item->bindString("size", std::to_string(attachment->data.size()));
}

void ManageAttachmentsDialog::onDeleteAttachment(Wt::WTemplate* item, dbo::ptr<Attachment> attachment)
{
    try
    {
        dbo::Transaction t { _session };
        attachment.remove();
        t.commit();

        item->removeFromParent();
    }
    catch (const std::exception&)
    {
    }
}
