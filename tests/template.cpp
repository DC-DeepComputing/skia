/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cassert>
#include <cstdint>
#include <memory>

//
// SkShaderBase.h
//
#define ALL_SHADERS(M) \
    M(Color)           \
    M(LinearGradient)  \
    M(Image)

enum class ShaderTypeID {
#define M(Type) k ## Type ,
    ALL_SHADERS(M)
#undef M
};

class SkShaderBase {
public:
    SkShaderBase() {}
    virtual ~SkShaderBase() {}

    // We could have the base class store the TypeID, so it doesn't cost a virtual to get it.
    // I went with the simple approach here...
    virtual ShaderTypeID getType() const = 0;
};

class FP;

// friend declaration is optional. We could also have getters for all data fields on all shaders
#define SHADER_TYPE(Type) \
    ShaderTypeID getType() const override { return ShaderTypeID::k ## Type; } \
    friend std::unique_ptr<FP> MakeFP(const Sk ## Type ## Shader*);

// SkColorShader.h
class SkColorShader : public SkShaderBase {
public:
    SkColorShader(uint32_t color) : fColor(color) {}
    // This is the only extra thing that each derived shader type needs to do:
    SHADER_TYPE(Color)
private:
    uint32_t fColor;
};

// etc...
class SkLinearGradientShader : public SkShaderBase {
    SHADER_TYPE(LinearGradient)
};
class SkImageShader : public SkShaderBase {
    SHADER_TYPE(Image)
};

// Now, in GPU code:
class FP {};

class ColorFP : public FP {};
class LinearGradientFP : public FP {};
class ImageFP : public FP {};

// These are auto-declared, then hand-authored (see the bottom of the code)
template <typename T>
std::unique_ptr<FP> MakeFP(const T*);

// Actual entry point called by GPU code, to turn any shader into an FP:
std::unique_ptr<FP> shader_as_FP(const SkShaderBase* shader) {
    switch (shader->getType()) {
#define M(Type)                       \
        case ShaderTypeID::k ## Type: \
            return MakeFP(static_cast<const Sk ## Type ## Shader*>(shader));
        ALL_SHADERS(M)
#undef M
    }
}

// Human-authored FP functions for each type:
std::unique_ptr<FP> MakeFP(const SkColorShader* shader) {
    return std::make_unique<ColorFP>();
}

std::unique_ptr<FP> MakeFP(const SkLinearGradientShader* shader) {
    return std::make_unique<LinearGradientFP>();
}

std::unique_ptr<FP> MakeFP(const SkImageShader* shader) {
    return std::make_unique<ImageFP>();
}
