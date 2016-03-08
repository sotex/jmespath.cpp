/****************************************************************************
**
** Author: Róbert Márki <gsmiko@gmail.com>
** Copyright (c) 2016 Róbert Márki
**
** This file is part of the jmespath.cpp project which is distributed under
** the MIT License (MIT).
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#include "jmespath/interpreter/expressionevaluator.h"
#include "jmespath/ast/allnodes.h"
#include "jmespath/detail/exceptions.h"
#include <numeric>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string.hpp>

namespace jmespath { namespace interpreter {

namespace rng = boost::range;
namespace alg = boost::algorithm;

ExpressionEvaluator::ExpressionEvaluator(const Json &contextValue)
    : AbstractVisitor()
{
    setContext(contextValue);
    using std::placeholders::_1;
    using std::make_tuple;
    using std::bind;
    m_functionMap = {
        {"abs", make_tuple(1, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::abs, this, _1))},
        {"avg", make_tuple(1, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::avg, this, _1))},
        {"contains", make_tuple(2, std::equal_to<size_t>{},
                        bind(&ExpressionEvaluator::contains, this, _1))},
        {"ceil", make_tuple(1, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::ceil, this, _1))},
        {"ends_with", make_tuple(2, std::equal_to<size_t>{},
                        bind(&ExpressionEvaluator::endsWith, this, _1))},
        {"floor", make_tuple(1, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::floor, this, _1))},
        {"join", make_tuple(2, std::equal_to<size_t>{},
                  bind(&ExpressionEvaluator::join, this, _1))},
        {"keys", make_tuple(1, std::equal_to<size_t>{},
                  bind(&ExpressionEvaluator::keys, this, _1))},
        {"length", make_tuple(1, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::length, this, _1))},
        {"map", make_tuple(2, std::equal_to<size_t>{},
                 bind(&ExpressionEvaluator::map, this, _1))},
        {"max", make_tuple(1, std::equal_to<size_t>{},
                 bind(&ExpressionEvaluator::max, this, _1))},
        {"max_by", make_tuple(2, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::maxBy, this, _1))},
        {"merge", make_tuple(0, std::greater_equal<size_t>{},
                   bind(&ExpressionEvaluator::merge, this, _1))},
        {"min", make_tuple(1, std::equal_to<size_t>{},
                 bind(&ExpressionEvaluator::min, this, _1))},
        {"min_by", make_tuple(2, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::minBy, this, _1))},
        {"not_null", make_tuple(1, std::greater_equal<size_t>{},
                      bind(&ExpressionEvaluator::notNull, this, _1))},
        {"reverse", make_tuple(1, std::equal_to<size_t>{},
                     bind(&ExpressionEvaluator::reverse, this, _1))},
        {"sort", make_tuple(1, std::equal_to<size_t>{},
                  bind(&ExpressionEvaluator::sort, this, _1))},
        {"sort_by", make_tuple(2, std::equal_to<size_t>{},
                     bind(&ExpressionEvaluator::sortBy, this, _1))},
        {"starts_with", make_tuple(2, std::equal_to<size_t>{},
                        bind(&ExpressionEvaluator::startsWith, this, _1))},
        {"sum", make_tuple(1, std::equal_to<size_t>{},
                 bind(&ExpressionEvaluator::sum, this, _1))},
        {"to_array", make_tuple(1, std::equal_to<size_t>{},
                      bind(&ExpressionEvaluator::toArray, this, _1))},
        {"to_string", make_tuple(1, std::equal_to<size_t>{},
                       bind(&ExpressionEvaluator::toString, this, _1))},
        {"to_number", make_tuple(1, std::equal_to<size_t>{},
                       bind(&ExpressionEvaluator::toNumber, this, _1))},
        {"type", make_tuple(1, std::equal_to<size_t>{},
                  bind(&ExpressionEvaluator::type, this, _1))},
        {"values", make_tuple(1, std::equal_to<size_t>{},
                    bind(&ExpressionEvaluator::values, this, _1))}
    };
}

void ExpressionEvaluator::setContext(const Json &value)
{
    m_context = value;
}

Json ExpressionEvaluator::currentContext() const
{
    return m_context;
}

void ExpressionEvaluator::evaluateProjection(ast::ExpressionNode *expression)
{
    Json result;
    if (m_context.is_array())
    {
        result = Json(Json::value_t::array);
        Json contextArray = std::move(m_context);
        for (auto& item: contextArray)
        {
            m_context = std::move(item);
            visit(expression);
            if (!m_context.is_null())
            {
                result.push_back(std::move(m_context));
            }
        }
    }
    m_context = std::move(result);
}

void ExpressionEvaluator::visit(ast::AbstractNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::ExpressionNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::IdentifierNode *node)
{
    Json result;
    if (m_context.is_object())
    {
        result = std::move(m_context[node->identifier]);
    }
    m_context = std::move(result);
}

void ExpressionEvaluator::visit(ast::RawStringNode *node)
{
    m_context = std::move(node->rawString);
}

void ExpressionEvaluator::visit(ast::LiteralNode *node)
{
    m_context = Json::parse(node->literal);
}

void ExpressionEvaluator::visit(ast::SubexpressionNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::IndexExpressionNode *node)
{
    visit(&node->leftExpression);
    if (m_context.is_array())
    {
        visit(&node->bracketSpecifier);
        if (node->isProjection())
        {
            evaluateProjection(&node->rightExpression);
        }
    }
    else
    {
        m_context = {};
    }
}

void ExpressionEvaluator::visit(ast::ArrayItemNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        int arrayIndex = node->index;
        if (arrayIndex < 0)
        {
            arrayIndex += m_context.size();
        }
        if ((arrayIndex >= 0) && (arrayIndex < m_context.size()))
        {
            result = std::move(m_context[arrayIndex]);
        }
    }
    m_context = std::move(result);
}

void ExpressionEvaluator::visit(ast::FlattenOperatorNode *)
{
    Json result;
    if (m_context.is_array())
    {
        result = Json(Json::value_t::array);
        for (auto& item: m_context)
        {
            if (item.is_array())
            {
                std::move(std::begin(item),
                          std::end(item),
                          std::back_inserter(result));
            }
            else
            {
                result.push_back(std::move(item));
            }
        }
    }
    m_context = std::move(result);
}

void ExpressionEvaluator::visit(ast::BracketSpecifierNode *node)
{
    node->accept(this);
}

void ExpressionEvaluator::visit(ast::SliceExpressionNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        int startIndex = 0;
        int endIndex = 0;
        int step = 1;
        int length = m_context.size();

        if (node->step)
        {
            if (*node->step == 0)
            {
                BOOST_THROW_EXCEPTION(detail::InvalidValue{});
            }
            step = *node->step;
        }
        if (!node->start)
        {
            startIndex = step < 0 ? length - 1: 0;
        }
        else
        {
            startIndex = adjustSliceEndpoint(length, *node->start, step);
        }
        if (!node->stop)
        {
            endIndex = step < 0 ? -1 : length;
        }
        else
        {
            endIndex = adjustSliceEndpoint(length, *node->stop, step);
        }

        result = Json(Json::value_t::array);
        auto beginIt = std::begin(m_context);
        auto it = beginIt + startIndex;
        auto stopIt = beginIt + endIndex;

        while (((step > 0) && (it < stopIt))
               || ((step < 0) && (it > stopIt)))
        {
            result.push_back(std::move(*it));
            it += step;
        }
    }
    m_context = std::move(result);
}

void ExpressionEvaluator::visit(ast::ListWildcardNode *node)
{
    if (!m_context.is_array())
    {
        m_context = {};
    }
}

void ExpressionEvaluator::visit(ast::HashWildcardNode *node)
{
    visit(&node->leftExpression);
    Json result;
    if (m_context.is_object())
    {
        std::move(std::begin(m_context),
                  std::end(m_context),
                  std::back_inserter(result));
    }
    m_context = std::move(result);
    evaluateProjection(&node->rightExpression);
}

void ExpressionEvaluator::visit(ast::MultiselectListNode *node)
{
    if (!m_context.is_null())
    {
        Json result(Json::value_t::array);
        Json childContext = std::move(m_context);
        for (auto& expression: node->expressions)
        {
            m_context = childContext;
            visit(&expression);
            result.push_back(std::move(m_context));
        }
        m_context = std::move(result);
    }
}

void ExpressionEvaluator::visit(ast::MultiselectHashNode *node)
{
    if (!m_context.is_null())
    {
        Json result(Json::value_t::object);
        Json childContext = std::move(m_context);
        for (auto& keyValuePair: node->expressions)
        {
            m_context = childContext;
            visit(&keyValuePair.second);
            result[keyValuePair.first.identifier] = std::move(m_context);
        }
        m_context = std::move(result);
    }
}

void ExpressionEvaluator::visit(ast::NotExpressionNode *node)
{
    visit(&node->expression);
    m_context = !toBoolean(m_context);
}

void ExpressionEvaluator::visit(ast::ComparatorExpressionNode *node)
{
    using Comparator = ast::ComparatorExpressionNode::Comparator;

    if (node->comparator == Comparator::Unknown)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidAgrument{});
    }

    Json childContext = m_context;
    visit(&node->leftExpression);
    Json leftResult = std::move(m_context);

    m_context = std::move(childContext);
    visit(&node->rightExpression);
    Json rightResult = std::move(m_context);

    if (node->comparator == Comparator::Less)
    {
        m_context = leftResult < rightResult;
    }
    else if (node->comparator == Comparator::LessOrEqual)
    {
        m_context = leftResult <= rightResult;
    }
    else if (node->comparator == Comparator::Equal)
    {
        m_context = leftResult == rightResult;
    }
    else if (node->comparator == Comparator::GreaterOrEqual)
    {
        m_context = leftResult >= rightResult;
    }
    else if (node->comparator == Comparator::Greater)
    {
        m_context = leftResult > rightResult;
    }
    else if (node->comparator == Comparator::NotEqual)
    {
        m_context = leftResult != rightResult;
    }
}

void ExpressionEvaluator::visit(ast::OrExpressionNode *node)
{
    Json childContext = m_context;
    visit(&node->leftExpression);
    if (!toBoolean(m_context))
    {
        m_context = std::move(childContext);
        visit(&node->rightExpression);
    }
}

void ExpressionEvaluator::visit(ast::AndExpressionNode *node)
{
    Json childContext = m_context;
    visit(&node->leftExpression);
    if (toBoolean(m_context))
    {
        m_context = std::move(childContext);
        visit(&node->rightExpression);
    }
}

void ExpressionEvaluator::visit(ast::ParenExpressionNode *node)
{
    visit(&node->expression);
}

void ExpressionEvaluator::visit(ast::PipeExpressionNode *node)
{
    visit(&node->leftExpression);
    visit(&node->rightExpression);
}

void ExpressionEvaluator::visit(ast::CurrentNode *)
{
}

void ExpressionEvaluator::visit(ast::FilterExpressionNode *node)
{
    Json result;
    if (m_context.is_array())
    {
        result = Json(Json::value_t::array);
        Json contextArray = m_context;
        for (const auto& item: contextArray)
        {
            m_context = item;
            visit(&node->expression);
            if (toBoolean(m_context))
            {
                result.push_back(std::move(item));
            }
        }
    }
    m_context = std::move(result);
}

void ExpressionEvaluator::visit(ast::FunctionExpressionNode *node)
{
    auto it = m_functionMap.find(node->functionName);
    if (it == m_functionMap.end())
    {
        BOOST_THROW_EXCEPTION(detail::UnknownFunction()
                              << detail::InfoFunctionName(node->functionName));
    }

    const auto& descriptor = it->second;
    size_t expectedArgumentCount = std::get<0>(descriptor);
    const auto& sizeComparator = std::get<1>(descriptor);
    const auto& function = std::get<2>(descriptor);
    if (!sizeComparator(node->arguments.size(), expectedArgumentCount))
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentArity());
    }

    FunctionArgumentList argumentList = evaluateArguments(node->arguments);
    m_context = function(argumentList);
}

void ExpressionEvaluator::visit(ast::ExpressionArgumentNode *)
{
}

int ExpressionEvaluator::adjustSliceEndpoint(int length,
                                             int endpoint,
                                             int step) const
{
    if (endpoint < 0)
    {
        endpoint += length;
        if (endpoint < 0)
        {
            endpoint = step < 0 ? -1 : 0;
        }
    }
    else if (endpoint >= length)
    {
        endpoint = step < 0 ? length - 1: length;
    }
    return endpoint;
}

bool ExpressionEvaluator::toBoolean(const Json &json) const
{
    return json.is_number()
            || ((!json.is_boolean() || json.get<bool>())
                && (!json.is_string()
                    || !json.get_ptr<const std::string*>()->empty())
                && !json.empty());
}

ExpressionEvaluator::FunctionArgumentList
ExpressionEvaluator::evaluateArguments(
        const FunctionExpressionArgumentList &arguments)
{
    FunctionArgumentList argumentList;
    rng::transform(arguments, std::back_inserter(argumentList),
                   [this](auto argument)
    {
        FunctionArgument functionArgument;

        ast::ExpressionNode* expressionArgument
                = boost::get<ast::ExpressionNode>(&argument);
        if (expressionArgument)
        {
            auto context = m_context;
            this->visit(expressionArgument);
            functionArgument = std::move(m_context);
            m_context = std::move(context);
        }

        ast::ExpressionArgumentNode* expressionTypeArgument
                = boost::get<ast::ExpressionArgumentNode>(&argument);
        if (expressionTypeArgument)
        {
            functionArgument = std::move(expressionTypeArgument->expression);
        }

        return functionArgument;
    });
    return argumentList;
}

Json ExpressionEvaluator::abs(const FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value || !value->is_number())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    if (value->is_number_integer())
    {
        return std::abs(value->get<Json::number_integer_t>());
    }
    else
    {
        return std::abs(value->get<Json::number_float_t>());
    }
}

Json ExpressionEvaluator::avg(const FunctionArgumentList &arguments) const
{
    const Json* items = boost::get<Json>(&arguments[0]);
    if (items && items->is_array())
    {
        double sum = std::accumulate(items->cbegin(),
                                     items->cend(),
                                     0.0,
                                     [](double sum, const Json& item) -> double
        {
            if (item.is_number_integer())
            {
                return sum + item.get<Json::number_integer_t>();
            }
            else if (item.is_number_float())
            {
                return sum + item.get<Json::number_float_t>();
            }
            else
            {
                BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
            }
        });
        return sum / items->size();
    }
    else
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
}

Json ExpressionEvaluator::contains(const FunctionArgumentList &arguments) const
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    const Json* item = boost::get<Json>(&arguments[1]);
    if (!subject || (!subject->is_array() && !subject->is_string()) || !item)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    bool result = false;
    if (subject->is_array())
    {
        auto it = rng::find(*subject, *item);
        result = (it != std::end(*subject));
    }
    else if (subject->is_string())
    {
        auto stringSubject = subject->get_ptr<const String*>();
        auto stringItem = item->get_ptr<const String*>();
        result = boost::contains(*stringSubject, *stringItem);
    }
    return result;
}

Json ExpressionEvaluator::ceil(const FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value || !value->is_number())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    if (value->is_number_integer())
    {
        return *value;
    }
    else
    {
        return std::ceil(value->get<Json::number_float_t>());
    }
}

Json ExpressionEvaluator::endsWith(const FunctionArgumentList &arguments) const
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    const Json* suffix = boost::get<Json>(&arguments[1]);
    if (!subject || !subject->is_string() || !suffix || !suffix->is_string())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    auto stringSubject = subject->get_ptr<const String*>();
    auto stringSuffix = suffix->get_ptr<const String*>();
    return boost::ends_with(*stringSubject, *stringSuffix);
}

Json ExpressionEvaluator::floor(const FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value || !value->is_number())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    if (value->is_number_integer())
    {
        return *value;
    }
    else
    {
        return std::floor(value->get<Json::number_float_t>());
    }
}

Json ExpressionEvaluator::join(const FunctionArgumentList &arguments) const
{
    const Json* glue = boost::get<Json>(&arguments[0]);
    const Json* array = boost::get<Json>(&arguments[1]);
    if (!glue || !glue->is_string() || !array || !array->is_array()
       || alg::any_of(*array, [](const auto& item) {return !item.is_string();}))
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    const String* glueString = glue->get_ptr<const String*>();
    return boost::accumulate(*array, String{},
                             [&](auto& string, const Json& value)
    {
        return string.empty() ?
                    value.get<String>() :
                    string += *glueString + value.get<String>();
    });
}

Json ExpressionEvaluator::keys(const FunctionArgumentList &arguments) const
{
    const Json* object = boost::get<Json>(&arguments[0]);
    if (!object || !object->is_object())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    Json result(Json::value_t::array);
    for (auto it = object->cbegin(); it != object->cend(); ++it)
    {
        result.push_back(it.key());
    }
    return result;
}

Json ExpressionEvaluator::length(const FunctionArgumentList &arguments) const
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    if (!subject || !(subject->is_array() || subject->is_object()
                      || subject->is_string()))
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    size_t result{};
    if (subject->is_string())
    {
        result = subject->get_ptr<const String*>()->size();
    }
    else
    {
        result = subject->size();
    }
    return result;
}

Json ExpressionEvaluator::map(FunctionArgumentList &arguments)
{
    ast::ExpressionNode* expression
            = boost::get<ast::ExpressionNode>(&arguments[0]);
    Json* array = boost::get<Json>(&arguments[1]);
    if (!expression || !array || !array->is_array())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    rng::for_each(*array, [this, expression](Json& item)
    {
        m_context = std::move(item);
        this->visit(expression);
        item = std::move(m_context);
    });
    return *array;
}

Json ExpressionEvaluator::max(const FunctionArgumentList &arguments) const
{
    return maxElement(arguments);
}

Json ExpressionEvaluator::maxBy(FunctionArgumentList &arguments)
{
    return maxElementBy(arguments);
}

Json ExpressionEvaluator::merge(FunctionArgumentList &arguments) const
{
    Json result(Json::value_t::object);
    rng::for_each(arguments, [&](auto& argument)
    {
        Json* object = boost::get<Json>(&argument);
        if (!object || !object->is_object())
        {
            BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
        }
        for (auto it = std::begin(*object); it != std::end(*object); ++it)
        {
            result[it.key()] = std::move(*it);
        }
    });
    return result;
}

Json ExpressionEvaluator::min(const FunctionArgumentList &arguments) const
{
    return maxElement(arguments, std::greater<Json>{});
}

Json ExpressionEvaluator::minBy(FunctionArgumentList &arguments)
{    
    return maxElementBy(arguments, std::greater<Json>{});
}

Json ExpressionEvaluator::notNull(FunctionArgumentList &arguments)
{
    Json result;
    for (auto& argument: arguments)
    {
        Json* item = boost::get<Json>(&argument);
        if (!item)
        {
            BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
        }
        if (!item->is_null())
        {
            result = std::move(*item);
            break;
        }
    }
    return result;
}

Json ExpressionEvaluator::reverse(FunctionArgumentList &arguments) const
{
    Json* subject = boost::get<Json>(&arguments[0]);
    if (!subject || !(subject->is_array() || subject->is_string()))
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    if (subject->is_array())
    {
        rng::reverse(*subject);
    }
    else if (subject->is_string())
    {
        rng::reverse(*subject->get_ptr<String*>());
    }
    return *subject;
}

Json ExpressionEvaluator::sort(FunctionArgumentList &arguments) const
{
    Json* array = boost::get<Json>(&arguments[0]);
    auto notNumberOrString = [](const Json& json) -> bool
    {
        return !(json.is_number() || json.is_string());
    };
    if (!array || !array->is_array() || alg::any_of(*array, notNumberOrString))
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    std::sort(std::begin(*array), std::end(*array));
    return *array;
}

Json ExpressionEvaluator::sortBy(FunctionArgumentList &arguments)
{
    Json* array = boost::get<Json>(&arguments[0]);
    ast::ExpressionNode* expression
            = boost::get<ast::ExpressionNode>(&arguments[1]);
    if (!array || !expression || !array->is_array())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    std::sort(std::begin(*array), std::end(*array),
              [&, this](auto& first, auto& second) -> bool
    {
        m_context = first;
        this->visit(expression);
        if (!(m_context.is_number() || m_context.is_string()))
        {
            BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
        }
        auto firstResult = std::move(m_context);

        m_context = second;
        this->visit(expression);
        if (!(m_context.is_number() || m_context.is_string()))
        {
            BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
        }

        return firstResult < m_context;
    });
    return *array;
}

Json ExpressionEvaluator::startsWith(
        const FunctionArgumentList &arguments) const
{
    const Json* subject = boost::get<Json>(&arguments[0]);
    const Json* prefix = boost::get<Json>(&arguments[1]);
    if (!subject || !subject->is_string() || !prefix || !prefix->is_string())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    auto stringSubject = subject->get_ptr<const String*>();
    auto stringPrefix = prefix->get_ptr<const String*>();
    return boost::starts_with(*stringSubject, *stringPrefix);
}

Json ExpressionEvaluator::sum(const FunctionArgumentList &arguments) const
{
    const Json* items = boost::get<Json>(&arguments[0]);
    if (items && items->is_array())
    {
        double sum = std::accumulate(items->cbegin(),
                                     items->cend(),
                                     0.0,
                                     [](double sum, const Json& item) -> double
        {
            if (item.is_number_integer())
            {
                return sum + item.get<Json::number_integer_t>();
            }
            else if (item.is_number_float())
            {
                return sum + item.get<Json::number_float_t>();
            }
            else
            {
                BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
            }
        });
        return sum;
    }
    else
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
}

Json ExpressionEvaluator::toArray(FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    Json result(Json::value_t::array);
    if (value->is_array())
    {
        result = std::move(*value);
    }
    else
    {
        result.push_back(std::move(*value));
    }
    return result;
}

Json ExpressionEvaluator::toString(FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    Json result;
    if (value->is_string())
    {
        result = std::move(*value);
    }
    else
    {
        result = value->dump();
    }
    return result;
}

Json ExpressionEvaluator::toNumber(FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    Json result;
    if (value->is_number())
    {
        result = std::move(*value);
    }
    else if (value->is_string())
    {
        try
        {
            result = std::stod(*value->get_ptr<const String*>());
        }
        catch (std::exception& exc)
        {
        }
    }
    return result;
}

Json ExpressionEvaluator::type(const FunctionArgumentList &arguments) const
{
    const Json* value = boost::get<Json>(&arguments[0]);
    if (!value)
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    auto type = value->type();
    String result;
    switch (type)
    {
    case Json::value_t::number_float:
    case Json::value_t::number_integer: result = "number"; break;
    case Json::value_t::string: result = "string"; break;
    case Json::value_t::boolean: result = "boolean"; break;
    case Json::value_t::array: result = "array"; break;
    case Json::value_t::object: result = "object"; break;
    default: result = "null";
    }
    return result;
}

Json ExpressionEvaluator::values(FunctionArgumentList &arguments) const
{
    const Json* object = boost::get<Json>(&arguments[0]);
    if (!object || !object->is_object())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    Json result(Json::value_t::array);
    for (auto it = object->begin(); it != object->end(); ++it)
    {
        result.push_back(std::move(it.value()));
    }
    return result;
}

Json ExpressionEvaluator::maxElement(const FunctionArgumentList &arguments,
                                     const JsonComparator &comparator) const
{
    const Json* array = boost::get<Json>(&arguments[0]);
    auto notNumberOrString = [](const auto& item)
    {
        return !(item.is_string() || item.is_number());
    };
    if (!array || !array->is_array() || alg::any_of(*array, notNumberOrString))
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }
    else if (!array->empty())
    {
        Json::value_t firstType = (*array)[0].type();
        bool result = alg::any_of(*array, [&](const Json& item){
           return item.type() != firstType;
        });
        if (result)
        {
            BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
        }
    }

    Json result;
    auto it = rng::max_element(*array, comparator);
    if (it != array->end())
    {
        result = *it;
    }
    return result;
}

Json ExpressionEvaluator::maxElementBy(FunctionArgumentList &arguments,
                                       const JsonComparator &comparator)
{
    Json* array = boost::get<Json>(&arguments[0]);
    ast::ExpressionNode* expression
            = boost::get<ast::ExpressionNode>(&arguments[1]);
    if (!array || !expression || !array->is_array())
    {
        BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
    }

    Json expressionResults(Json::value_t::array);
    rng::transform(*array, std::back_inserter(expressionResults),
                   [this, expression](const Json& item)
    {
        m_context = item;
        this->visit(expression);
        if (!(m_context.is_number() || m_context.is_string()))
        {
            BOOST_THROW_EXCEPTION(detail::InvalidFunctionArgumentType());
        }
        return m_context;
    });
    auto maxResultsIt = rng::max_element(expressionResults, comparator);
    auto maxIt = std::begin(*array)
            + std::distance(std::begin(expressionResults), maxResultsIt);
    return *maxIt;
}
}} // namespace jmespath::interpreter
