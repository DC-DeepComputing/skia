/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ScratchResourceManager.h"

#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

#if defined(SK_DEBUG)
bool ProxyReadCountMap::hasPendingReads() const {
    bool hasPendingReads = false;
    fCounts.foreach([&hasPendingReads](const TextureProxy*, int proxyReadCount) {
        hasPendingReads |= (proxyReadCount > 0);
    });
    return hasPendingReads;
}
#endif

ScratchResourceManager::ScratchResourceManager(ResourceProvider* resourceProvider,
                                               std::unique_ptr<ProxyReadCountMap> proxyCounts)
        : fResourceProvider(resourceProvider)
        , fProxyReadCounts(std::move(proxyCounts))
        , fTextureLabelAndUsageMap(std::make_unique<TextureLabelAndUsageMap>()) {
    SkASSERT(resourceProvider);
    SkASSERT(fProxyReadCounts);
    SkASSERT(fTextureLabelAndUsageMap);
}

ScratchResourceManager::~ScratchResourceManager() {
    SkASSERT(fUnavailable.empty());
    SkASSERT(!fProxyReadCounts->hasPendingReads());
}

sk_sp<Texture> ScratchResourceManager::getScratchTexture(SkISize dimensions,
                                                         const TextureInfo& info,
                                                         std::string_view label,
                                                         SkEnumBitMask<TextureUsageFlags> usages) {
    sk_sp<Texture> scratchTexture = fResourceProvider->findOrCreateScratchTexture(
            dimensions, info, label, usages, fUnavailable);

    // Store the returned scratch texture into fUnavailable so that it is filtered from the
    // ResourceCache when going through *this* ScratchResourceManager. But the scratch texture will
    // remain visible to other Recorders.
    SkASSERT(!fUnavailable.contains(scratchTexture.get()));
    fUnavailable.add(scratchTexture.get());

    // Check to see if the scratchTexture is already listed in the map of textures.
    for (auto& textureLabelAndUsage : *fTextureLabelAndUsageMap) {
        Texture* tex = textureLabelAndUsage.first;
        if (scratchTexture.get() != tex) {
            continue;
        }

        // Update the texture's mapped string label and texture usage flags (adding, not replacing)
        ScratchResourceManager::LabelAndUsage labelAndUsage = textureLabelAndUsage.second;
        // assert that ScratchTexture_ is prepended to label
        // if (labelAndUsage.first.empty()) {
        //     labelAndUsage.first = "ScratchTexture_"
        //     labelAndUsage.first = label;
        // } else {
            labelAndUsage.first.append("_");
            labelAndUsage.first.append(label);
        // }
        labelAndUsage.second |= usages;
        return scratchTexture;
    }

    // TODO(nicolettep): Make all these std::strings that are static into better data type
    std::string scratchTexLabelPrefix = "ScratchTexture";
    std::string prefixAndLabel = scratchTexLabelPrefix;
    if (!label.empty()) {
        prefixAndLabel.append("_");
        prefixAndLabel.append(label);
    }
    // Note that we do not want to include the scratch texture's existing label, which will be
    // used as a point of comparison against the label stored here to determine whether updates
    // are needed.
    // TODO(nicolettep): Consider base label for textures that have no dynamic string labels.
    fTextureLabelAndUsageMap->push_back(
            std::pair<Texture*, LabelAndUsage>(scratchTexture.get(),
                                               LabelAndUsage(prefixAndLabel, usages)));

    return scratchTexture;
}

void ScratchResourceManager::snapTextureLabelAndUsageMap(
        std::unique_ptr<const TextureLabelAndUsageMap>& textureLabelAndUsageMap) {
    SkASSERT(fTextureLabelAndUsageMap);

    textureLabelAndUsageMap =
            std::make_unique<const TextureLabelAndUsageMap>(*fTextureLabelAndUsageMap);

    // Upon snapping the label/usage information for scratch textures, clear out the label and usage
    // maps. This includes the scratch texture pointers as well since they are trivial enough
    // to re-populate and help avoid referencing or updating resources that aren't actually used by
    // a Recording.
    fTextureLabelAndUsageMap->clear();
}

void ScratchResourceManager::returnTexture(sk_sp<Texture> texture) {
    // Fails if trying to return a resource that didn't come from getScratchTexture()
    SkASSERT(fUnavailable.contains(texture.get()));
    fUnavailable.remove(texture.get());
}

void ScratchResourceManager::pushScope() {
    // Push a null pointer to mark the beginning of the list of listeners in the next depth
    fListenerStack.push_back(nullptr);
}

void ScratchResourceManager::popScope() {
    // Must have at least the null element to start the scope being popped
    SkASSERT(!fListenerStack.empty());

    // TODO: Assert that the current sublist is empty (i.e. the back element is a null pointer) but
    // for now skip over them and leave them un-invoked to keep the unconsumed scratch resources
    // out of the pool so they remain valid in later recordings.
    int n = 0;
    while (fListenerStack.fromBack(n)) {
        n++;
    }
    SkASSERT(n < fListenerStack.size() && fListenerStack.fromBack(n) == nullptr);
    // Remove all non-null listeners after the most recent null entry AND the null entry
    fListenerStack.pop_back_n(n + 1);
}

void ScratchResourceManager::notifyResourcesConsumed() {
    // Should only be called inside a scope
    SkASSERT(!fListenerStack.empty());

    int n = 0;
    while (PendingUseListener* listener = fListenerStack.fromBack(n)) {
        listener->onUseCompleted(this);
        n++;
    }
    SkASSERT(n < fListenerStack.size() && fListenerStack.fromBack(n) == nullptr);
    // Remove all non-null listeners that were just invoked, but do not remove the null entry that
    // marks the start of this scope boundary.
    if (n > 0) {
        fListenerStack.pop_back_n(n);
    }
}

void ScratchResourceManager::markResourceInUse(PendingUseListener* listener) {
    // Should only be called inside a scope
    SkASSERT(!fListenerStack.empty());
    fListenerStack.push_back(listener);
}

}  // namespace skgpu::graphite
