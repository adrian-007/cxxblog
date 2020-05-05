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

#include "PostView.h"
#include "Markdown.h"
#include "ExpressionParser.h"
#include "ValidatorUtils.h"
#include "NotificationDialog.h"
#include "ManageAttachmentsDialog.h"

#include "models/Attachment.h"

#include <Wt/WToolBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WApplication.h>
#include <Wt/WTemplate.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WFormModel.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPopupMenu.h>
#include <Wt/Utils.h>
#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WEnvironment.h>

#include <variant>
#include <numeric>

#include <boost/format.hpp>

namespace
{
    constexpr std::string_view g_DisqusScript = R"jsCode(
        DISQUS.reset({
            reload: true,
            config: function () {
                this.page.url = "%1%";
                this.page.identifier = "%2%";
            }
        });
    )jsCode";

    struct DraftFormModel : public Wt::WFormModel
    {
        static Field TitleField;
        static Field IntroField;
        static Field ContentField;

        DraftFormModel() : Wt::WFormModel()
        {
            addField(TitleField);
            addField(IntroField);
            addField(ContentField);

            setValidator(TitleField, MakeMandatoryValidator<Wt::WLengthValidator>(1, 100));
            setValidator(IntroField, MakeMandatoryValidator<Wt::WLengthValidator>(1, 600));
            setValidator(ContentField, MakeMandatoryValidator<Wt::WLengthValidator>(1, std::numeric_limits<int>::max()));
        }
    };

    DraftFormModel::Field DraftFormModel::TitleField = "title";
    DraftFormModel::Field DraftFormModel::IntroField = "intro";
    DraftFormModel::Field DraftFormModel::ContentField = "content";

    struct DraftFormView : public Wt::WTemplateFormView
    {
        DraftFormView(Session& session, dbo::ptr<Post> post, dbo::ptr<PostDraft> draft) :_session { session }, _model { std::make_unique<DraftFormModel>() }, _currentDraft { std::move(draft) }
        {
            if (!_currentDraft)
            {
                _currentDraft = dbo::ptr<PostDraft>(std::make_unique<PostDraft>(std::move(post)));
            }

            _model->setValue(DraftFormModel::TitleField, _currentDraft->title);
            _model->setValue(DraftFormModel::IntroField, _currentDraft->intro);
            _model->setValue(DraftFormModel::ContentField, _currentDraft->content);

            setTemplateText(tr("postView.edit"));
            addFunction("id", &Wt::WTemplate::Functions::id);

            setFormWidget(DraftFormModel::TitleField, std::make_unique<Wt::WLineEdit>());
            setFormWidget(DraftFormModel::IntroField, std::make_unique<Wt::WTextArea>());
            setFormWidget(DraftFormModel::ContentField, std::make_unique<Wt::WTextArea>());

            updateView(_model.get());
        }

        std::variant<dbo::ptr<Post>, dbo::ptr<PostDraft>, bool> save()
        {
            // First, update the model with values from the form
            updateModel(_model.get());
            // Then, validate the model.
            auto isValid = _model->validate();
            // Now we need to update the view in case validated field have changed their state.
            updateView(_model.get());

            try
            {
                if (!isValid)
                    throw std::runtime_error("Form is not valid");

                auto title = _model->valueText(DraftFormModel::TitleField).trim();
                auto intro = _model->valueText(DraftFormModel::IntroField).trim();
                auto content = _model->valueText(DraftFormModel::ContentField).trim();

                dbo::Transaction t { _session };
                auto draftDbo { _currentDraft };

                if (!draftDbo->post)
                {
                    auto postDbo = _session.addNew<Post>();
                    auto post { postDbo.modify() };

                    post->author = _session.editor();
                    post->created = Wt::WDateTime::currentDateTime();
                    post->visibility = Post::Visibility::Hidden;

                    post->title = title;
                    post->intro = intro;
                    post->content = content;

                    draftDbo.remove();
                    t.commit();

                    return postDbo;
                }
                else
                {
                    auto draftChanged = title != draftDbo->title || intro != draftDbo->intro || content != draftDbo->content;

                    if (draftChanged)
                    {
                        auto targetDraftDbo { _session.addNew<PostDraft>(std::move(draftDbo)) };
                        auto draft { targetDraftDbo.modify() };

                        draft->title = title;
                        draft->intro = intro;
                        draft->content = content;
                        draft->created = Wt::WDateTime::currentDateTime();

                        t.commit();

                        return targetDraftDbo;
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }

            return false;
        }

    private:
        Session& _session;
        std::unique_ptr<DraftFormModel> _model;

        dbo::ptr<PostDraft> _currentDraft;
    };
}

PostView::PostView(Session& session, dbo::ptr<Post> post)
    : Wt::WTemplate(tr("postView"))
    , _session(session)
    , _post(std::move(post))
{
    try
    {
        dbo::Transaction t { _session };
        _session.validateEditorLoginState();

        _editorControls = std::make_unique<EditorControls>();
        createEditorToolbar();

        if (!_post)
        {
            showPostEditView();
        }
        else
        {
            auto latestDrafts { _post->latestDrafts() };
            if (!latestDrafts.empty())
            {
                _currentDraft = latestDrafts.front();

                if (_currentDraft->title == _post->title && _currentDraft->intro == _post->intro && _currentDraft->content == _post->content)
                    _currentDraft = {};
            }

            showPostView();
        }
    }
    catch (const AccessDeniedException&)
    {
        dbo::Transaction t { _session };
        if (!_post)
            throw PageNotFoundException(wApp->internalPath());

        bindEmpty("toolbar");
        bindEmpty("infoMessage");

        showPostView();
    }
}

void PostView::createEditorToolbar()
{
    assert(_editorControls != nullptr);

    auto container = bindNew<Wt::WTemplate>("toolbar", tr("postView.toolbar"));
    auto toolbar = container->bindNew<Wt::WToolBar>("toolbar");

    auto addButton = [&](const char* css, const Wt::WString& title = { }, Wt::AlignmentFlag alignment = Wt::AlignmentFlag::Left)
    {
        auto button = std::make_unique<Wt::WPushButton>(title);
        auto buttonPtr = button.get();

        button->addStyleClass(css);
        button->setText(title);

        toolbar->addButton(std::move(button), alignment);

        return buttonPtr;
    };

    // Create / Edit button.
    _editorControls->toggleSaveEdit = addButton("btn-primary", tr(_post ? "str.edit" : "str.create"));
    _editorControls->toggleSaveEdit->clicked().connect(this, &PostView::onToggleEditSave);

    toolbar->addSeparator();

    // Publish / Hide post button.
    _editorControls->toggleVisibility = addButton("btn-primary", tr(!_post || _post->visibility != Post::Visibility::Published ? "str.publishPost" : "str.hidePost"));
    _editorControls->toggleVisibility->clicked().connect(this, &PostView::onToggleVisibility);

    toolbar->addSeparator();

    // Manage attachments.
    _editorControls->manageAttachments = addButton("btn-primary", tr("str.manageAttachments"));
    _editorControls->manageAttachments->clicked().connect(this, &PostView::onManageAttachments);

    toolbar->addSeparator();

    // Cancel create / Cancel edit button.
    _editorControls->cancelEdit = addButton("btn-warning", tr("str.cancel"));
    _editorControls->cancelEdit->clicked().connect(this, &PostView::onCancelEdit);

    toolbar->addSeparator();

    // Delete post button.
    _editorControls->deletePost = addButton("btn-danger", tr("str.deletePost"), Wt::AlignmentFlag::Right);
    _editorControls->deletePost->clicked().connect(this, &PostView::onDeletePost);

    toolbar->addSeparator();

    // Switch drafts button.
    _editorControls->selectDraft = addButton("btn-info", "", Wt::AlignmentFlag::Right);
    _editorControls->selectDraft->setMenu(std::make_unique<Wt::WPopupMenu>());

    toolbar->addSeparator();

    // Save draft as post button.
    _editorControls->saveDraftAsPost = addButton("btn-primary", tr("str.saveDraftAsPost"), Wt::AlignmentFlag::Right);
    _editorControls->saveDraftAsPost->clicked().connect(this, &PostView::onSaveDraftAsPost);

    // Setup draft info area below toolbar.
    auto infoMessageContainer = bindNew<Wt::WTemplate>("infoMessage", tr("postView.infoMessage"));
    _editorControls->infoMessage = infoMessageContainer->bindNew<Wt::WText>("infoMessage");
}

void PostView::showPostView()
{
    auto view = bindNew<Wt::WTemplate>("view", tr("postView.show"));

    if (!_post)
    {
        // This should be assert instead (or redirect to main page)
        view->bindEmpty("title");
        view->bindEmpty("content");
        view->bindEmpty("created");
        view->bindEmpty("author");
        return;
    }

    try
    {
        _session.validateEditorLoginState();

        if (_editorControls == nullptr)
            throw std::logic_error("_editorControls == nullptr");

        dbo::Transaction t { _session };

        invalidateDraftsMenu();

        _editorControls->cancelEdit->hide();
        _editorControls->manageAttachments->hide();

        _editorControls->deletePost->show();
        _editorControls->toggleVisibility->show();

        if (_currentDraft)
        {
            setInfoMessage(tr("str.draftInfo").arg(_currentDraft->created.toString()));
            _editorControls->saveDraftAsPost->show();

            bindPost(view, _currentDraft);
        }
        else
        {
            setInfoMessage(tr("str.postInfo").arg(_post->created.toString()));
            _editorControls->saveDraftAsPost->hide();

            bindPost(view, _post);
        }
    }
    catch (const AccessDeniedException&)
    {
        bindPost(view, _post);
    }
    catch (const std::exception& e)
    {
        setInfoMessage(e.what());
    }

    view->doJavaScript("$('#" + view->id() + "').find('code[class*=language-], pre[class*=language-]').each(function() { hljs.highlightBlock(this); $(this).removeClass('hljs'); });");
}

void PostView::showPostEditView()
{
    dbo::Transaction t { _session };

    bindNew<DraftFormView>("view", _session, _post, _currentDraft);
    invalidateDraftsMenu();

    // Hides info message block.
    setInfoMessage();

    if (_editorControls != nullptr)
    {
        _editorControls->cancelEdit->show();
        _editorControls->manageAttachments->show();

        _editorControls->deletePost->hide();
        _editorControls->toggleVisibility->hide();
        _editorControls->saveDraftAsPost->hide();
    }
}

void PostView::onToggleVisibility()
{
    try
    {
        dbo::Transaction t { _session };
        auto post = _post.modify();
        Wt::WString notificationMessage;

        if (post->visibility == Post::Visibility::Published)
        {
            post->visibility = Post::Visibility::Hidden;
            _editorControls->toggleVisibility->setText(tr("str.publishPost"));
            notificationMessage = tr("str.postHiddenNotificationMessage");
        }
        else
        {
            post->visibility = Post::Visibility::Published;

            if (!post->published.isValid())
                post->published = Wt::WDateTime::currentDateTime();

            _editorControls->toggleVisibility->setText(tr("str.hidePost"));
            notificationMessage = tr("str.postPublishedNotificationMessage");
        }

        t.commit();

        NotificationDialog::show(this, tr("str.postVisibilityStatusUpdatedNotificationTitle"), notificationMessage);
    }
    catch (const std::exception&)
    {
        NotificationDialog::show(this, tr("str.unexpectedErrorNotificationTitle"), tr("str.unexpectedErrorNotificationMessage"), Wt::Icon::Critical);
    }
}

void PostView::onToggleEditSave()
{
    bool wasEditing = false;
    if (auto draftFormView = resolve<DraftFormView*>("view"))
    {
        wasEditing = true;

        try
        {
            auto result = draftFormView->save();

            if (std::holds_alternative<dbo::ptr<Post>>(result))
            {
                auto post = std::get<dbo::ptr<Post>>(result);

                if (post)
                {
                    _post = std::move(post);
                    _currentDraft = {};

                    NotificationDialog::show(this, tr("str.postSavedNotificationTitle"), tr("str.postSavedNotificationMessage"), Wt::Icon::Information, [=]
                    {
                        wApp->setInternalPath(_post->url(), true);
                    });

                    return;
                }
            }
            else if (std::holds_alternative<dbo::ptr<PostDraft>>(result))
            {
                auto draft = std::get<dbo::ptr<PostDraft>>(result);

                if (draft)
                {
                    _post = draft->post;
                    _currentDraft = std::move(draft);

                    NotificationDialog::show(this, tr("str.postSavedNotificationTitle"), tr("str.postSavedNotificationMessage"));
                }
            }
            else
            {
                // Either there were no changes or something unrecoverable has happened.
                onCancelEdit();
                return;
            }
        }
        catch (...)
        {
            return;
        }
    }

    _editorControls->toggleVisibility->enable();
    _editorControls->deletePost->enable();

    // Now we need to finish editing.
    if (wasEditing)
    {
        showPostView();
        _editorControls->toggleSaveEdit->setText(tr("str.edit"));
        _editorControls->cancelEdit->hide();
    }
    else
    {
        showPostEditView();
        _editorControls->toggleSaveEdit->setText(tr("str.save"));
        _editorControls->cancelEdit->show();
    }
}

void PostView::onSelectDraft(const Wt::WString& buttonLabel, const dbo::ptr<PostDraft>& draft)
{
    _currentDraft = draft;

    _editorControls->selectDraft->setText(buttonLabel);
    _currentDraft ? _editorControls->saveDraftAsPost->show() : _editorControls->saveDraftAsPost->hide();

    isEditing() ? showPostEditView() : showPostView();
}

void PostView::onSaveDraftAsPost()
{
    try
    {
        if (!_post)
            throw std::logic_error("_post == nullptr");

        if (!_currentDraft)
            throw std::logic_error("_currentDraft == nullptr");

        dbo::Transaction t { _session };

        auto draft = _currentDraft.modify();
        auto post = _post.modify();

        post->title = draft->title;
        post->intro = draft->intro;
        post->content = draft->content;

        t.commit();

        _currentDraft = {};

        NotificationDialog::show(this, tr("str.draftSavedAsPostNotificationTitle"), tr("str.draftSavedAsPostNotificationMessage"));
    }
    catch (const std::exception& e)
    {
        setInfoMessage(e.what());
    }

    isEditing() ? showPostEditView() : showPostView();
}

void PostView::onCancelEdit()
{
    _editorControls->toggleVisibility->enable();
    _editorControls->deletePost->enable();
    _editorControls->cancelEdit->hide();

    _editorControls->toggleSaveEdit->setText(tr("str.edit"));

    showPostView();

    if (!_post)
        wApp->setInternalPath("/", true);
}

void PostView::onDeletePost()
{
    if (!_post)
        return;

    auto confirmationDialog = addChild(std::make_unique<Wt::WMessageBox>(
            tr("str.confirmationRequired"),
            tr("str.confirmPostDeleteMessage"),
            Wt::Icon::Question,
            Wt::StandardButton::Yes | Wt::StandardButton::No
    ));

    confirmationDialog->buttonClicked().connect(this, [=]
    {
        if (confirmationDialog->buttonResult() == Wt::StandardButton::Yes)
        {
            try
            {
                dbo::Transaction t { _session };
                _post.remove();
                t.commit();

                wApp->setInternalPath("/", true);
            }
            catch (const std::exception&)
            {
            }
        }

        removeChild(confirmationDialog);
    });

    confirmationDialog->setModal(true);
    confirmationDialog->show();
}

void PostView::onManageAttachments()
{
    auto dialog = addChild(std::make_unique<ManageAttachmentsDialog>(_session));

    dialog->finished().connect(this, [=] { removeChild(dialog); });
    dialog->show();
}

bool PostView::isEditing()
{
    return resolve<DraftFormView*>("view") != nullptr;
}

std::vector<dbo::ptr<PostDraft>> PostView::invalidateDraftsMenu()
{
    dbo::Transaction t { _session };
    auto latestDrafts { _post ? _post->latestDrafts() : std::vector<dbo::ptr<PostDraft>> { }};

    if (_editorControls == nullptr)
        return latestDrafts;

    if (latestDrafts.empty())
    {
        _editorControls->selectDraft->hide();
        return latestDrafts;
    }

    auto menu = _editorControls->selectDraft->menu();
    for (auto item : menu->items())
        menu->removeItem(item);

    _editorControls->selectDraft->show();

    auto menuItemText = [](auto prefixTrId, auto p)
    {
        return Wt::WString(tr("str.draftsMenuItemText"))
            .arg(tr(prefixTrId))
            .arg(p->created.toString("yyyy-MM-dd HH:MM", false));
    };

    auto item = menu->addItem(menuItemText("str.post", _post));
    item->triggered().connect(this, std::bind(&PostView::onSelectDraft, this, item->text(), dbo::ptr<PostDraft> {}));

    menu->addSeparator();

    for (const auto& draft : latestDrafts)
    {
        item = menu->addItem(menuItemText("str.draft", draft));
        item->triggered().connect(this, std::bind(&PostView::onSelectDraft, this, item->text(), draft));
    }

    // Initial update of button value.
    _editorControls->selectDraft->setText(_currentDraft ? menuItemText("str.draft", _currentDraft) : menuItemText("str.post", _post));

    return latestDrafts;
}

template<typename PostType>
void PostView::bindPost(Wt::WTemplate* view, const PostType& post)
{
    assert(post);

    struct PostResolver
    {
        static auto id(const dbo::ptr<Post>& post) { return post.id(); }
        static auto id(const dbo::ptr<PostDraft>& draft) { return draft->post.id(); }

        static auto author(const dbo::ptr<Post>& post) { return post->author; }
        static auto author(const dbo::ptr<PostDraft>& draft) { return draft->post->author; }

        static auto created(const dbo::ptr<Post>& post) { return post->published.isValid() ? post->published : post->created; }
        static auto created(const dbo::ptr<PostDraft>& draft) { return draft->created; }

        static auto url(const dbo::ptr<Post>& post) { return post->url(); }
        static auto url(const dbo::ptr<PostDraft>& draft) { return draft->post->url(); }
    };

    ExpressionParser expParser;
    expParser.registerFunction("image", [this](const auto& args)
    {
        return imageExpression(ExpressionParser::Expression::resolveArguments(args));
    });

    auto author = PostResolver::author(post);
    auto created = PostResolver::created(post).toString();

    Wt::WLink avatarLink { Wt::LinkType::Url, _session.relativePath({ "avatar", author->handle.toUTF8() }) };
    auto avatar = view->bindNew<Wt::WImage>("avatar", avatarLink);
    avatar->setMaximumSize(32.0, Wt::WLength("auto"));

    auto intro { Markdown(post->intro.toUTF8()).renderHTML() };
    auto content { Markdown(post->content.toUTF8()).renderHTML() };

    if (expParser.parse(content))
        content = expParser.resolve();

    view->bindString("title", Wt::Utils::htmlEncode(post->title));
    view->bindString("intro", intro);
    view->bindString("content", content);
    view->bindNew<Wt::WAnchor>("author", Wt::WLink(Wt::LinkType::InternalPath, author->url()), author->name);
    view->bindString("created", created);

    const auto& env { wApp->environment() };

    // TODO: https is hardcoded here, but in fact there is no way of telling whether we're running native TLS or
    // we are using plain HTTP behind a reverse proxy.
    auto absoluteUrl { "https://" + env.hostName() + _session.relativePath(PostResolver::url(post))};
    auto shareButtonsView = view->bindNew<Wt::WTemplate>("shareButtons", tr("postView.shareButtons"));

    shareButtonsView->addFunction("tr", &Wt::WTemplate::Functions::tr);
    shareButtonsView->bindString("postUrl", absoluteUrl);
    shareButtonsView->bindString("encodedPostUrl", Wt::Utils::urlEncode(absoluteUrl));

    if (!_session.siteConfig().disqusShortname().empty())
    {
        auto stubContainer = view->bindNew<Wt::WContainerWidget>("comments");
        stubContainer->setId("disqus_thread");

        auto disqusScript { boost::format(std::string { g_DisqusScript }) % absoluteUrl % PostResolver::id(post) };
        view->doJavaScript(disqusScript.str());
    }
    else
    {
        view->bindEmpty("comments");
    }
}

std::string PostView::imageExpression(const std::vector<std::string>& args) const
{
    if (args.empty() || args.front().find_first_not_of("0123456789") != std::string::npos)
        return {};

    const auto& id { args.front() };

    std::string caption;

    if (args.size() > 1)
        caption = std::accumulate(args.begin() + 2, args.end(), *(args.begin() + 1), [](const auto& s1, const auto& s2) { return s1 + ", " + s2; });

    try
    {
#if 0 // Version when attachment is checked in the database.
        dbo::Transaction t { _session };

        auto query = _session.find<Attachment>().where("mimeType like 'image/%'").where("id = ?").bind(id);
        auto attachment = query.resultValue();

        if (attachment)
        {
            auto view { std::make_unique<Wt::WTemplate>(tr("expressions.image")) };
            view->bindString("imageLink", _session.relativePath("attachment/" + id));
            view->bindString("caption", caption);

            std::ostringstream ss;
            view->renderTemplate(ss);
            return ss.str();
        }
#else
        auto view { std::make_unique<Wt::WTemplate>(tr("expressions.image")) };
        view->bindString("imageLink", _session.relativePath({ "attachment", id }));

        if (!caption.empty())
            view->bindNew<Wt::WText>("caption", caption);
        else
            view->bindEmpty("caption");

        std::ostringstream ss;
        view->renderTemplate(ss);
        return ss.str();
#endif
    }
    catch (const std::exception& e)
    {
    }

    return {};
}

void PostView::setInfoMessage(const Wt::WString& message)
{
    if (_editorControls == nullptr)
        return;

    if (message.empty())
    {
        _editorControls->infoMessage->hide();
    }
    else
    {
        _editorControls->infoMessage->setText(message);
        _editorControls->infoMessage->show();
    }
}
