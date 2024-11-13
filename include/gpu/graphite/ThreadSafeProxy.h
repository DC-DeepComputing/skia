/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ThreadSafeProxy_DEFINED
#define skgpu_graphite_ThreadSafeProxy_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class SharedContext;
class ThreadSafeProxyPriv;

class SK_API ThreadSafeProxy : public SkRefCnt {
public:
    ~ThreadSafeProxy() override;

    // Provides access to functions that aren't part of the public API.
    ThreadSafeProxyPriv priv();
    const ThreadSafeProxyPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class ThreadSafeProxyPriv;
    friend class Context; // for ctor

    ThreadSafeProxy(sk_sp<SharedContext>);

    sk_sp<SharedContext> fSharedContext;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_ThreadSafeProxy_DEFINED
