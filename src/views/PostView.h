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

#include <Wt/WTemplate.h>

#include "models/Session.h"
#include "models/Post.h"

#include "CompositeWrapper.h"

class PostView
    : public CompositeWrapper<PostView>
    , public Wt::WTemplate
{
    friend class CompositeWrapper<PostView>;

    PostView(Session& session, dbo::ptr<Post> post);

    void createEditorToolbar();
    void showPostView();
    void showPostEditView();

    void onToggleVisibility();
    void onToggleEditSave();
    void onSelectDraft(const Wt::WString& buttonLabel, const dbo::ptr<PostDraft>& draft);
    void onSaveDraftAsPost();
    void onCloseEdit();
    void onDeletePost();
    void onManageAttachments();

    bool isEditing();
    std::vector<dbo::ptr<PostDraft>> invalidateDraftsMenu();

    template<typename PostType>
    void bindPost(Wt::WTemplate* view, const PostType& post);

    std::string imageExpression(const std::vector<std::string>& args) const;

    void setInfoMessage(const Wt::WString& message = {});

    Session& _session;
    dbo::ptr<Post> _post;
    dbo::ptr<PostDraft> _currentDraft;

    /**
     * Helper structure to avoid checking for null on every member. Instead, it
     * can be checked if parent (_editorControls) is present or not.
     */
    struct EditorControls
    {
        Wt::WText* infoMessage = nullptr;
        Wt::WPushButton* toggleVisibility = nullptr;
        Wt::WPushButton* toggleSaveEdit = nullptr;
        Wt::WPushButton* selectDraft = nullptr;
        Wt::WPushButton* saveDraftAsPost = nullptr;
        Wt::WPushButton* closeEdit = nullptr;
        Wt::WPushButton* deletePost = nullptr;
        Wt::WPushButton* manageAttachments = nullptr;
    };

    std::unique_ptr<EditorControls> _editorControls = nullptr;
};
