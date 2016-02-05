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
#include "jmespath/interpreter/expressionevaluator.h"
#include "jmespath/ast/allnodes.h"

TEST_CASE("ExpressionEvaluator")
{
    using jmespath::interpreter::ExpressionEvaluator;
    using jmespath::detail::Json;
    using jmespath::detail::String;
    namespace ast = jmespath::ast;
    using namespace fakeit;

    ExpressionEvaluator evaluator;

    SECTION("can be constructed with context value")
    {
        Json context{"ident", "value"};

        ExpressionEvaluator ev{context};

        REQUIRE(ev.currentContext() == context);
    }

    SECTION("accepts abstract node")
    {
        Mock<ast::AbstractNode> node;
        When(Method(node, accept).Using(&evaluator)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept)).Once();
    }

    SECTION("accepts expression node")
    {
        Mock<ast::ExpressionNode> node;
        When(Method(node, accept).Using(&evaluator)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept)).Once();
    }

    SECTION("evaluates identifier node")
    {
        ast::IdentifierNode node{"identifier"};
        int value = 15;
        Json expectedValue = value;
        REQUIRE(expectedValue.is_number());
        evaluator.setContext({{"identifier", value}});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedValue);
    }

    SECTION("evaluates non existing identifier to null")
    {
        ast::IdentifierNode node{"non-existing"};

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates identifier on non object to null")
    {
        ast::IdentifierNode node{"identifier"};
        evaluator.setContext(15);

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates raw string")
    {
        String rawString{"[baz]"};
        ast::RawStringNode node{rawString};
        Json expectedValue = rawString;
        REQUIRE(expectedValue.is_string());

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == expectedValue);
    }

    SECTION("evaluates literal")
    {
        String stringLiteralValue{"foo"};
        ast::LiteralNode stringNode{"\"" + stringLiteralValue + "\""};
        ast::LiteralNode arrayNode{"[1, 2, 3]"};
        Json expectedStringValue = stringLiteralValue;
        Json expectedArrayValue{1, 2, 3};
        REQUIRE(expectedStringValue.is_string());
        REQUIRE(expectedArrayValue.is_array());

        evaluator.visit(&stringNode);
        auto stringResult = evaluator.currentContext();
        evaluator.visit(&arrayNode);
        auto arrayResult = evaluator.currentContext();

        REQUIRE(stringResult == expectedStringValue);
        REQUIRE(arrayResult == expectedArrayValue);
    }

    SECTION("evaluates subexpression")
    {
        Mock<ast::SubexpressionNode> node;
        When(Method(node, accept)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept).Using(&evaluator)).Once();
    }    

    SECTION("evaluates bracket specifier")
    {
        Mock<ast::BracketSpecifierNode> node;
        When(Method(node, accept)).AlwaysReturn();

        evaluator.visit(&node.get());

        Verify(Method(node, accept).Using(&evaluator)).Once();
    }

    SECTION("evaluates index expression")
    {
        ast::IndexExpressionNode node;
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::BracketSpecifierNode*)))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                  void(ast::BracketSpecifierNode*))
                    .Using(&node.bracketSpecifier))
                .Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates projected index expression")
    {
        ast::IndexExpressionNode node{
            ast::ExpressionNode{},
            ast::BracketSpecifierNode{
                ast::FlattenOperatorNode{}},
            ast::ExpressionNode{}};
        Mock<ExpressionEvaluator> evaluatorMock(evaluator);
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::ExpressionNode*)))
                .AlwaysReturn();
        When(OverloadedMethod(evaluatorMock, visit,
                              void(ast::BracketSpecifierNode*)))
                .AlwaysReturn();
        When(Method(evaluatorMock, evaluateProjection))
                .AlwaysReturn();

        evaluatorMock.get().visit(&node);

        Verify(OverloadedMethod(evaluatorMock, visit,
                                void(ast::ExpressionNode*))
                    .Using(&node.leftExpression)
               + OverloadedMethod(evaluatorMock, visit,
                                  void(ast::BracketSpecifierNode*))
                    .Using(&node.bracketSpecifier)
               + Method(evaluatorMock, evaluateProjection)
                    .Using(&node.rightExpression))
                .Once();
        VerifyNoOtherInvocations(evaluatorMock);
    }

    SECTION("evaluates array item expression")
    {
        ast::ArrayItemNode node{2};
        evaluator.setContext({"zero", "one", "two", "three", "four"});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "two");
    }

    SECTION("evaluates array item expression with negative index")
    {
        ast::ArrayItemNode node{-4};
        evaluator.setContext({"zero", "one", "two", "three", "four"});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == "one");
    }

    SECTION("evaluates array item expression on non arrays to null")
    {
        ast::ArrayItemNode node{2};
        evaluator.setContext(3);

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates array item expression with out of bounds index to null")
    {
        ast::ArrayItemNode node{15};
        evaluator.setContext({"zero", "one", "two", "three", "four"});

        evaluator.visit(&node);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates projection")
    {
        Json context = {{{"id", 1}}, {{"id", 2}}, {{"id2", 3}}, {{"id", 4}}};
        REQUIRE(context.is_array());
        evaluator.setContext(context);
        ast::ExpressionNode expression{
            ast::IdentifierNode{"id"}};
        evaluator.setContext(context);
        Json expectedResult = {1, 2, 4};
        REQUIRE(expectedResult.is_array());

        evaluator.evaluateProjection(&expression);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates projection on non arrays to null")
    {
        Json context = "string";
        REQUIRE(context.is_string());
        ast::ExpressionNode expression{
            ast::IdentifierNode{"id"}};
        evaluator.setContext(context);

        evaluator.evaluateProjection(&expression);

        REQUIRE(evaluator.currentContext() == Json{});
    }

    SECTION("evaluates flatten operator")
    {
        Json context = "[1, 2, [3], [4, [5, 6, 7], 8], [9, 10] ]"_json;
        evaluator.setContext(context);
        Json expectedResult = "[1, 2, 3, 4, [5, 6, 7], 8, 9, 10]"_json;
        ast::FlattenOperatorNode flattenNode;

        evaluator.visit(&flattenNode);

        REQUIRE(evaluator.currentContext() == expectedResult);
    }

    SECTION("evaluates flatten operator on non array to null")
    {
        Json context = {{"id", "value"}};
        REQUIRE(context.is_object());
        evaluator.setContext(context);
        ast::FlattenOperatorNode flattenNode;

        evaluator.visit(&flattenNode);

        REQUIRE(evaluator.currentContext() == Json{});
    }
}
