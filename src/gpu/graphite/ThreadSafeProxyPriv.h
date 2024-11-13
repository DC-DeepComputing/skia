/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ThreadSafeProxyPriv_DEFINED
#define skgpu_graphite_ThreadSafeProxyPriv_DEFINED

#include "include/gpu/graphite/ThreadSafeProxy.h"

class Caps;

namespace skgpu::graphite {

/** Class that adds methods to ThreadSafeProxy that are only intended for use internal to Skia.
This class is purely a privileged window into ThreadSafeProxy. It should never have additional
data members or virtual methods. */
class ThreadSafeProxyPriv {
public:
    const Caps* caps() const { return fThreadSafeProxy->fSharedContext->caps(); }
    const ShaderCodeDictionary* shaderCodeDictionary() const {
        return fThreadSafeProxy->fSharedContext->shaderCodeDictionary();
    }
    ShaderCodeDictionary* shaderCodeDictionary() {
        return fThreadSafeProxy->fSharedContext->shaderCodeDictionary();
    }
#if 0
    const GlobalCache* globalCache() const {
        return fContext->fSharedContext->globalCache();
    }
    GlobalCache* globalCache() {
        return fContext->fSharedContext->globalCache();
    }
#endif
    const RendererProvider* rendererProvider() const {
        return fThreadSafeProxy->fSharedContext->rendererProvider();
    }
    SharedContext* sharedContext() {
        return fThreadSafeProxy->fSharedContext.get();
    }

private:
    friend class ThreadSafeProxy; // to construct/copy this type.

    explicit ThreadSafeProxyPriv(ThreadSafeProxy* threadSafeProxy)
            : fThreadSafeProxy(threadSafeProxy) {}

    ThreadSafeProxyPriv& operator=(const ThreadSafeProxyPriv&) = delete;

    // No taking addresses of this type.
    const ThreadSafeProxyPriv* operator&() const;
    ThreadSafeProxyPriv *operator&();

    ThreadSafeProxy* fThreadSafeProxy;
};

inline ThreadSafeProxyPriv ThreadSafeProxy::priv() { return ThreadSafeProxyPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const ThreadSafeProxyPriv ThreadSafeProxy::priv() const {
    return ThreadSafeProxyPriv(const_cast<ThreadSafeProxy *>(this));
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_ThreadSafeProxyPriv_DEFINED
