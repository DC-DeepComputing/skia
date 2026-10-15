/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureProxyView_DEFINED
#define skgpu_graphite_TextureProxyView_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/SkEnumBitMask.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/TextureProxy.h"

enum class SkBackingFit;

namespace skgpu::graphite {

class Recorder;
enum class UsageLabelFlag : uint64_t;

class TextureProxyView {
public:
    TextureProxyView() = default;

    TextureProxyView(sk_sp<TextureProxy> proxy, Swizzle swizzle)
            : fProxy(std::move(proxy)), fSwizzle(swizzle) {}

    TextureProxyView(sk_sp<TextureProxy> proxy, Swizzle swizzle, Origin origin)
            : fProxy(std::move(proxy)), fSwizzle(swizzle), fOrigin(origin) {}

    // This entry point is used when we don't care about the swizzle and assume TopLeft origin.
    // Note that texture proxies should be limited to having one resource usage flag, but scratch
    // texture resources are allowed to have multiple. Therefore, when creating a proxy view from
    // a proxy, pass in usage flags and a label which may differ from that of fProxy.
    explicit TextureProxyView(sk_sp<TextureProxy> proxy,
                              LabelUsageBitMask usageFlags,
                              std::string_view label = {})
            : fProxy(std::move(proxy)) {}

    TextureProxyView(TextureProxyView&& view) = default;
    TextureProxyView(const TextureProxyView&) = default;

    explicit operator bool() const { return SkToBool(fProxy.get()); }

    TextureProxyView& operator=(const TextureProxyView&) = default;
    TextureProxyView& operator=(TextureProxyView&& view) = default;

    bool operator==(const TextureProxyView& view) const {
        return fProxy == view.fProxy &&
               fSwizzle == view.fSwizzle &&
               fOrigin == view.fOrigin;
    }
    bool operator!=(const TextureProxyView& other) const { return !(*this == other); }

    int width() const { return this->proxy()->dimensions().width(); }
    int height() const { return this->proxy()->dimensions().height(); }
    SkISize dimensions() const { return this->proxy()->dimensions(); }

    Protected isProtected() const {
        return fProxy ? fProxy->isProtected() : Protected::kNo;
    }
    Mipmapped mipmapped() const {
        return fProxy ? fProxy->mipmapped() : Mipmapped::kNo;
    }

    TextureProxy* proxy() const { return fProxy.get(); }
    sk_sp<TextureProxy> refProxy() const { return fProxy; }

    Swizzle swizzle() const { return fSwizzle; }
    Origin origin() const { return fOrigin; }

    void concatSwizzle(Swizzle swizzle) {
        fSwizzle = Swizzle::Concat(fSwizzle, swizzle);
    }

    // makeSwizzle returns a new view with 'swizzle' composed on to this view's existing swizzle
    TextureProxyView makeSwizzle(Swizzle swizzle) const & {
        return {fProxy, Swizzle::Concat(fSwizzle, swizzle), fOrigin};
    }

    TextureProxyView makeSwizzle(Swizzle swizzle) && {
        return {std::move(fProxy), Swizzle::Concat(fSwizzle, swizzle), fOrigin};
    }

    // resetSwizzle returns a new view that uses 'swizzle' and disregards this view's prior swizzle.
    TextureProxyView replaceSwizzle(Swizzle swizzle) const {
        return {fProxy, swizzle, fOrigin};
    }

    void reset() {
        *this = {};
    }

    // This does not reset the swizzle, so the View can still be used to access those
    // properties associated with the detached proxy.
    sk_sp<TextureProxy> detachProxy() {
        return std::move(fProxy);
    }

private:
    sk_sp<TextureProxy> fProxy;
    Swizzle fSwizzle;
    Origin fOrigin = Origin::kTopLeft;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_TextureProxyView_DEFINED
