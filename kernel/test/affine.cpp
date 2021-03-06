/*
 *  Copyright (C) 2016 Matthew Keeter  <matt.j.keeter@gmail.com>
 *
 *  This file is part of the Ao library.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  Ao is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Ao.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <catch/catch.hpp>

#include "ao/kernel/tree/store.hpp"
#include "ao/kernel/tree/token.hpp"
#include "glm.hpp"

glm::vec4 affineVec(Token* t)
{
    return glm::vec4(t->a->a->b->value, t->a->b->b->value,
                     t->b->a->b->value, t->b->b->value);
}

TEST_CASE("Affine math")
{
    Store s;

    SECTION("Affine plus constant")
    {
        Token* t = s.operation(OP_ADD, s.affine(1, 0, 0, 0), s.constant(1));

        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(1, 0, 0, 1));
    }

    SECTION("Constant plus affine")
    {
        Token* t = s.operation(OP_ADD, s.constant(2), s.affine(1, 0, 0, 0));

        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(1, 0, 0, 2));
    }

    SECTION("Affine plus affine")
    {
        Token* t = s.operation(OP_ADD, s.affine(1, 2, 3, 4),
                                       s.affine(2, 4, 6, 8));
        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(3, 6, 9, 12));
    }

    SECTION("Affine minus constant")
    {
        Token* t = s.operation(OP_SUB, s.affine(1, 0, 0, 0), s.constant(1));

        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(1, 0, 0, -1));
    }

    SECTION("Constant minus affine")
    {
        Token* t = s.operation(OP_SUB, s.constant(2), s.affine(1, 0, 0, 0));

        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(-1, 0, 0, 2));
    }

    SECTION("Affine minus affine")
    {
        Token* t = s.operation(OP_SUB, s.affine(1, 2, 3, 4),
                                       s.affine(2, 4, 6, 8));
        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(-1, -2, -3, -4));
    }

    SECTION("Affine times constant")
    {
        Token* t = s.operation(OP_MUL, s.affine(1, 2, 3, 4), s.constant(2));
        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(2, 4, 6, 8));
    }

    SECTION("Constant times affine")
    {
        Token* t = s.operation(OP_MUL, s.constant(2), s.affine(1, 2, 3, 4));
        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(2, 4, 6, 8));
    }

    SECTION("Affine divided by constant")
    {
        Token* t = s.operation(OP_DIV, s.affine(1, 2, 3, 4), s.constant(2));
        REQUIRE(t->op == AFFINE);
        REQUIRE(affineVec(t) == glm::vec4(0.5, 1, 1.5, 2));
    }
}

TEST_CASE("collapseAffine")
{
    Store s;

    SECTION("Base case")
    {
        auto t = s.affine(1, 0, 0, 0);
        auto t_ = s.collapseAffine(t);
        REQUIRE(t_->op == OP_X);
        REQUIRE(t_->weight == 0);
    }

    SECTION("Sum")
    {
        auto t = s.affine(1, 1, 0, 0);
        auto t_ = s.collapseAffine(t);
        REQUIRE(t_->op == OP_ADD);
        REQUIRE(t_->a->op == OP_X);
        REQUIRE(t_->b->op == OP_Y);
        REQUIRE(t_->weight == 1);
    }

    SECTION("Multiplication")
    {
        auto t = s.affine(2, 3, 0, 0);
        auto t_ = s.collapseAffine(t);
        REQUIRE(t_->op == OP_ADD);
        REQUIRE(t_->a->op == OP_MUL);
        REQUIRE(t_->b->op == OP_MUL);

        REQUIRE(t_->a->a->op == OP_X);
        REQUIRE(t_->a->b->op == OP_CONST);
        REQUIRE(t_->a->b->value == 2);
        REQUIRE(t_->b->a->op == OP_Y);
        REQUIRE(t_->b->b->op == OP_CONST);
        REQUIRE(t_->b->b->value == 3);
    }

    SECTION("Constant")
    {
        auto t = s.affine(1, 0, 0, 3);
        auto t_ = s.collapseAffine(t);
        REQUIRE(t_->op == OP_ADD);
        REQUIRE(t_->a->op == OP_X);
        REQUIRE(t_->b->op == OP_CONST);
        REQUIRE(t_->b->value == 3);
    }

    SECTION("Not affine at all")
    {
        auto t = s.X();
        auto t_ = s.collapseAffine(t);
        REQUIRE(t == t_);
    }
}
