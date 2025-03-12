/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RenderPassDesc.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

namespace {

const char* to_str(LoadOp op) {
    switch (op) {
        case LoadOp::kLoad:    return "load";
        case LoadOp::kClear:   return "clear";
        case LoadOp::kDiscard: return "discard";
    }

    SkUNREACHABLE;
}

const char* to_str(StoreOp op) {
    switch (op) {
        case StoreOp::kStore:   return "store";
        case StoreOp::kDiscard: return "discard";
    }

    SkUNREACHABLE;
}

} // anonymous namespace

RenderPassDesc RenderPassDesc::Make(const Caps* caps,
                                    const TextureInfo& targetInfo,
                                    LoadOp loadOp,
                                    StoreOp storeOp,
                                    SkEnumBitMask<DepthStencilFlags> depthStencilFlags,
                                    const std::array<float, 4>& clearColor,
                                    bool requiresMSAA,
                                    Swizzle writeSwizzle,
                                    const DstReadStrategy targetReadStrategy) {
    // It doesn't make sense to have a storeOp for our main target not be store. Why are we doing
    // this DrawPass then
    SkASSERT(storeOp == StoreOp::kStore);

    RenderPassDesc desc;
    // desc.fClearColor = clearColor;
    // desc.fClearDepth = 0.f; // Depth and stencil is currently always cleared to 0 if it's used
    // desc.fClearStencil = 0;
    desc.fWriteSwizzle = writeSwizzle;
    // Should a dst read be required later on, record what dst read strategy should be used. Must be
    // a valid strategy.
    SkASSERT(targetReadStrategy != DstReadStrategy::kNoneRequired);
    desc.fDstReadStrategyIfRequired = targetReadStrategy;

    TextureFormat colorFormat = TextureInfoPriv::ViewFormat(targetInfo);
    uint8_t sampleCount = caps->getDefaultSupportedSampleCount(colorFormat);

    // We need to handle MSAA with a color attachment if:
    const bool needsMSAAColorAttachment =
            requiresMSAA &&                              // it's required for the recorded draws,
            targetInfo.numSamples() == 1 &&              // the target isn't already MSAA'ed,
            !caps->msaaRenderToSingleSampledSupport() && // can't use an extension,
            sampleCount > 1;                             // and it's supported for the target format
    if (needsMSAAColorAttachment) {
        // We set the color and resolve attachments up the same regardless of if the backend ends up
        // using msaaRenderToSingleSampledSupport() to skip explicitly creating the MSAA attachment.
        // The color attachment (and any depth/stencil attachment) will use `sampleCount` and the
        // resolve attachment will be single-sampled.
        desc.fColorAttachment = {colorFormat,
                                 loadOp != LoadOp::kClear ? LoadOp::kDiscard : LoadOp::kClear,
                                 StoreOp::kDiscard,
                                 sampleCount};
        desc.fColorResolveAttachment = {colorFormat,
                                        loadOp != LoadOp::kLoad ? LoadOp::kDiscard : LoadOp::kLoad,
                                        storeOp,
                                        /*sampleCount=*/1};
    } else {
        // The target will be the color attachment and skip configuring the resolve attachment.
        SkASSERT(desc.fColorResolveAttachment.fFormat == TextureFormat::kUnsupported);
        desc.fColorAttachment = {colorFormat, loadOp, storeOp, SkTo<uint8_t>(targetInfo.numSamples())};
    }

    if (depthStencilFlags != DepthStencilFlags::kNone) {
        TextureFormat dsFormat = caps->getDepthStencilFormat(depthStencilFlags);
        SkASSERT(dsFormat != TextureFormat::kUnsupported);
        // Depth and stencil values are currently always cleared and don't need to persist.
        // The sample count matches the color attachment.
        desc.fDepthStencilAttachment = {dsFormat, LoadOp::kClear, StoreOp::kDiscard, desc.fColorAttachment.fSampleCount};
    } else {
        SkASSERT(desc.fDepthStencilAttachment.fFormat == TextureFormat::kUnsupported);
    }

    // The render pass's overall sample count will be `sampleCount`, which either matches the
    // color attachment or will be the implicit sample count to use msaa-render-to-single-sampling.
    desc.fSampleCount = sampleCount;
    return desc;
}

SkString RenderPassDesc::toString() const {
    // Note: Purposefully omitting the fDstReadStrategyIfRequired attribute. Since the shader /
    // pipeline actually determines whether a dst read is needed, it would make more sense to
    // report the actual used dst read strategy there.
    return SkStringPrintf("RP(color: %s, resolve: %s, ds: %s, swizzle: %s, "
                          "clear: c(%f,%f,%f,%f), d(%f), s(0x%02x))",
                          fColorAttachment.toString().c_str(),
                          fColorResolveAttachment.toString().c_str(),
                          fDepthStencilAttachment.toString().c_str(),
                          fWriteSwizzle.asString().c_str(),
                          fClearColor[0], fClearColor[1], fClearColor[2], fClearColor[3],
                          fClearDepth,
                          fClearStencil);
}

SkString RenderPassDesc::toPipelineLabel() const {
    // Given current policies, these assumptions should hold and mean the conciseness in the label
    // is still unambiguous.
    SkASSERT(fColorAttachment.fFormat != TextureFormat::kUnsupported);
    SkASSERT(fColorResolveAttachment.fFormat == TextureFormat::kUnsupported ||
             fColorResolveAttachment.fFormat == fColorAttachment.fFormat);
    SkASSERT(fDepthStencilAttachment.fFormat == TextureFormat::kUnsupported ||
             fDepthStencilAttachment.fSampleCount == fColorAttachment.fSampleCount);
    SkASSERT(fColorResolveAttachment.fFormat == TextureFormat::kUnsupported ||
             fColorResolveAttachment.fSampleCount == 1);
    SkASSERT(fColorAttachment.fSampleCount == fSampleCount ||
             (fColorAttachment.fSampleCount == 1 && fSampleCount > 1));

    const char* colorFormatStr = TextureFormatName(fColorAttachment.fFormat);
    const char* dsFormatStr = "{}";
    if (fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
        dsFormatStr = TextureFormatName(fDepthStencilAttachment.fFormat);
    }

    // This intentionally only includes the fixed state that impacts pipeline compilation.
    // We include the load op of the color attachment when there is a resolve attachment because
    // the load may trigger a different renderpass description.
    const char* colorLoadStr = "";
    if (fColorAttachment.fLoadOp == LoadOp::kLoad &&
        fColorResolveAttachment.fFormat != TextureFormat::kUnsupported) {
        colorLoadStr = " w/ msaa load";
    }

    // There are three supported ways of achieving MSAA rendering that we distinguish compactly.

    // NOTE: This label does not differentiate between explicitly resolved MSAA color attachments
    // and MSAA-render-to-single-sample renderpasses. For a given set of Caps, we currently only
    // expect to generate one or the other variety.
    return SkStringPrintf("RP((%s+%s x%u).%s%s)",
                          colorFormatStr,
                          dsFormatStr,
                          fSampleCount,
                          fWriteSwizzle.asString().c_str(),
                          colorLoadStr);
}

SkString AttachmentDesc::toString() const {
    if (fFormat == TextureFormat::kUnsupported) {
        return SkString("{}");
    } else {
        return SkStringPrintf("{f: %s x%u, ops: %s->%s}",
                              TextureFormatName(fFormat),
                              fSampleCount,
                              to_str(fLoadOp),
                              to_str(fStoreOp));
    }
}

bool AttachmentDesc::isCompatible(const TextureInfo& texInfo) const {
    return fFormat == TextureInfoPriv::ViewFormat(texInfo) &&
           fSampleCount == texInfo.numSamples();
}

} // namespace skgpu::graphite
