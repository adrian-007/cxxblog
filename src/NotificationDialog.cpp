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

#include "NotificationDialog.h"
#include <Wt/WMessageBox.h>

NotificationDialog::NotificationDialog(Wt::WWidget* parent, const Wt::WString& title, const Wt::WString& message, Wt::Icon icon, const std::function<void()>& callback)
    : Wt::WObject()
{
    auto dialog = parent->addChild(std::make_unique<Wt::WMessageBox>(title, message, icon, Wt::StandardButton::Ok));
    dialog->buttonClicked().connect(parent, [=]
    {
        // Remove actual dialog *and* wrapper instance
        parent->removeChild(dialog);
        parent->removeChild(this);

        if (callback != nullptr)
            callback();
    });

    dialog->setModal(true);
    dialog->show();
}

void NotificationDialog::show(Wt::WWidget* parent, const Wt::WString& title, const Wt::WString& message, Wt::Icon icon, const std::function<void()>& callback)
{
    parent->addChild(std::unique_ptr<NotificationDialog> { new NotificationDialog(parent, title, message, icon, callback) });
}
