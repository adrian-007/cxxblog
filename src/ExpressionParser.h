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

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <string_view>

class ExpressionParser
{
public:
    struct Expression
    {
        using ArgType = std::variant<std::string, Expression>;
        using ArgumentList = std::vector<ArgType>;
        using ExpressionFunction = std::function<std::string(const ArgumentList&)>;

        [[nodiscard]] inline std::string exec() const
        {
            return function != nullptr ? function(args) : std::string {"#error#"};
        }

        static std::vector<std::string> resolveArguments(const ArgumentList& args);
    private:
        friend class ExpressionParser;

        Expression() = default;
        Expression(std::string::size_type start, std::string::size_type end);

        std::string::size_type start = std::string::npos;
        std::string::size_type end = std::string::npos;

        std::function<std::string(const ArgumentList&)> function;
        std::vector<std::variant<std::string, Expression>> args;

        [[nodiscard]] inline bool valid() const { return start != std::string::npos && end != std::string::npos; }
    };

    ExpressionParser();

    void registerFunction(std::string name, Expression::ExpressionFunction function);
    bool parse(std::string_view content);
    std::string resolve();

private:
    std::map<std::string, Expression::ExpressionFunction> _functions;
    std::vector<Expression> _expressions;

    [[nodiscard]] bool parseExpression(Expression& exp) const;
    [[nodiscard]] std::vector<Expression> findExpressions(const std::string_view& view, size_t offset = 0u) const;
    [[nodiscard]] Expression findNextExpression(const std::string_view& view, size_t offset = 0u) const;

    std::string_view _content;
};
