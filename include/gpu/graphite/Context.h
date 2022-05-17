/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Context_DEFINED
#define skgpu_graphite_Context_DEFINED

#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTArray.h"

#include "include/gpu/graphite/GraphiteTypes.h"

class SkRuntimeEffect;
class SkShaderCodeDictionary;

namespace skgpu::graphite {

class BackendTexture;
class CommandBuffer;
class ContextPriv;
class GlobalCache;
class Gpu;
struct MtlBackendContext;
class Recorder;
class Recording;
class TextureInfo;

struct ShaderCombo {
    enum class ShaderType {
        kNone, // does not modify color buffer, e.g. depth and/or stencil only
        kSolidColor,
        kLinearGradient,
        kRadialGradient,
        kSweepGradient,
        kConicalGradient
    };

    ShaderCombo() {}
    ShaderCombo(std::vector<ShaderType> types,
                std::vector<SkTileMode> tileModes)
            : fTypes(std::move(types))
            , fTileModes(std::move(tileModes)) {
    }
    std::vector<ShaderType> fTypes;
    std::vector<SkTileMode> fTileModes;
};

static constexpr int kSkBlendModeCnt = static_cast<int>(SkBlendMode::kLastMode) + 1;

// If no blendmodes are specified as part of the combination, kSrcOver will be used.
struct PaintCombo {
public:
    PaintCombo() {
        this->reset();
    }

private:
    friend class CombinationBuilder;
    friend class Context;

    void reset() {
        fShaders.clear();
        fBlendModes = 0;
    }

    void add(ShaderCombo shaderCombo) { fShaders.push_back(shaderCombo); }

    void add(SkBlendMode bm) { fBlendModes |= 0x1 << (int) bm; }

    std::vector<ShaderCombo> fShaders;
    uint32_t fBlendModes;

    struct BlendModeIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = int;
        using pointer           = int*;  // or also value_type*
        using reference         = int&;  // or also value_type&

        BlendModeIterator(uint32_t blendModes, int index)
                : fBlendModes(blendModes)
                , fIndex(index) {}

//        reference operator*() const { return *fPtr; }
//        pointer operator->() { return fPtr; }

        // Prefix increment
        BlendModeIterator& operator++() {
            for (; fIndex < kSkBlendModeCnt; ++fIndex) {
                if (fBlendModes & (0x1 << fIndex)) {
                    break;
                }
            }
            return *this;
        }

        // Postfix increment
        BlendModeIterator operator++(int) { BlendModeIterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const BlendModeIterator& a, const BlendModeIterator& b) {
            return a.fBlendModes == b.fBlendModes &&  a.fIndex == b.fIndex;
        }
        friend bool operator!= (const BlendModeIterator& a, const BlendModeIterator& b) {
            return !(a == b);
        }

    private:
        uint32_t fBlendModes;
        int fIndex;
    };

    BlendModeIterator begin() { return BlendModeIterator(fBlendModes, 0); }
    BlendModeIterator end()   { return BlendModeIterator(fBlendModes, kSkBlendModeCnt); }
    BlendModeIterator cbegin() const { return BlendModeIterator(fBlendModes, 0); }
    BlendModeIterator cend()   const { return BlendModeIterator(fBlendModes, kSkBlendModeCnt); }
};

class CombinationBuilder {
public:

    void add(ShaderCombo shaderCombo) {
        fCombination.add(shaderCombo);
    }

    enum class BlendModeGroups {
        kPorterDuff,         // [ kClear .. kScreen ]
        kAdvanced,           // [ kOverlay .. kMultiply ]
        kTrouble,            // [ kHue .. kLuminosity ]
        kAll
    };

    // Blend Mode
    void add(SkBlendMode bm) { fCombination.add(bm); }
    void add(SkBlendMode rangeStart, SkBlendMode rangeEnd) {  // inclusive
        for (int i = (int)rangeStart; i <= (int) rangeEnd; ++i) {
            this->add((SkBlendMode) i);
        }
    }
    void add(BlendModeGroups group) {
        switch (group) {
            case BlendModeGroups::kPorterDuff:
                this->add(SkBlendMode::kClear, SkBlendMode::kScreen);
                break;
            case BlendModeGroups::kAdvanced:
                this->add(SkBlendMode::kOverlay, SkBlendMode::kMultiply);
                break;
            case BlendModeGroups::kTrouble:
                this->add(SkBlendMode::kHue, SkBlendMode::kLuminosity);
                break;
            case BlendModeGroups::kAll:
                this->add(SkBlendMode::kClear, SkBlendMode::kLastMode);
                break;
        }
    }

    const PaintCombo* snap() {
        if (!fCombination.fBlendModes) {
            fCombination.add(SkBlendMode::kSrcOver);
        }
        return &fCombination;
    }
    void reset() {
        fCombination.reset();
    }

private:
    PaintCombo fCombination;

};

// TODO: add SkShaderID and SkColorFilterID too
class SkBlenderID {
public:
    SkBlenderID() : fID(0) {}  // 0 is an invalid blender ID
    SkBlenderID(const SkBlenderID& src) : fID(src.fID) {}

    bool isValid() const { return fID > 0; }

    bool operator==(const SkBlenderID& other) const { return fID == other.fID; }

    SkBlenderID& operator=(const SkBlenderID& src) {
        fID = src.fID;
        return *this;
    }

private:
    friend class ::SkShaderCodeDictionary;   // for ctor and asUInt access

    SkBlenderID(uint32_t id) : fID(id) {}

    uint32_t asUInt() const { return fID; }

    uint32_t fID;
};

class Context final {
public:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    ~Context();

#ifdef SK_METAL
    static std::unique_ptr<Context> MakeMetal(const skgpu::graphite::MtlBackendContext&);
#endif

    BackendApi backend() const { return fBackend; }

    std::unique_ptr<Recorder> makeRecorder();

    void insertRecording(const InsertRecordingInfo&);
    void submit(SyncToCpu = SyncToCpu::kNo);

    /**
     * Checks whether any asynchronous work is complete and if so calls related callbacks.
     */
    void checkAsyncWorkCompletion();

    // TODO: add "SkShaderID addUserDefinedShader(sk_sp<SkRuntimeEffect>)" here
    // TODO: add "SkColorFilterID addUserDefinedColorFilter(sk_sp<SkRuntimeEffect>)" here
    SkBlenderID addUserDefinedBlender(sk_sp<SkRuntimeEffect>);

    void preCompile(const PaintCombo&);

    /**
     * Creates a new backend gpu texture matching the dimensinos and TextureInfo. If an invalid
     * TextureInfo or a TextureInfo Skia can't support is passed in, this will return an invalid
     * BackendTexture. Thus the client should check isValid on the returned BackendTexture to know
     * if it succeeded or not.
     *
     * If this does return a valid BackendTexture, the caller is required to use
     * Context::deleteBackendTexture to delete that texture.
     */
    BackendTexture createBackendTexture(SkISize dimensions, const TextureInfo&);

    /**
     * Called to delete the passed in BackendTexture. This should only be called if the
     * BackendTexture was created by calling Context::createBackendTexture. If the BackendTexture is
     * not valid or does not match the BackendApi of the Context then nothing happens.
     *
     * Otherwise this will delete/release the backend object that is wrapped in the BackendTexture.
     * The BackendTexture will be reset to an invalid state and should not be used again.
     */
    void deleteBackendTexture(BackendTexture&);

    // Provides access to functions that aren't part of the public API.
    ContextPriv priv();
    const ContextPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    Context(sk_sp<Gpu>, BackendApi);

private:
    friend class ContextPriv;

    sk_sp<CommandBuffer> fCurrentCommandBuffer;

    sk_sp<Gpu> fGpu;
    sk_sp<GlobalCache> fGlobalCache;
    BackendApi fBackend;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Context_DEFINED
