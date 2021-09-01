/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrUniformAggregator.h"

#include "src/sksl/dsl/priv/DSLFPs.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

using ProcessorUniforms = GrUniformAggregator::ProcessorUniforms;

ProcessorUniforms GrUniformAggregator::addUniforms(const GrProcessor& p,
                                                   const SkString& mangleSuffix) {
    Processor processor{
            &p,
            fUniforms.size(),
            fUniforms.size(),
    };
    for (size_t i = 0; i < p.uniforms().size(); ++i) {
        if (!p.uniforms()[i].isInitialized()) {
            continue;
        }
        // We give every uniform an initial name so it always can be validly declared. When code is
        // emitted the processor can give it a more meaningful name. The actual name doesn't matter,
        // other than for readability.
        SkString unusedName = SkStringPrintf("default_%zu%s", i, mangleSuffix.c_str());
        fUniforms.push_back(Record{std::move(unusedName), &p, i});
        ++processor.end;
    }
    fProcessors.push_back(processor);
    return ProcessorUniforms(p, mangleSuffix, this);
}

//////////////////////////////////////////////////////////////////////////////

ProcessorUniforms::ProcessorUniforms(const GrProcessor& p,
                                     const SkString& mangleSuffix,
                                     GrUniformAggregator* aggregator)
        : fAgg(aggregator), fMangleSuffix(mangleSuffix) {
    for (size_t i = 0; i < fAgg->fProcessors.size(); ++i) {
        if (fAgg->fProcessors[i].processor == &p) {
            fBegin = fAgg->fProcessors[i].begin;
            fEnd   = fAgg->fProcessors[i].end;
            return;
        }
    }
}

const char* ProcessorUniforms::getUniformName(size_t index, const char* newBaseName) const {
    for (size_t i = fBegin; i < fEnd; ++i) {
        if (fAgg->fUniforms[i].indexInProcessor == index) {
            GrUniformAggregator::Record& r = fAgg->fUniforms[i];
            if (newBaseName) {
                SkString mangledName = SkStringPrintf("%s%s", newBaseName, fMangleSuffix.c_str());
                r.name = mangledName;
            }
            return r.name.c_str();
        } else if (fAgg->fUniforms[i].indexInProcessor > index) {
            break;
        }
    }
    return nullptr;
}

static constexpr SkSL::dsl::TypeConstant sltype_to_dsl_typeconstant(GrSLType type) {
    switch (type) {
        case kFloat_GrSLType:    return SkSL::dsl::kFloat_Type;
        case kFloat2_GrSLType:   return SkSL::dsl::kFloat2_Type;
        case kFloat3_GrSLType:   return SkSL::dsl::kFloat3_Type;
        case kFloat4_GrSLType:   return SkSL::dsl::kFloat4_Type;
        case kFloat2x2_GrSLType: return SkSL::dsl::kFloat2x2_Type;
        case kFloat3x3_GrSLType: return SkSL::dsl::kFloat3x3_Type;
        case kFloat4x4_GrSLType: return SkSL::dsl::kFloat4x4_Type;

        case kHalf_GrSLType:     return SkSL::dsl::kHalf_Type;
        case kHalf2_GrSLType:    return SkSL::dsl::kHalf2_Type;
        case kHalf3_GrSLType:    return SkSL::dsl::kHalf3_Type;
        case kHalf4_GrSLType:    return SkSL::dsl::kHalf4_Type;
        case kHalf2x2_GrSLType:  return SkSL::dsl::kHalf2x2_Type;
        case kHalf3x3_GrSLType:  return SkSL::dsl::kHalf3x3_Type;
        case kHalf4x4_GrSLType:  return SkSL::dsl::kHalf4x4_Type;

        case kBool_GrSLType:     return SkSL::dsl::kBool_Type;
        case kBool2_GrSLType:    return SkSL::dsl::kBool2_Type;
        case kBool3_GrSLType:    return SkSL::dsl::kBool3_Type;
        case kBool4_GrSLType:    return SkSL::dsl::kBool4_Type;

        case kShort_GrSLType:    return SkSL::dsl::kShort_Type;
        case kShort2_GrSLType:   return SkSL::dsl::kShort2_Type;
        case kShort3_GrSLType:   return SkSL::dsl::kShort3_Type;
        case kShort4_GrSLType:   return SkSL::dsl::kShort4_Type;

        case kUShort_GrSLType:   return SkSL::dsl::kUShort_Type;
        case kUShort2_GrSLType:  return SkSL::dsl::kUShort2_Type;
        case kUShort3_GrSLType:  return SkSL::dsl::kUShort3_Type;
        case kUShort4_GrSLType:  return SkSL::dsl::kUShort4_Type;

        case kInt_GrSLType:      return SkSL::dsl::kInt_Type;
        case kInt2_GrSLType:     return SkSL::dsl::kInt2_Type;
        case kInt3_GrSLType:     return SkSL::dsl::kInt3_Type;
        case kInt4_GrSLType:     return SkSL::dsl::kInt4_Type;

        case kUint_GrSLType:     return SkSL::dsl::kUInt_Type;
        case kUint2_GrSLType:    return SkSL::dsl::kUInt2_Type;
        case kUint3_GrSLType:    return SkSL::dsl::kUInt3_Type;
        case kUint4_GrSLType:    return SkSL::dsl::kUInt4_Type;

        case kVoid_GrSLType:     return SkSL::dsl::kVoid_Type;

        case kByte_GrSLType:
        case kByte2_GrSLType:
        case kByte3_GrSLType:
        case kByte4_GrSLType:
        case kUByte_GrSLType:
        case kUByte2_GrSLType:
        case kUByte3_GrSLType:
        case kUByte4_GrSLType:
        case kTexture2DSampler_GrSLType:
        case kTextureExternalSampler_GrSLType:
        case kTexture2DRectSampler_GrSLType:
        case kTexture2D_GrSLType:
        case kSampler_GrSLType:
        case kInput_GrSLType:
            SkUNREACHABLE;
    }
    SkUNREACHABLE;
}
SkSL::dsl::GlobalVar ProcessorUniforms::getUniformAsDSLVar(size_t index,
                                                           const GrProcessor& owner,
                                                           const char* newBaseName) const {
    for (size_t i = fBegin; i < fEnd; ++i) {
        if (fAgg->fUniforms[i].indexInProcessor == index) {
            GrUniformAggregator::Record& r = fAgg->fUniforms[i];
            if (newBaseName) {
                SkString mangledName = SkStringPrintf("%s%s", newBaseName, fMangleSuffix.c_str());
                r.name = mangledName;
            }
            const GrProcessor::Uniform& u = owner.uniforms()[index];
            SkSL::dsl::DSLType type = sltype_to_dsl_typeconstant(u.type());
            if (u.isArray()) {
                type = SkSL::dsl::Array(type, u.count());
            }
            SkSL::dsl::GlobalVar var(SkSL::dsl::kNo_Modifier,
                                     type,
                                     r.name.c_str());
            // All aggregated uniforms get declared elsewhere.
            SkSL::dsl::DSLWriter::MarkDeclared(var);
            r.name = SkString(var.name().data(), var.name().size());
            return var;

        } else if (fAgg->fUniforms[i].indexInProcessor > index) {
            break;
        }
    }
    return {};
}
