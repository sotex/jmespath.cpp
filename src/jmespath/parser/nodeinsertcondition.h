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
#ifndef NODEINSERTCONDITION_H
#define NODEINSERTCONDITION_H
#include "jmespath/ast/allnodes.h"
#include "jmespath/parser/noderank.h"
#include <boost/variant/polymorphic_get.hpp>

namespace jmespath { namespace parser {

/**
 * @brief The NodeInsertCondition class is a functor that will either yield
 * a true or false result based on whether the given @a node should be inserted
 * at the location of the @a targetNode.
 */
class NodeInsertCondition
{
public:
    /**
     * The result type of the functor.
     */
    using result_type = bool;

    /**
     * @brief Returns true or false based on whether the given @a node should be
     * inserted at the location of the @a targetNode.
     * @param targetNode The node located where the given @a node might be
     * insertable.
     * @param node The node to test for insertability.
     * @{
     */
    template <typename T, typename
        std::enable_if<
            std::is_base_of<ast::BinaryExpressionNode,
                            T>::value, int>::type = 0>
    bool operator()(ast::ExpressionNode* targetNode,
                    T* node) const
    {
        int targetNodeRank = nodeRank(*targetNode);
        int currentNodeRank = nodeRank(*node);
        ast::BinaryExpressionNode* targetBinaryNode
                = boost::polymorphic_get<ast::BinaryExpressionNode>(
                    &targetNode->value);
        return (targetNodeRank < currentNodeRank)
                || ((targetNodeRank == currentNodeRank)
                    && targetBinaryNode
                    && targetBinaryNode->isProjection()
                    && !targetBinaryNode->stopsProjection());
    }

    template <typename T, typename
        std::enable_if<
            !std::is_base_of<ast::BinaryExpressionNode,
                             T>::value, int>::type = 0>
    bool operator()(ast::ExpressionNode* targetNode,
                    T* node) const
    {
        int targetNodeRank = nodeRank(*targetNode);
        int currentNodeRank = nodeRank(*node);
        return (targetNodeRank < currentNodeRank);
    }
    /** @}*/
};
}} // namespace jmespath::parser
#endif // NODEINSERTCONDITION_H
