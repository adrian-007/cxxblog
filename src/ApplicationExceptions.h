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

#include <stdexcept>
#include <string>

class ApplicationException
    : public std::exception
{
public:
    [[nodiscard]] const char* what() const noexcept override { return _what.c_str(); }

protected:
    explicit ApplicationException(std::string message) : _what(std::move(message)) {}

private:
    std::string _what;
};

struct AccessDeniedException
    : public ApplicationException
{
    AccessDeniedException() : ApplicationException("Access denied.") {}
    explicit AccessDeniedException(const std::string& s) : ApplicationException("Access denied: " + s) {}
};

struct PageNotFoundException
    : public ApplicationException
{
    PageNotFoundException() : ApplicationException("Page not found.") {}
    explicit PageNotFoundException(const std::string& s) : ApplicationException("Page not found: " + s) {}
};

class HTTPStatusException
    : public ApplicationException
{
public:
    explicit HTTPStatusException(int status, std::string message = "HTTP Exception") : ApplicationException(std::move(message)) , _status(status) {}

    [[nodiscard]] auto status() const { return _status; }

private:
    int _status;
};
