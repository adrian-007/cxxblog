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

#include "ExpressionParser.h"

#include <stack>
#include <cassert>

std::vector<std::string> ExpressionParser::Expression::resolveArguments(const ArgumentList& args)
{
    struct Visitor
    {
        std::string operator()(const std::string& value) { return value; }
        std::string operator()(const Expression& exp) { return exp.exec(); }
    };

    std::vector<std::string> results;
    Visitor v;

    for (const auto& arg : args)
        results.emplace_back(std::move(std::visit(v, arg)));

    return results;
}

ExpressionParser::Expression::Expression(std::string::size_type start, std::string::size_type end)
{
    assert(start < end);
    this->start = start;
    this->end = end;
}

ExpressionParser::ExpressionParser()
{
    // Register default expressions.
    registerFunction("echo", [](auto args)
    {
        std::string result;

        for (const auto& arg : Expression::resolveArguments(args))
        {
            if (!result.empty())
                result += " ";

            result += arg;
        }

        return result;
    });
}

void ExpressionParser::registerFunction(std::string name, Expression::ExpressionFunction function)
{
    assert(_functions.find(name) == _functions.end());
    _functions.insert(std::make_pair(std::move(name), std::move(function)));
}

bool ExpressionParser::parse(std::string_view content)
{
    _expressions.clear();
    _content = content;

    std::string_view contentView { &_content[0], _content.size() };
    auto expressions = findExpressions(contentView);

    for (auto& exp : expressions)
    {
        if (parseExpression(exp))
        {
            _expressions.emplace_back(std::move(exp));
        }
    }

    return !_expressions.empty();
}

std::string ExpressionParser::resolve()
{
    std::string content;
    content.reserve(_content.size() * 2);
    content.assign(_content);

    // Resolve in reverse order - makes string substitution easy.
    for (auto it = _expressions.rbegin(); it != _expressions.rend(); ++it)
    {
        const auto& exp = *it;
        auto result = exp.exec();
        content.replace(exp.start, exp.end - exp.start, result);
    }

    return content;
}

bool ExpressionParser::parseExpression(Expression& exp) const
{
    // View for expression body, skipping starting and ending tags (hence + 2 and - 3).
    std::string_view expView { &_content[exp.start + 2], exp.end - exp.start - 3 };

    std::string_view::size_type j;

    // First, extract function name.
    std::string name;
    name.reserve(32u);

    for (j = 0u; j < expView.size(); ++j)
    {
        auto c = expView[j];

        // Skip leading whitespaces.
        if (name.empty() && std::isblank(c))
            continue;

        // Expression names can only be alphanumeric.
        if (!std::isalnum(c))
            break;

        name += c;
    }

    // Expression cannot have empty name. If it does, simply return false.
    if (name.empty())
        return false;

    // Check if function corresponding with expression name exists.
    auto functionIt = _functions.find(name);
    if (functionIt == _functions.end())
        return false;

    // It does, copy it to expression object.
    exp.function = functionIt->second;

    // Sanity check that let us skip some j == 0 checks.
    assert(j > 0u);

    // Next, extract expression arguments (if any).
    std::string arg;
    arg.reserve(128u);

    for (; j < expView.size(); ++j)
    {
        auto c = expView[j];

        if (arg.empty())
        {
            // If there is a leading whitespace character, skip it.
            if (std::isblank(c))
                continue;

            if (c == '$' && expView[j - 1] != '\\' && (j + 1) < expView.size() && expView[j + 1] == '(')
            {
                // We have a sub-expression as an argument. Try to extract it.
                auto subExp = findNextExpression(expView, j);

                if (subExp.valid())
                {
                    // Fix relative position to absolute position. + 2 is for starting tag of parent expression (it was skipped when expView was created).
                    subExp.start += exp.start + 2;
                    subExp.end += exp.start + 2;

                    if (parseExpression(subExp))
                    {
                        // Update current position past parsed sub-expression.
                        j += subExp.end - subExp.start + 1;
                        exp.args.emplace_back(std::move(subExp));

                        arg.clear();
                        continue;
                    }
                }
            }
        }

        if (c == ',' && expView[j - 1] != '\\')
        {
            // Remove trailing whitespace characters, if any.
            auto pos = arg.size();
            while (pos > 0 && std::isblank(arg[pos - 1]))
                --pos;

            if (pos < arg.size())
                arg.erase(pos);

            exp.args.emplace_back(std::move(arg));
        }
        else
        {
            arg += c;
        }
    }

    // Check last argument.
    if (!arg.empty())
        exp.args.emplace_back(std::move(arg));

    return true;
}

std::vector<ExpressionParser::Expression> ExpressionParser::findExpressions(const std::string_view& view, size_t offset) const
{
    std::vector<Expression> expressions;

    while (true)
    {
        auto exp { std::move(findNextExpression(view, offset)) };
        if (!exp.valid())
            break;

        offset = exp.end;
        expressions.emplace_back(std::move(exp));
    }

    return expressions;
}

ExpressionParser::Expression ExpressionParser::findNextExpression(const std::string_view& view, size_t offset) const
{
    std::vector<Expression> expressions;
    std::stack<size_t> s;
    std::string::size_type i = offset;

    // Look for the first character of the starting tag or the last one. When found, they'll be put
    // on a stack
    while ((i = view.find_first_of("$)", i)) != std::string::npos)
    {
        switch (view[i])
        {
            case '$':
            {
                // $ is escaped, skip it.
                if (i > 0 && view[i - 1] == '\\')
                    break;

                // Next character is not (, skip it.
                if ((i + 1) >= view.size() || view[i + 1] != '(')
                    break;

                s.push(i);
                break;
            }
            case ')':
            {
                // There was no matching start tag, skip it.
                if (s.empty())
                    break;

                // This was escaped end tag, continue search.
                if (i > 0 && view[i - 1] == '\\')
                    break;

                if (s.size() == 1u)
                {
                    // At this point we have matching start and end tags of top-level expression.
                    return Expression { s.top(), i + 1 };
                }

                s.pop();
                break;
            }
        }

        ++i;
    }

    return Expression {};
}
