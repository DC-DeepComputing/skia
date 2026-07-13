/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_SkContextPriv_DEFINED
#define skgpu_graphite_SkContextPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkContext.h"
#include "src/gpu/graphite/QueueManager.h"
#include "src/gpu/graphite/SharedContext.h"

#include <memory>

class SkPixmap;
struct SkImageInfo;

namespace skcpu {
class ContextImpl;
}

namespace skcontext {

class Caps;
class GlobalCache;
class Recorder;
class RendererProvider;
class ResourceProvider;
class ShaderCodeDictionary;
class TextureProxyView;
enum class PathRendererStrategy;
struct SkContextOptions;
class SkSharedContext;
class SkContext;

/** Class that adds methods to SkContext that are only intended for use internal to Skia.
    This class is purely a privileged window into SkContext. It should never have additional
    data members or virtual methods. */
class SkContextPriv {
private:
    static void Init() {
        // SkGraphics::Init() must be thread-safe and idempotent.
        SkCpu::CacheRuntimeFeatures();
        SkOpts::Init();
        SkOpts::Init_BitmapProcState();
        SkOpts::Init_BlitMask();
        SkOpts::Init_BlitRow();
        SkOpts::Init_Memset();
        SkOpts::Init_Swizzler();
    }

    friend class SkContext;  // to construct/copy this type.

    SkContext* fSkContext;
};

// This class is friended by the Context and allows the backend ContextFactory functions to
// trampoline through this to call the private Context ctor. We can't directly friend the factory
// functions in Context because they are in a different namespace and we don't want to declare the
// functions in Context.h
class ContextCtorAccessor {
public:
    static std::unique_ptr<Context> MakeContext(sk_sp<SharedContext>,
                                                std::unique_ptr<QueueManager>,
                                                const ContextOptions&);
};

}  // namespace skcontext

#endif  // skgpu_graphite_SkContextPriv_DEFINED
