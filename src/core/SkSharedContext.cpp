/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnifiedContext_SharedContext_DEFINED
#define SkUnifiedContext_SharedContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/SingleOwner.h"
#include "src/capture/SkCaptureManager.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/PipelineManager.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

#include <memory>

namespace SkUnifiedContext {

class BackendTexture;
class Caps;
class CommandBuffer;
class Context;
class PersistentPipelineStorage;
class RendererProvider;
class ResourceProvider;
class TextureInfo;
class ThreadSafeResourceProvider;

class SkSharedContext : public SkRefCnt {};

}  // namespace SkUnifiedContext

#endif  // SkUnifiedContext_SharedContext_DEFINED
