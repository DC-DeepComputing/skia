/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLChildCall.h"

namespace SkSL {

bool ChildCall::hasProperty(Property property) const {
    for (const auto& arg : this->arguments()) {
        if (arg->hasProperty(property)) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<Expression> ChildCall::clone() const {
    ExpressionArray cloned;
    cloned.reserve_back(this->arguments().size());
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        cloned.push_back(arg->clone());
    }
    return std::make_unique<ChildCall>(fOffset, &this->type(), &this->child(), std::move(cloned));
}

String ChildCall::description() const {
    String result;
    switch (this->child().type().typeKind()) {
        case Type::TypeKind::kBlender:     result += "blend(";  break;
        case Type::TypeKind::kColorFilter: result += "filter("; break;
        case Type::TypeKind::kShader:      result += "shade(";  break;
        default: SkUNREACHABLE;
    }
    result += this->child().name();
    for (const std::unique_ptr<Expression>& arg : this->arguments()) {
        result += ", ";
        result += arg->description();
    }
    result += ")";
    return result;
}

#ifdef SK_DEBUG
namespace {
struct ChildCallSignature {
    const Type*               fReturnType = nullptr;
    SkSTArray<2, const Type*> fParamTypes;
};

static ChildCallSignature child_call_signature(const Context& context, const Variable& child) {
    const Type* half4  = context.fTypes.fHalf4.get();
    const Type* float2 = context.fTypes.fFloat2.get();

    switch (child.type().typeKind()) {
        case Type::TypeKind::kBlender:     return { half4, { half4, half4 } };
        case Type::TypeKind::kColorFilter: return { half4, { half4 } };
        case Type::TypeKind::kShader:      return { half4, { float2 } };
        default:
            SkUNREACHABLE;
    }
}
}  // namespace
#endif

std::unique_ptr<Expression> ChildCall::Make(const Context& context,
                                            int offset,
                                            const Type* returnType,
                                            const Variable& child,
                                            ExpressionArray arguments) {
#ifdef SK_DEBUG
    ChildCallSignature signature = child_call_signature(context, child);
    SkASSERT(signature.fParamTypes.size() == arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
        SkASSERT(arguments[i]->type() == *signature.fParamTypes[i]);
    }
#endif

    return std::make_unique<ChildCall>(offset, returnType, &child, std::move(arguments));
}

}  // namespace SkSL
