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

#include "PostsListView.h"
#include "Markdown.h"

#include <Wt/WText.h>
#include <Wt/WLink.h>
#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/Utils.h>

PostsListView::PostsListView(Session& session)
    : Wt::WTemplate(tr("postView.itemsList"))
    , _session(session)
{
    auto container = bindNew<Wt::WContainerWidget>("items");

    dbo::Transaction t { _session };
    auto query = _session.find<Post>();
    const auto isLoggedIn = _session.login().loggedIn();

    if (!isLoggedIn)
        query.where("visibility = ?").bind(Post::Visibility::Published);

    query.orderBy("id desc");

    for (const auto& post : query.resultList())
    {
        auto itemLink = Wt::WLink(Wt::LinkType::InternalPath, _session.relativePath(post->url()));

        auto item = container->addNew<Wt::WTemplate>(tr("postView.itemsList.item"));
        item->bindString("title", Wt::Utils::htmlEncode(post->title));
        item->bindString("intro", Markdown(post->intro.toUTF8()).renderHTML());
        item->bindWidget("link", std::make_unique<Wt::WAnchor>(itemLink, "Read more"));
        item->bindString("created", post->created.toString());
        item->bindString("author", post->author->name);

        if (isLoggedIn)
        {
            switch (post->visibility)
            {
                case Post::Visibility::Published:
                    item->bindNew<Wt::WText>("visibility", "Published")->addStyleClass("label label-info");
                    break;
                case Post::Visibility::Hidden:
                    item->bindNew<Wt::WText>("visibility", "Hidden")->addStyleClass("label label-warning");
                    break;
            }
        }
        else
        {
            item->bindEmpty("visibility");
        }
    }

    doJavaScript("$('#" + id() + "').find('code[class*=language-], pre[class*=language-]').each(function() { hljs.highlightBlock(this); $(this).removeClass('hljs'); });");
}
