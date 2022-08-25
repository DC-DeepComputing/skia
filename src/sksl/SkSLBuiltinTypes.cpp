// namespace SkSL
/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinTypes.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/spirv.h"

namespace SkSL {

/**
 * Initializes the core SkSL types.
 */

using BuiltinTypePtr = const std::unique_ptr<Type> BuiltinTypes::*;

struct AliasInitializationData {
    BuiltinTypePtr dest;
    const char* name;
    BuiltinTypePtr targetType;
};

struct Generic4InitializationData {
    BuiltinTypePtr dest;
    const char* name;
    BuiltinTypePtr type0, type1, type2, type3;
};

#define TYPE(x) &BuiltinTypes::f##x

static constexpr AliasInitializationData kAliasData[] = {
        {TYPE(Vec2), "vec2", TYPE(Float2)},
        {TYPE(Vec3), "vec3", TYPE(Float3)},
        {TYPE(Vec4), "vec4", TYPE(Float4)},
        {TYPE(IVec2), "ivec2", TYPE(Int2)},
        {TYPE(IVec3), "ivec3", TYPE(Int3)},
        {TYPE(IVec4), "ivec4", TYPE(Int4)},
        {TYPE(BVec2), "bvec2", TYPE(Bool2)},
        {TYPE(BVec3), "bvec3", TYPE(Bool3)},
        {TYPE(BVec4), "bvec4", TYPE(Bool4)},
        {TYPE(Mat2), "mat2", TYPE(Float2x2)},
        {TYPE(Mat3), "mat3", TYPE(Float3x3)},
        {TYPE(Mat4), "mat4", TYPE(Float4x4)},
        {TYPE(Mat2x2), "mat2x2", TYPE(Float2x2)},
        {TYPE(Mat2x3), "mat2x3", TYPE(Float2x3)},
        {TYPE(Mat2x4), "mat2x4", TYPE(Float2x4)},
        {TYPE(Mat3x2), "mat3x2", TYPE(Float3x2)},
        {TYPE(Mat3x3), "mat3x3", TYPE(Float3x3)},
        {TYPE(Mat3x4), "mat3x4", TYPE(Float3x4)},
        {TYPE(Mat4x2), "mat4x2", TYPE(Float4x2)},
        {TYPE(Mat4x3), "mat4x3", TYPE(Float4x3)},
        {TYPE(Mat4x4), "mat4x4", TYPE(Float4x4)}};

static constexpr Generic4InitializationData kGeneric4Data[] = {
        {TYPE(GenType), "$genType", TYPE(Float), TYPE(Float2), TYPE(Float3), TYPE(Float4)},
        {TYPE(GenHType), "$genHType", TYPE(Half), TYPE(Half2), TYPE(Half3), TYPE(Half4)},
        {TYPE(GenIType), "$genIType", TYPE(Int), TYPE(Int2), TYPE(Int3), TYPE(Int4)},
        {TYPE(GenUType), "$genUType", TYPE(UInt), TYPE(UInt2), TYPE(UInt3), TYPE(UInt4)},
        {TYPE(GenBType), "$genBType", TYPE(Bool), TYPE(Bool2), TYPE(Bool3), TYPE(Bool4)},
        {TYPE(Vec), "$vec", TYPE(Invalid), TYPE(Float2), TYPE(Float3), TYPE(Float4)},
        {TYPE(HVec), "$hvec", TYPE(Invalid), TYPE(Half2), TYPE(Half3), TYPE(Half4)},
        {TYPE(IVec), "$ivec", TYPE(Invalid), TYPE(Int2), TYPE(Int3), TYPE(Int4)},
        {TYPE(UVec), "$uvec", TYPE(Invalid), TYPE(UInt2), TYPE(UInt3), TYPE(UInt4)},
        {TYPE(SVec), "$svec", TYPE(Invalid), TYPE(Short2), TYPE(Short3), TYPE(Short4)},
        {TYPE(USVec), "$usvec", TYPE(Invalid), TYPE(UShort2), TYPE(UShort3), TYPE(UShort4)},
        {TYPE(BVec), "$bvec", TYPE(Invalid), TYPE(Bool2), TYPE(Bool3), TYPE(Bool4)},
        {TYPE(SquareMat), "$squareMat", TYPE(Invalid), TYPE(Float2x2),
                                        TYPE(Float3x3), TYPE(Float4x4)},
        {TYPE(SquareHMat), "$squareHMat", TYPE(Invalid), TYPE(Half2x2),
                                          TYPE(Half3x3), TYPE(Half4x4)},
};

BuiltinTypes::BuiltinTypes()
        : fFloat(Type::MakeScalarType(
                  "float", "f", Type::NumberKind::kFloat, /*priority=*/10, /*bitWidth=*/32))
        , fFloat2(Type::MakeVectorType("float2", "f2", *fFloat, /*columns=*/2))
        , fFloat3(Type::MakeVectorType("float3", "f3", *fFloat, /*columns=*/3))
        , fFloat4(Type::MakeVectorType("float4", "f4", *fFloat, /*columns=*/4))
        , fHalf(Type::MakeScalarType(
                  "half", "h", Type::NumberKind::kFloat, /*priority=*/9, /*bitWidth=*/16))
        , fHalf2(Type::MakeVectorType("half2", "h2", *fHalf, /*columns=*/2))
        , fHalf3(Type::MakeVectorType("half3", "h3", *fHalf, /*columns=*/3))
        , fHalf4(Type::MakeVectorType("half4", "h4", *fHalf, /*columns=*/4))
        , fInt(Type::MakeScalarType(
                  "int", "i", Type::NumberKind::kSigned, /*priority=*/7, /*bitWidth=*/32))
        , fInt2(Type::MakeVectorType("int2", "i2", *fInt, /*columns=*/2))
        , fInt3(Type::MakeVectorType("int3", "i3", *fInt, /*columns=*/3))
        , fInt4(Type::MakeVectorType("int4", "i4", *fInt, /*columns=*/4))
        , fUInt(Type::MakeScalarType(
                  "uint", "I", Type::NumberKind::kUnsigned, /*priority=*/6, /*bitWidth=*/32))
        , fUInt2(Type::MakeVectorType("uint2", "I2", *fUInt, /*columns=*/2))
        , fUInt3(Type::MakeVectorType("uint3", "I3", *fUInt, /*columns=*/3))
        , fUInt4(Type::MakeVectorType("uint4", "I4", *fUInt, /*columns=*/4))
        , fShort(Type::MakeScalarType(
                  "short", "s", Type::NumberKind::kSigned, /*priority=*/4, /*bitWidth=*/16))
        , fShort2(Type::MakeVectorType("short2", "s2", *fShort, /*columns=*/2))
        , fShort3(Type::MakeVectorType("short3", "s3", *fShort, /*columns=*/3))
        , fShort4(Type::MakeVectorType("short4", "s4", *fShort, /*columns=*/4))
        , fUShort(Type::MakeScalarType(
                  "ushort", "S", Type::NumberKind::kUnsigned, /*priority=*/3, /*bitWidth=*/16))
        , fUShort2(Type::MakeVectorType("ushort2", "S2", *fUShort, /*columns=*/2))
        , fUShort3(Type::MakeVectorType("ushort3", "S3", *fUShort, /*columns=*/3))
        , fUShort4(Type::MakeVectorType("ushort4", "S4", *fUShort, /*columns=*/4))
        , fBool(Type::MakeScalarType(
                  "bool", "b", Type::NumberKind::kBoolean, /*priority=*/0, /*bitWidth=*/1))
        , fBool2(Type::MakeVectorType("bool2", "b2", *fBool, /*columns=*/2))
        , fBool3(Type::MakeVectorType("bool3", "b3", *fBool, /*columns=*/3))
        , fBool4(Type::MakeVectorType("bool4", "b4", *fBool, /*columns=*/4))
        , fInvalid(Type::MakeSpecialType("<INVALID>", "O", Type::TypeKind::kOther))
        , fPoison(Type::MakeSpecialType(Compiler::POISON_TAG, "P", Type::TypeKind::kOther))
        , fVoid(Type::MakeSpecialType("void", "v", Type::TypeKind::kVoid))
        , fFloatLiteral(Type::MakeLiteralType("$floatLiteral", *fFloat, /*priority=*/8))
        , fIntLiteral(Type::MakeLiteralType("$intLiteral", *fInt, /*priority=*/5))
        , fFloat2x2(Type::MakeMatrixType("float2x2", "f22", *fFloat, /*columns=*/2, /*rows=*/2))
        , fFloat2x3(Type::MakeMatrixType("float2x3", "f23", *fFloat, /*columns=*/2, /*rows=*/3))
        , fFloat2x4(Type::MakeMatrixType("float2x4", "f24", *fFloat, /*columns=*/2, /*rows=*/4))
        , fFloat3x2(Type::MakeMatrixType("float3x2", "f32", *fFloat, /*columns=*/3, /*rows=*/2))
        , fFloat3x3(Type::MakeMatrixType("float3x3", "f33", *fFloat, /*columns=*/3, /*rows=*/3))
        , fFloat3x4(Type::MakeMatrixType("float3x4", "f34", *fFloat, /*columns=*/3, /*rows=*/4))
        , fFloat4x2(Type::MakeMatrixType("float4x2", "f42", *fFloat, /*columns=*/4, /*rows=*/2))
        , fFloat4x3(Type::MakeMatrixType("float4x3", "f43", *fFloat, /*columns=*/4, /*rows=*/3))
        , fFloat4x4(Type::MakeMatrixType("float4x4", "f44", *fFloat, /*columns=*/4, /*rows=*/4))
        , fHalf2x2(Type::MakeMatrixType("half2x2", "h22", *fHalf, /*columns=*/2, /*rows=*/2))
        , fHalf2x3(Type::MakeMatrixType("half2x3", "h23", *fHalf, /*columns=*/2, /*rows=*/3))
        , fHalf2x4(Type::MakeMatrixType("half2x4", "h24", *fHalf, /*columns=*/2, /*rows=*/4))
        , fHalf3x2(Type::MakeMatrixType("half3x2", "h32", *fHalf, /*columns=*/3, /*rows=*/2))
        , fHalf3x3(Type::MakeMatrixType("half3x3", "h33", *fHalf, /*columns=*/3, /*rows=*/3))
        , fHalf3x4(Type::MakeMatrixType("half3x4", "h34", *fHalf, /*columns=*/3, /*rows=*/4))
        , fHalf4x2(Type::MakeMatrixType("half4x2", "h42", *fHalf, /*columns=*/4, /*rows=*/2))
        , fHalf4x3(Type::MakeMatrixType("half4x3", "h43", *fHalf, /*columns=*/4, /*rows=*/3))
        , fHalf4x4(Type::MakeMatrixType("half4x4", "h44", *fHalf, /*columns=*/4, /*rows=*/4))
        , fTexture2D(Type::MakeTextureType("texture2D",
                                           SpvDim2D,
                                           /*isDepth=*/false,
                                           /*isArrayedTexture=*/false,
                                           /*isMultisampled=*/false,
                                           /*isSampled=*/true))
        , fTextureExternalOES(Type::MakeTextureType("textureExternalOES",
                                                    SpvDim2D,
                                                    /*isDepth=*/false,
                                                    /*isArrayedTexture=*/false,
                                                    /*isMultisampled=*/false,
                                                    /*isSampled=*/true))
        , fTexture2DRect(Type::MakeTextureType("texture2DRect",
                                               SpvDimRect,
                                               /*isDepth=*/false,
                                               /*isArrayedTexture=*/false,
                                               /*isMultisampled=*/false,
                                               /*isSampled=*/true))
        , fSampler2D(Type::MakeSamplerType("sampler2D", *fTexture2D))
        , fSamplerExternalOES(Type::MakeSamplerType("samplerExternalOES", *fTextureExternalOES))
        , fSampler2DRect(Type::MakeSamplerType("sampler2DRect", *fTexture2DRect))

        , fSampler(Type::MakeSpecialType("sampler", "ss", Type::TypeKind::kSeparateSampler))

        , fSubpassInput(Type::MakeTextureType("subpassInput",
                                              SpvDimSubpassData,
                                              /*isDepth=*/false,
                                              /*isArrayedTexture=*/false,
                                              /*isMultisampled=*/false,
                                              /*isSampled=*/false))
        , fSubpassInputMS(Type::MakeTextureType("subpassInputMS",
                                                SpvDimSubpassData,
                                                /*isDepth=*/false,
                                                /*isArrayedTexture=*/false,
                                                /*isMultisampled=*/true,
                                                /*isSampled=*/false))

        , fMat(Type::MakeGenericType("$mat", SkSpan<const Type* const>({fFloat2x2.get(), fFloat2x3.get(), fFloat2x4.get(),
                                                                        fFloat3x2.get(), fFloat3x3.get(), fFloat3x4.get(),
                                                                        fFloat4x2.get(), fFloat4x3.get(), fFloat4x4.get()})))
        , fHMat(Type::MakeGenericType("$hmat", SkSpan<const Type* const>({fHalf2x2.get(), fHalf2x3.get(), fHalf2x4.get(),
                                                                          fHalf3x2.get(), fHalf3x3.get(), fHalf3x4.get(),
                                                                          fHalf4x2.get(), fHalf4x3.get(), fHalf4x4.get()})))
        , fSkCaps(Type::MakeSpecialType("$sk_Caps", "O", Type::TypeKind::kOther))
        , fColorFilter(Type::MakeSpecialType("colorFilter", "CF", Type::TypeKind::kColorFilter))
        , fShader(Type::MakeSpecialType("shader", "SH", Type::TypeKind::kShader))
        , fBlender(Type::MakeSpecialType("blender", "B", Type::TypeKind::kBlender)) {

    for (const AliasInitializationData& data : kAliasData) {
        auto& dest = const_cast<std::unique_ptr<Type>&>(this->*data.dest);
        const Type* target = (this->*data.targetType).get();
        dest = Type::MakeAliasType(data.name, *target);
    }

    for (const Generic4InitializationData& data : kGeneric4Data) {
        auto& dest = const_cast<std::unique_ptr<Type>&>(this->*data.dest);
        const Type* type0 = (this->*data.type0).get();
        const Type* type1 = (this->*data.type1).get();
        const Type* type2 = (this->*data.type2).get();
        const Type* type3 = (this->*data.type3).get();
        dest = Type::MakeGenericType(data.name, {type0, type1, type2, type3});
    }
}

}  // namespace SkSL
