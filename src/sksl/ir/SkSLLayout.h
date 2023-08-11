/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LAYOUT
#define SKSL_LAYOUT

#include "src/base/SkEnumBitMask.h"
#include "src/sksl/SkSLLexer.h"

#include <string>

namespace SkSL {

class Context;
class Position;

template <Token::Kind t>
struct TokenToLayoutBit {
    static_assert(t >= Token::Kind::TK_ORIGIN_UPPER_LEFT);
    static constexpr int bit = 1 << ((int)t - (int)Token::Kind::TK_ORIGIN_UPPER_LEFT);
};

enum class LayoutFlag : int {
    kNone                       = 0,
    kAll                        = ~0,

    kOriginUpperLeft            = TokenToLayoutBit<Token::Kind::TK_ORIGIN_UPPER_LEFT>::bit,
    kPushConstant               = TokenToLayoutBit<Token::Kind::TK_PUSH_CONSTANT>::bit,
    kBlendSupportAllEquations   = TokenToLayoutBit<Token::Kind::TK_BLEND_SUPPORT_ALL_EQUATIONS>::bit,
    kColor                      = TokenToLayoutBit<Token::Kind::TK_COLOR>::bit,

    // These flags indicate if the qualifier appeared, regardless of the accompanying value.
    kLocation                   = TokenToLayoutBit<Token::Kind::TK_LOCATION>::bit,
    kOffset                     = TokenToLayoutBit<Token::Kind::TK_OFFSET>::bit,
    kBinding                    = TokenToLayoutBit<Token::Kind::TK_BINDING>::bit,
    kTexture                    = TokenToLayoutBit<Token::Kind::TK_TEXTURE>::bit,
    kSampler                    = TokenToLayoutBit<Token::Kind::TK_SAMPLER>::bit,
    kIndex                      = TokenToLayoutBit<Token::Kind::TK_INDEX>::bit,
    kSet                        = TokenToLayoutBit<Token::Kind::TK_SET>::bit,
    kBuiltin                    = TokenToLayoutBit<Token::Kind::TK_BUILTIN>::bit,
    kInputAttachmentIndex       = TokenToLayoutBit<Token::Kind::TK_INPUT_ATTACHMENT_INDEX>::bit,

    // These flags indicate the backend type; only one at most can be set.
    kSPIRV                      = TokenToLayoutBit<Token::Kind::TK_SPIRV>::bit,
    kMetal                      = TokenToLayoutBit<Token::Kind::TK_METAL>::bit,
    kWGSL                       = TokenToLayoutBit<Token::Kind::TK_WGSL>::bit,

    kAllBackends                = kSPIRV | kMetal | kWGSL,

    // These flags indicate the pixel format; only one at most can be set.
    // (https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Image_formats)
    kRGBA8                      = TokenToLayoutBit<Token::Kind::TK_RGBA8>::bit,
    kRGBA32F                    = TokenToLayoutBit<Token::Kind::TK_RGBA32F>::bit,
    kR32F                       = TokenToLayoutBit<Token::Kind::TK_R32F>::bit,

    kAllPixelFormats            = kRGBA8 | kRGBA32F | kR32F,

    // The local invocation size of a compute program.
    kLocalSizeX                 = TokenToLayoutBit<Token::Kind::TK_LOCAL_SIZE_X>::bit,
    kLocalSizeY                 = TokenToLayoutBit<Token::Kind::TK_LOCAL_SIZE_Y>::bit,
    kLocalSizeZ                 = TokenToLayoutBit<Token::Kind::TK_LOCAL_SIZE_Z>::bit,
};

}  // namespace SkSL

SK_MAKE_BITMASK_OPS(SkSL::LayoutFlag);

namespace SkSL {

using LayoutFlags = SkEnumBitMask<SkSL::LayoutFlag>;

/**
 * Represents a layout block appearing before a variable declaration, as in:
 *
 * layout (location = 0) int x;
 */
struct Layout {
    Layout(LayoutFlags flags, int location, int offset, int binding, int index, int set,
           int builtin, int inputAttachmentIndex)
            : fFlags(flags)
            , fLocation(location)
            , fOffset(offset)
            , fBinding(binding)
            , fIndex(index)
            , fSet(set)
            , fBuiltin(builtin)
            , fInputAttachmentIndex(inputAttachmentIndex) {}

    constexpr Layout() = default;

    static Layout builtin(int builtin) {
        Layout result;
        result.fBuiltin = builtin;
        return result;
    }

    std::string description() const;
    std::string paddedDescription() const;

    /**
     * Verifies that only permitted layout flags are included. Reports errors and returns false in
     * the event of a violation.
     */
    bool checkPermittedLayout(const Context& context,
                              Position pos,
                              LayoutFlags permittedLayoutFlags) const;

    bool operator==(const Layout& other) const;

    bool operator!=(const Layout& other) const {
        return !(*this == other);
    }

    LayoutFlags fFlags = LayoutFlag::kNone;
    int fLocation = -1;
    int fOffset = -1;
    int fBinding = -1;
    int fTexture = -1;
    int fSampler = -1;
    int fIndex = -1;
    int fSet = -1;
    // builtin comes from SPIR-V and identifies which particular builtin value this object
    // represents.
    int fBuiltin = -1;
    // input_attachment_index comes from Vulkan/SPIR-V to connect a shader variable to the a
    // corresponding attachment on the subpass in which the shader is being used.
    int fInputAttachmentIndex = -1;

    // The local invocation size dimensions of a compute program.
    int fLocalSizeX = -1;
    int fLocalSizeY = -1;
    int fLocalSizeZ = -1;
};

}  // namespace SkSL

#endif
