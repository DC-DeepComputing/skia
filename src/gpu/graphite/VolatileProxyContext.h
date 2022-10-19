/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VolatileProxyContext_DEFINED
#define skgpu_graphite_VolatileProxyContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkChecksum.h"

#include <unordered_set>

namespace skgpu::graphite {

class TextureProxy;

class VolatileProxyContext : public SkRefCnt {
public:
    VolatileProxyContext();
    ~VolatileProxyContext() override;

    void insert(sk_sp<TextureProxy> volatileProxy);

    struct ProxyHash {
        std::size_t operator()(const sk_sp<TextureProxy>& proxy) const {
            return SkGoodHash()(proxy.get());
        }
    };

    bool empty() const { return fVolatileProxies.empty(); }

    std::unordered_set<sk_sp<TextureProxy>, ProxyHash> fVolatileProxies;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VolatileProxyContext_DEFINED
