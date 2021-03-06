/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2020 The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ValueTranslator.cpp
 *
 ***********************************************************************/

#include "ast2ram/ValueTranslator.h"
#include "ast/IntrinsicFunctor.h"
#include "ast/NumericConstant.h"
#include "ast/RecordInit.h"
#include "ast/StringConstant.h"
#include "ast/UserDefinedFunctor.h"
#include "ast/analysis/Functor.h"
#include "ast2ram/utility/TranslatorContext.h"
#include "ast2ram/utility/Utils.h"
#include "ast2ram/utility/ValueIndex.h"
#include "ram/AutoIncrement.h"
#include "ram/FloatConstant.h"
#include "ram/IntrinsicOperator.h"
#include "ram/PackRecord.h"
#include "ram/SignedConstant.h"
#include "ram/SubroutineArgument.h"
#include "ram/TupleElement.h"
#include "ram/UndefValue.h"
#include "ram/UnsignedConstant.h"
#include "ram/UserDefinedOperator.h"
#include "ram/utility/Utils.h"
#include "souffle/SymbolTable.h"
#include "souffle/utility/StringUtil.h"

namespace souffle::ast2ram {

Own<ram::Expression> ValueTranslator::translate(const TranslatorContext& context, SymbolTable& symbolTable,
        const ValueIndex& index, const ast::Argument* arg) {
    assert(arg != nullptr && "arg should be defined");
    return ValueTranslator(context, symbolTable, index)(*arg);
}

Own<ram::Expression> ValueTranslator::translateValue(const ast::Argument* arg) const {
    return ValueTranslator::translate(context, symbolTable, index, arg);
}

Own<ram::Expression> ValueTranslator::visitVariable(const ast::Variable& var) {
    assert(index.isDefined(var) && "variable not grounded");
    return makeRamTupleElement(index.getDefinitionPoint(var));
}

Own<ram::Expression> ValueTranslator::visitUnnamedVariable(const ast::UnnamedVariable&) {
    return mk<ram::UndefValue>();
}

Own<ram::Expression> ValueTranslator::visitNumericConstant(const ast::NumericConstant& c) {
    assert(c.getFinalType().has_value() && "constant should have valid type");
    switch (c.getFinalType().value()) {
        case ast::NumericConstant::Type::Int:
            return mk<ram::SignedConstant>(RamSignedFromString(c.getConstant(), nullptr, 0));
        case ast::NumericConstant::Type::Uint:
            return mk<ram::UnsignedConstant>(RamUnsignedFromString(c.getConstant(), nullptr, 0));
        case ast::NumericConstant::Type::Float:
            return mk<ram::FloatConstant>(RamFloatFromString(c.getConstant()));
    }

    fatal("unexpected numeric constant type");
}

Own<ram::Expression> ValueTranslator::visitStringConstant(const ast::StringConstant& c) {
    return mk<ram::SignedConstant>(symbolTable.lookup(c.getConstant()));
}

Own<ram::Expression> ValueTranslator::visitNilConstant(const ast::NilConstant&) {
    return mk<ram::SignedConstant>(0);
}

Own<ram::Expression> ValueTranslator::visitTypeCast(const ast::TypeCast& typeCast) {
    return translateValue(typeCast.getValue());
}

Own<ram::Expression> ValueTranslator::visitIntrinsicFunctor(const ast::IntrinsicFunctor& inf) {
    VecOwn<ram::Expression> values;
    for (const auto& cur : inf.getArguments()) {
        values.push_back(translateValue(cur));
    }

    if (ast::analysis::FunctorAnalysis::isMultiResult(inf)) {
        return makeRamTupleElement(index.getGeneratorLoc(inf));
    } else {
        return mk<ram::IntrinsicOperator>(inf.getFinalOpType().value(), std::move(values));
    }
}

Own<ram::Expression> ValueTranslator::visitUserDefinedFunctor(const ast::UserDefinedFunctor& udf) {
    VecOwn<ram::Expression> values;
    for (const auto& cur : udf.getArguments()) {
        values.push_back(translateValue(cur));
    }
    auto returnType = context.getFunctorReturnType(&udf);
    auto argTypes = context.getFunctorArgTypes(udf);
    return mk<ram::UserDefinedOperator>(
            udf.getName(), argTypes, returnType, context.isStatefulFunctor(&udf), std::move(values));
}

Own<ram::Expression> ValueTranslator::visitCounter(const ast::Counter&) {
    return mk<ram::AutoIncrement>();
}

Own<ram::Expression> ValueTranslator::visitRecordInit(const ast::RecordInit& init) {
    VecOwn<ram::Expression> values;
    for (const auto& cur : init.getArguments()) {
        values.push_back(translateValue(cur));
    }
    return mk<ram::PackRecord>(std::move(values));
}

Own<ram::Expression> ValueTranslator::visitAggregator(const ast::Aggregator& agg) {
    // here we look up the location the aggregation result gets bound
    return makeRamTupleElement(index.getGeneratorLoc(agg));
}

Own<ram::Expression> ValueTranslator::visitSubroutineArgument(const ast::SubroutineArgument& subArg) {
    return mk<ram::SubroutineArgument>(subArg.getNumber());
}

}  // namespace souffle::ast2ram
