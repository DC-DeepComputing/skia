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

namespace SkUnifiedContext {

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

    friend class SkContext;  // to construct/copy this type.

    SkContext* fSkContext;
};

}  // namespace SkUnifiedContext

#endif  // skgpu_graphite_SkContextPriv_DEFINED
