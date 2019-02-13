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
#include "fakeit.hpp"
#include "src/ast/allnodes.h"
#include "src/interpreter/abstractvisitor.h"

TEST_CASE("HashWildcardNode")
{
    using namespace jmespath::ast;
    using namespace jmespath::interpreter;
    using namespace fakeit;

    SECTION("can be constructed")
    {
        SECTION("without parameters")
        {
            REQUIRE_NOTHROW(HashWildcardNode{});
        }

        SECTION("with left and right expression")
        {
            ExpressionNode leftExpression{
                IdentifierNode{"id1"}};
            ExpressionNode rightExpression{
                IdentifierNode{"id2"}};

            HashWildcardNode node{leftExpression, rightExpression};

            REQUIRE(node.leftExpression == leftExpression);
            REQUIRE(node.rightExpression == rightExpression);
        }
    }

    SECTION("can be compared for equality")
    {
        ExpressionNode leftExpression{
            IdentifierNode{"id1"}};
        ExpressionNode rightExpression{
            IdentifierNode{"id2"}};
        HashWildcardNode node1{leftExpression, rightExpression};
        HashWildcardNode node2;
        node2 = node1;

        REQUIRE(node1 == node2);
        REQUIRE(node1 == node1);
    }

    SECTION("return true for isProjection")
    {
        REQUIRE(HashWildcardNode{}.isProjection());
    }

    SECTION("return false for stopsProjection")
    {
        REQUIRE_FALSE(HashWildcardNode{}.stopsProjection());
    }

    SECTION("accepts visitor")
    {
        HashWildcardNode node{};
        Mock<AbstractVisitor> visitor;
        When(OverloadedMethod(visitor, visit, void(const HashWildcardNode*)))
                .AlwaysReturn();

        node.accept(&visitor.get());

        Verify(OverloadedMethod(visitor, visit, void(const HashWildcardNode*)))
                .Once();
    }
}
