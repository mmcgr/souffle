/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2019, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ram_condition_equal_clone_test.cpp
 *
 * Tests equal and clone function of RamCondition classes.
 *
 ***********************************************************************/

#include "tests/test.h"

#include "RelationTag.h"
#include "ram/Conjunction.h"
#include "ram/Constraint.h"
#include "ram/EmptinessCheck.h"
#include "ram/ExistenceCheck.h"
#include "ram/Expression.h"
#include "ram/False.h"
#include "ram/Negation.h"
#include "ram/ProvenanceExistenceCheck.h"
#include "ram/Relation.h"
#include "ram/SignedConstant.h"
#include "ram/True.h"
#include "ram/TupleElement.h"
#include "souffle/BinaryConstraintOps.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

namespace test {

TEST(RamTrue, CloneAndEquals) {
    RamTrue a;
    RamTrue b;
    EXPECT_EQ(a, b);
    EXPECT_NE(&a, &b);

    RamTrue* c = a.clone();
    EXPECT_EQ(a, *c);
    EXPECT_NE(&a, c);
    delete c;
}

TEST(RamFalse, CloneAndEquals) {
    RamFalse a;
    RamFalse b;
    EXPECT_EQ(a, b);
    EXPECT_NE(&a, &b);

    RamFalse* c = a.clone();
    EXPECT_EQ(a, *c);
    EXPECT_NE(&a, c);
    delete c;
}

TEST(RamConjunction, CloneAndEquals) {
    // true /\ false
    auto a = mk<RamConjunction>(mk<RamTrue>(), mk<RamFalse>());
    auto b = mk<RamConjunction>(mk<RamTrue>(), mk<RamFalse>());
    EXPECT_EQ(*a, *b);
    EXPECT_NE(a, b);

    Own<RamConjunction> c(a->clone());
    EXPECT_EQ(*a, *c);
    EXPECT_NE(a, c);

    // true /\ (false /\ true)
    auto d = mk<RamConjunction>(mk<RamTrue>(), mk<RamConjunction>(mk<RamFalse>(), mk<RamTrue>()));
    auto e = mk<RamConjunction>(mk<RamTrue>(), mk<RamConjunction>(mk<RamFalse>(), mk<RamTrue>()));
    EXPECT_EQ(*d, *e);
    EXPECT_NE(d, e);

    Own<RamConjunction> f(d->clone());
    EXPECT_EQ(*d, *f);
    EXPECT_NE(d, f);

    // (true /\ false) /\ (true /\ (false /\ true))
    auto a_conj_d = mk<RamConjunction>(std::move(a), std::move(d));
    auto b_conj_e = mk<RamConjunction>(std::move(b), std::move(e));
    EXPECT_EQ(*a_conj_d, *b_conj_e);
    EXPECT_NE(a_conj_d, b_conj_e);

    auto c_conj_f = mk<RamConjunction>(std::move(c), std::move(f));
    EXPECT_EQ(*c_conj_f, *a_conj_d);
    EXPECT_EQ(*c_conj_f, *b_conj_e);
    EXPECT_NE(c_conj_f, a_conj_d);
    EXPECT_NE(c_conj_f, b_conj_e);

    Own<RamConjunction> a_conj_d_copy(a_conj_d->clone());
    EXPECT_EQ(*a_conj_d, *a_conj_d_copy);
    EXPECT_NE(a_conj_d, a_conj_d_copy);
}

TEST(RamNegation, CloneAndEquals) {
    auto a = mk<RamTrue>();
    auto neg_a = mk<RamNegation>(std::move(a));
    auto b = mk<RamTrue>();
    auto neg_b = mk<RamNegation>(std::move(b));
    EXPECT_EQ(*neg_a, *neg_b);
    EXPECT_NE(neg_a, neg_b);

    auto c = mk<RamFalse>();
    auto neg_neg_c = mk<RamNegation>(mk<RamNegation>(std::move(c)));
    auto d = mk<RamFalse>();
    auto neg_neg_d = mk<RamNegation>(mk<RamNegation>(std::move(d)));
    EXPECT_EQ(*neg_neg_c, *neg_neg_d);
    EXPECT_NE(neg_neg_c, neg_neg_d);
}

TEST(RamConstraint, CloneAndEquals) {
    // constraint t0.1 = t1.0
    Own<RamExpression> a_lhs(new RamTupleElement(0, 1));
    Own<RamExpression> a_rhs(new RamTupleElement(1, 0));
    Own<RamConstraint> a(new RamConstraint(BinaryConstraintOp::EQ, std::move(a_lhs), std::move(a_rhs)));
    Own<RamExpression> b_lhs(new RamTupleElement(0, 1));
    Own<RamExpression> b_rhs(new RamTupleElement(1, 0));
    Own<RamConstraint> b(new RamConstraint(BinaryConstraintOp::EQ, std::move(b_lhs), std::move(b_rhs)));
    EXPECT_EQ(*a, *b);
    EXPECT_NE(a, b);

    Own<RamConstraint> c(a->clone());
    EXPECT_EQ(*a, *c);
    EXPECT_EQ(*b, *c);
    EXPECT_NE(a, c);
    EXPECT_NE(b, c);

    // constraint t2.0 >= 5
    Own<RamExpression> d_lhs(new RamTupleElement(2, 0));
    Own<RamExpression> d_rhs(new RamSignedConstant(5));
    Own<RamConstraint> d(new RamConstraint(BinaryConstraintOp::EQ, std::move(d_lhs), std::move(d_rhs)));
    Own<RamExpression> e_lhs(new RamTupleElement(2, 0));
    Own<RamExpression> e_rhs(new RamSignedConstant(5));
    Own<RamConstraint> e(new RamConstraint(BinaryConstraintOp::EQ, std::move(e_lhs), std::move(e_rhs)));
    EXPECT_EQ(*d, *e);
    EXPECT_NE(d, e);

    Own<RamConstraint> f(d->clone());
    EXPECT_EQ(*d, *f);
    EXPECT_EQ(*e, *f);
    EXPECT_NE(d, f);
    EXPECT_NE(e, f);
}

TEST(RamExistenceCheck, CloneAndEquals) {
    // N(1) in relation N(x:number)
    RamRelation N("N", 1, 1, {"x"}, {"i"}, RelationRepresentation::DEFAULT);
    VecOwn<RamExpression> tuple_a;
    tuple_a.emplace_back(new RamSignedConstant(1));
    RamExistenceCheck a(mk<RamRelationReference>(&N), std::move(tuple_a));
    VecOwn<RamExpression> tuple_b;
    tuple_b.emplace_back(new RamSignedConstant(1));
    RamExistenceCheck b(mk<RamRelationReference>(&N), std::move(tuple_b));
    EXPECT_EQ(a, b);
    EXPECT_NE(&a, &b);

    RamExistenceCheck* c = a.clone();
    EXPECT_EQ(a, *c);
    EXPECT_EQ(b, *c);
    EXPECT_NE(&a, c);
    EXPECT_NE(&b, c);

    delete c;

    // edge(1,2) in relation edge(x:number,y:number)
    RamRelation edge("edge", 2, 1, {"x", "y"}, {"i", "i"}, RelationRepresentation::BRIE);
    VecOwn<RamExpression> tuple_d;
    tuple_d.emplace_back(new RamSignedConstant(1));
    tuple_d.emplace_back(new RamSignedConstant(2));
    RamExistenceCheck d(mk<RamRelationReference>(&edge), std::move(tuple_d));
    VecOwn<RamExpression> tuple_e;
    tuple_e.emplace_back(new RamSignedConstant(1));
    tuple_e.emplace_back(new RamSignedConstant(2));
    RamExistenceCheck e(mk<RamRelationReference>(&edge), std::move(tuple_e));
    EXPECT_EQ(d, e);
    EXPECT_NE(&d, &e);

    RamExistenceCheck* f = d.clone();
    EXPECT_EQ(d, *f);
    EXPECT_EQ(e, *f);
    EXPECT_NE(&d, f);
    EXPECT_NE(&e, f);

    delete f;
}

TEST(RamProvenanceExistCheck, CloneAndEquals) {
    RamRelation N("N", 1, 1, {"x"}, {"i"}, RelationRepresentation::DEFAULT);
    VecOwn<RamExpression> tuple_a;
    tuple_a.emplace_back(new RamSignedConstant(1));
    RamExistenceCheck a(mk<RamRelationReference>(&N), std::move(tuple_a));
    VecOwn<RamExpression> tuple_b;
    tuple_b.emplace_back(new RamSignedConstant(1));
    RamExistenceCheck b(mk<RamRelationReference>(&N), std::move(tuple_b));
    EXPECT_EQ(a, b);
    EXPECT_NE(&a, &b);

    RamExistenceCheck* c = a.clone();
    EXPECT_EQ(a, *c);
    EXPECT_EQ(b, *c);
    EXPECT_NE(&a, c);
    EXPECT_NE(&b, c);

    delete c;

    // address(state:symbol, postCode:number, street:symbol)
    RamRelation address("address", 3, 1, {"state", "postCode", "street"}, {"s", "i", "s"},
            RelationRepresentation::DEFAULT);
    VecOwn<RamExpression> tuple_d;
    tuple_d.emplace_back(new RamSignedConstant(0));
    tuple_d.emplace_back(new RamSignedConstant(2000));
    tuple_d.emplace_back(new RamSignedConstant(0));
    RamProvenanceExistenceCheck d(mk<RamRelationReference>(&address), std::move(tuple_d));
    VecOwn<RamExpression> tuple_e;
    tuple_e.emplace_back(new RamSignedConstant(0));
    tuple_e.emplace_back(new RamSignedConstant(2000));
    tuple_e.emplace_back(new RamSignedConstant(0));
    RamProvenanceExistenceCheck e(mk<RamRelationReference>(&address), std::move(tuple_e));
    EXPECT_EQ(d, e);
    EXPECT_NE(&d, &e);

    RamProvenanceExistenceCheck* f = d.clone();
    EXPECT_EQ(d, *f);
    EXPECT_EQ(e, *f);
    EXPECT_NE(&d, f);
    EXPECT_NE(&e, f);

    delete f;
}

TEST(RamEmptinessCheck, CloneAndEquals) {
    // Check A(x:number)
    RamRelation A("A", 1, 1, {"x"}, {"i"}, RelationRepresentation::DEFAULT);
    RamEmptinessCheck a(mk<RamRelationReference>(&A));
    RamEmptinessCheck b(mk<RamRelationReference>(&A));
    EXPECT_EQ(a, b);
    EXPECT_NE(&a, &b);
    RamEmptinessCheck* c = a.clone();
    EXPECT_EQ(a, *c);
    EXPECT_EQ(b, *c);
    EXPECT_NE(&a, c);
    EXPECT_NE(&b, c);
    delete c;
}

}  // end namespace test
}  // end namespace souffle