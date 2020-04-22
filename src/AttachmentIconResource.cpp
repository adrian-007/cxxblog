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

#include "AttachmentIconResource.h"
#include "ApplicationExceptions.h"

#include "models/BasicSession.h"
#include "models/Attachment.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <Wt/WRasterImage.h>
#include <Wt/WFont.h>
#include <Wt/WPainter.h>

AttachmentIconResource::AttachmentIconResource(dbo::SqlConnectionPool& connectionPool)
    : _connectionPool(connectionPool)
    , _unknowFileTypeIconData { iconDataFromFileType({}, {}) }
{
}

AttachmentIconResource::~AttachmentIconResource()
{
    beingDeleted();
}

void AttachmentIconResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
    try
    {
        auto id = request.urlParam("id");

        if (id.empty())
            throw HTTPStatusException(404);

        BasicSession session { _connectionPool };
        dbo::Transaction t { session };

        auto attachment = session.find<Attachment>().where("id = ?").bind(id).resultValue();

        if (!attachment)
            throw HTTPStatusException(404);

        auto fileName = attachment->name.toUTF8();
        auto mimeType = attachment->mimeType.toUTF8();

        if (auto pos = fileName.rfind('.'); pos != std::string::npos)
            fileName = fileName.substr(pos + 1);
        else
            fileName.clear();

        std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::toupper);

        const auto& data = iconDataForFileType(fileName, mimeType);

        if (data.empty())
            throw HTTPStatusException(500);

        response.setMimeType("image/png");
        response.setStatus(200);
        response.setContentLength(data.size());
        response.addHeader("Cache-Control", "max-age=86400");
        response.out().write(reinterpret_cast<const char*>(&data[0]), data.size());
    }
    catch (const HTTPStatusException& e)
    {
        response.setStatus(e.status());
    }
    catch (const std::exception& e)
    {
        response.setStatus(500);
    }
}

std::vector<uint8_t> AttachmentIconResource::iconDataFromFileType(const std::string& fileExtension, const std::string& mimeType) const
{
    Wt::WRasterImage png("png", 304.0, 386.0);
    const Wt::WColor backgroundColor { 255, 255, 255 };
    const Wt::WColor foregroundColor { 70, 70, 70 };

    {
        const auto padding = 32.0;
        const Wt::WRectF drawBoundingBox { padding, padding, png.width().value() - (padding * 2), png.height().value() - (padding * 2)};

        Wt::WPainter p { &png };
        p.fillRect(0.0, 0.0, png.width().value(), png.height().value(), Wt::WBrush { Wt::StandardColor::Transparent });

        {
            Wt::WPainterPath pp;
            pp.moveTo(drawBoundingBox.left(), drawBoundingBox.top());

            pp.lineTo(drawBoundingBox.right() - (padding * 2), drawBoundingBox.top());
            pp.lineTo(drawBoundingBox.right(), drawBoundingBox.top() + (padding * 2));
            pp.lineTo(drawBoundingBox.right(), drawBoundingBox.bottom());
            pp.lineTo(drawBoundingBox.left(), drawBoundingBox.bottom());
            pp.lineTo(drawBoundingBox.left(), drawBoundingBox.top());

            p.fillPath(pp, Wt::WBrush { Wt::StandardColor::White });

            Wt::WPen pen { Wt::StandardColor::Black };
            pen.setWidth(2);
            pen.setCapStyle(Wt::PenCapStyle::Round);
            pen.setJoinStyle(Wt::PenJoinStyle::Bevel);

            p.setPen(pen);
            p.drawPath(pp);
        }

        {
            Wt::WFont font { Wt::FontFamily::SansSerif };
            font.setSize(Wt::FontSize::XXLarge);

            p.setFont(font);

            Wt::WPen pen { foregroundColor };
            pen.setCapStyle(Wt::PenCapStyle::Round);
            pen.setJoinStyle(Wt::PenJoinStyle::Round);

            p.setPen(pen);

            const auto height = 128.0;
            const Wt::WRectF textBoundingRect { padding, drawBoundingBox.bottom() - height, drawBoundingBox.width(), height };

            auto text = fileExtension;
            if (text.empty())
            {
                if (mimeType.compare(0, 5, "text/") == 0)
                    text = "TEXT";
                else if (mimeType.compare(0, 6, "image/") == 0)
                    text = "IMAGE";
                else
                    text = "BIN";
            }

            p.drawText(textBoundingRect, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Top, Wt::TextFlag::WordWrap, text);
        }
    }

    // TODO: I don't like it, it copies data twice. Optimize it.
    std::stringstream ss;
    png.write(ss);

    auto s = ss.str();

    std::vector<uint8_t> buffer;

    buffer.resize(s.size());
    std::memcpy(&buffer[0], &s[0], buffer.size());

    return buffer;
}

const std::vector<uint8_t>& AttachmentIconResource::iconDataForFileType(const std::string& fileExtension, const std::string& mimeType)
{
    if (fileExtension.empty())
        return _unknowFileTypeIconData;

    std::scoped_lock<std::mutex> lock { _extensionIconDataMapMutex };

    auto it = _extensionIconDataMap.find(fileExtension);
    if (it == _extensionIconDataMap.end())
        it = _extensionIconDataMap.insert(std::make_pair(fileExtension, iconDataFromFileType(fileExtension, mimeType))).first;

    return it->second;
}
