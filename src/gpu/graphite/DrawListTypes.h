/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DrawListTypes_DEFINED
#define skgpu_graphite_DrawListTypes_DEFINED

#include "include/private/SkDebug.h"
#include "include/private/SkEnumBitMask.h"
#include "src/core/SkBlockAllocator.h"
#include "src/core/SkTBlockList.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PipelineData.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform.h"

#include <cstdint>
#include <functional>
#include <limits>
#include <optional>

namespace skgpu::graphite {


/**
 * Defines a bitmask that defines what types of buffer modifications are blocked by draws within a
 * Layer (when set on LayerKey) or the buffer modifications a draw can't overlap with (testMask).
 *
 * This table shows the BoundsFlags layer and test masks for the different types of renderer draws:
 *
 *     Draw                                | Layer Mask      | Test Mask
 *     ------------------------------------|-----------------|---------------------
 *     Opaque shading                      | Color           | None
 *     Non-opaque shading                  | Color           | Color
 *     Opaque shading + stencil cover      | Color + Stencil | Stencil
 *     Non-opaque shading + stencil cover  | Color + Stencil | Color + Stencil
 *     Stencil step for opaque shading     | Stencil         | Stencil
 *     Stencil step for non-opaque shading | Stencil         | Color + Stencil
 *     Depth-only                          | None            | Color
 *     Depth-only + stencil cover          | Stencil         | Color + Stencil
 *     Stencil step for depth-only         | Stencil         | Color + Stencil
 *
 *     NOTE: This table does not include MustBeDisjoint for brevity. MustBeDisjoint behaves similar
 *           to a Stencil dependency, except that it only affects intersections within a matching
 *           BindingList and Stencil affects any binding that also depends on Stencil.
 *
 * Shading draws always have Color in their layer key to enforce painter's order against new draws
 * that must blend. Opaque shading draws do not include Color in their test key because they can
 * draw early and resolve to the correct painter's order with a depth test.
 *
 * Stencil steps for shading draws (e.g. the step itself isn't shading) do not have Color in their
 * key so they can batch with depth-only draws, but their test mask matches that of their shading
 * step. This is so that the layer search can be done for a single step and be valid for the
 * remaining steps.
 *
 * Depth-only draws do not have Color in their layer key because they rely on the ClipStack's more
 * refined bounds checking to determine what is impacted. They include Color in their test mask,
 * however, to ensure they do not draw before shading draws whose z value shouldn't be impacted by
 * the depth clip's z value.
 *
 * If a render step uses the stencil buffer, Stencil must be in both the layer mask and the test
 * mask. Stencil-using draws must always be fully disjoint across the entire layer.
 */
enum class BoundsFlags {
    kNone    = 0x0, // No need to test against draws; only respect the stop layer when searching
    kStencil = 0x1, // Cannot intersect with anything that uses stencil; but could draw out of order
    kColor   = 0x2, // Cannot intersect with anything that uses color, and cannot be ordered earlier
    // Adds a requirement that draws within the same BindingList must be disjoint from each other,
    // even if there wasn't otherwise a stencil or color dependency. This arises in two cases:
    //  - blending requires barriers so we can't just rely on the GPU's rasterization order to
    //    handling the painter's order for us.
    //  - the draw belongs to a multi-step renderer and we don't want the intermediate steps to
    //    contaminate other draws within the same layer and renderer.
    kMustBeDisjoint = 0x4
};
SK_MAKE_BITMASK_OPS(BoundsFlags)

/**
 * BoundsTestResult describes how a new draw can be ordered with regards to a Layer. A layer can
 * completely block a draw, forcing it into a later layer. A draw can be added to the layer, and
 * can be allowed to draw before the layer. These last two are not mutually exclusive. It can be
 * helpful to continue searching for a deeper layer to keep the layer chain shallow. It is also
 * possible to allow a draw before the layer but not within the layer (if there just a stencil
 * overlap for instance).
 */
enum class BoundsTestResult {
    kBlocked = 0x0,            // The draw must go in a layer after the tested layer
    kAllowedInLayer = 0x1,     // The draw can go in the layer
    kAllowedBeforeLayer = 0x2, // The draw can go before the tested layer
};
SK_MAKE_BITMASK_OPS(BoundsTestResult)

struct LayerKey {
    GraphicsPipelineCache::Index fPipelineIndex;
    TextureDataCache::Index fTextureIndex;

    // Set to Invalid for BindingLists when SSBOs are used; for SSBOs, each draw's uniform index
    // is stored on the Draw itself.
    UniformDataCache::Index fUniformIndex;

    // New draws with a testMask that overlaps with `fFlags` must be checked for bounds
    // intersections with the draws in the BindingList for this key.
    SkEnumBitMask<BoundsFlags> fFlags;

    int32_t hash() const {
        // Smash bits together, it's fine
        return ((int32_t)fPipelineIndex << 22) | ((int32_t) fTextureIndex << 10) | (int32_t) fUniformIndex;
    }

    bool performsShading() const { return SkToBool(fFlags & BoundsFlags::kColor); }
    bool usesStencil()     const { return SkToBool(fFlags & BoundsFlags::kStencil); }

    bool isDepthOnly() const { return fFlags == BoundsFlags::kNone; }
    bool isSimpleShading() const { return fFlags == BoundsFlags::kColor; }

    SK_ALWAYS_INLINE bool isEqual(const LayerKey& other) const {
        // The pipeline defines the layer key's flags, so if the pipeline index is the same the
        // flags should be too and we skip checking them as part of isEqual.
        SkASSERT(fPipelineIndex != other.fPipelineIndex || fFlags == other.fFlags);
        return fPipelineIndex == other.fPipelineIndex &&
               fTextureIndex == other.fTextureIndex &&
               fUniformIndex == other.fUniformIndex;
    }
};

#define USE_BINDING_TREE
// #define USE_QUICK_LOOKUP
#define USE_INLINE_BINDINGS 8
// #define SEARCH_IN_TEST
#define BULK_BOUNDS 16
// #define BULK_BINDING_BOUNDS 16

#if defined(BULK_BOUNDS) || defined(BULK_BINDING_BOUNDS)
struct BoundsBlock {

    BoundsBlock() {
        fBounds = Rect::InfiniteInverted();
        fRects.fill(Rect::InfiniteInverted());
    }

    bool intersects(Rect::ComplementRect test) const {
        if (!fBounds.intersects(test)) {
            return false;
        }

        const int count = std::min(kN, fJoinIndex);
        for (int i = 0; i < count; i++) {
            // if (!(fFlags[i] & testMask)) { continue; }
            if (fRects[i].intersects(test)) {
                return true;
            }
        }
        return false;
    }

    void add(Rect rect) {
        fBounds.join(rect);

        // int join = fJoinIndex++;
        // if (join >= kN) {
        //     float area = std::numeric_limits<float>::infinity();
        //     for (int i = 0; i < kN; ++i) {
        //         Rect opt = fRects[i].makeJoin(rect);
        //         float optArea = opt.area();
        //         if (optArea < area) {
        //             join = i;
        //         }
        //     }
        // }

        int join = (fJoinIndex++) % kN;
        fRects[join].join(rect);
    }

private:
#if defined(BULK_BINDING_BOUNDS)
    static constexpr int kN = BULK_BINDING_BOUNDS;
#else
    static constexpr int kN = BULK_BOUNDS;
#endif

    Rect fBounds;
    std::array<Rect, kN> fRects;

    int fJoinIndex = 0;
};
#endif

/**
 * A Draw represents the combination of a DrawParams and a specific RenderStep from the
 * chosen Renderer. DrawListLayer ensures Draws for a multi-step Renderer are drawn in the
 * right order. A Draw holds the specific uniform data and pointers to live in a BindingList.
 */
struct Draw {
    Draw(const DrawParams* params, const UniformDataCache::Index uniformIndex)
            : fDrawParams(params), fUniformIndex(uniformIndex) {}

    Draw() = default;

    const DrawParams* fDrawParams;
    UniformDataCache::Index fUniformIndex;

    Draw* fNext = nullptr;
};

/**
 * BindingList represents a collection of Draws that share the same RenderStep and other binding
 * state (i.e. pipeline, textures, and optionally uniforms). When SSBOs are used for "uniform"
 * data, the draws in a BindingList can have different fUniformIndex values; otherwise their
 * fUniformIndex will match that of the BindingList's fKey.
 *
 * If the LayerKey's flags does not include kMustBeDisjoint, the Draws in a BindingList may not be
 * disjoint from each other. However, DrawListLayer ensures that this still results in the correct
 * painter's order rendering.
 */
struct BindingList {
    BindingList(const RenderStep* step, LayerKey key) : fStep(step), fKey(key) {}

#if defined(BULK_BINDING_BOUNDS)
    BoundsBlock fBounds;

#elif !defined(BULK_BOUNDS)
    Rect fBounds = Rect::InfiniteInverted();
    int fDrawCount = 0;
#endif


    const RenderStep* fStep;
    Draw* fHead = nullptr;
    Draw* fTail = nullptr;

    Draw fFirstDraw;

    const LayerKey fKey;

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(BindingList);

    SK_ALWAYS_INLINE bool isBetterMatch(const LayerKey& key,
                                        const BindingList* existingMatch) const {
        if (key.fPipelineIndex != fKey.fPipelineIndex) {
            // Any partial match must still share the same pipeline
            return false;
        } else if (!existingMatch) {
            return true; // Any pipeline match is better than no match
        }

        // Otherwise we need to rank based on similarities, preferring texture matches to
        // uniform matches.
        SkASSERT(existingMatch->fKey.fPipelineIndex == key.fPipelineIndex);
        const bool existingTextureMatch = existingMatch->fKey.fTextureIndex == key.fTextureIndex;
        const bool newTextureMatch = fKey.fTextureIndex == key.fTextureIndex;
        if (existingTextureMatch != newTextureMatch) {
            // If `newTextureMatch` is true, then the new BindingList is definitely the better
            // match (prioritizing textures over UBO changes). If it's false, then the old
            // match was better since it had a texture match.
            return newTextureMatch;
        } // else either both match on the texture, or neither match so equal preference.

        const bool existingUniformMatch = existingMatch->fKey.fUniformIndex == key.fUniformIndex;
        const bool newUniformMatch= fKey.fUniformIndex == key.fUniformIndex;
        if (existingUniformMatch != newUniformMatch) {
            // Like above, if `newUniformMatch` is true, it's the better match.
            return newUniformMatch;
        } // else either both match on the uniform, or neither match so equal preference.

        // // At this point, they are equivalent, so prefer the new list as it's deeper
        return true;
    }

#if defined(BULK_BINDING_BOUNDS)
    SK_ALWAYS_INLINE bool intersects(Rect::ComplementRect rect) const {
        return fBounds.intersects(rect);
    }
#elif !defined(BULK_BOUNDS)
    SK_ALWAYS_INLINE bool intersects(Rect::ComplementRect rect) const {
        if (!fBounds.intersects(rect)) {
            return false;
        }

        if (fDrawCount > 64) {
            return true;
        }

        for (const Draw* d = fHead; d != nullptr; d = d->fNext) {
            if (d->fDrawParams->drawBounds().intersects(rect)) {
                return true;
            }
        }
        return false;
    }
#endif

    SK_ALWAYS_INLINE void addDraw(SkArenaAlloc* alloc,
                                  DrawParams* drawParams,
                                  UniformDataCache::Index uniformIndex,
                                  bool backToFront) {
#if defined(BULK_BINDING_BOUNDS)
        fBounds.add(drawParams->drawBounds());
#elif !defined(BULK_BOUNDS)
        fBounds.join(drawParams->drawBounds());
        fDrawCount++;
#endif
        if (fHead) {
            // Singly-linked list for additional draws, created from the arena
            Draw* newDraw = alloc->make<Draw>(drawParams, uniformIndex);
            if (backToFront) {
                // The new draw goes to the end of the tail
                fTail->fNext = newDraw;
                fTail = newDraw;
            } else {
                // The new draw becomes the new head
                newDraw->fNext = fHead;
                fHead = newDraw;
            }
        } else {
            // The first draw uses inline storage and becomes the head
            fFirstDraw = {drawParams, uniformIndex};
            fHead = fTail = &fFirstDraw;
        }
    }
};

using BindingListStorage = std::array<uint8_t, sizeof(BindingList)>;

#if defined(USE_BINDING_TREE)
struct BindingNode {
    TextureDataCache::Index fTexture = TextureDataCache::kInvalidIndex;
    UniformDataCache::Index fUniform = UniformDataCache::kInvalidIndex;

    BindingList* fBindings = nullptr;

    BindingNode* fNext = nullptr;

    SK_ALWAYS_INLINE BindingList* search(const LayerKey& key, int* testDepth) const {
        (*testDepth)++;
        if (fTexture == key.fTextureIndex && fUniform == key.fUniformIndex) {
            // Found an exact match
            return fBindings;
        }

        BindingList* possibleMatch = fNext ? fNext->search(key, testDepth) : nullptr;
        if (fBindings && fBindings->isBetterMatch(key, possibleMatch)) {
            return fBindings;
        } else {
            return possibleMatch;
        }
    }

    SK_ALWAYS_INLINE void add(SkArenaAlloc* alloc, BindingList* bindings) {
        if (!fBindings) {
            // Store the binding in this node
            fTexture = bindings->fKey.fTextureIndex;
            fUniform = bindings->fKey.fUniformIndex;
            fBindings = bindings;
            return;
        }

        if (!fNext) {
            fNext = alloc->make<BindingNode>();
        }
        fNext->add(alloc, bindings);
    }

    SK_ALWAYS_INLINE bool remove(BindingList* bindings) {
        if (fBindings == bindings) {
            // Found the node to remove
            if (fNext) {
                *this = *fNext;
            } else {
                fBindings = nullptr; // unlock root node
                return true; // bubble up
            }
        } else if (fNext) {
            if (fNext->remove(bindings)) {
                fNext = nullptr;
            }
        }
        return false;
    }
};

struct PipelineNode {
    GraphicsPipelineCache::Index fPipeline = GraphicsPipelineCache::kInvalidIndex;
    PipelineNode* fNext = nullptr;

    std::array<BindingNode, 8> fChildren{};

    int index(const LayerKey& key) const {
        return (key.fTextureIndex ^ key.fUniformIndex) & 7;
    }

    SK_ALWAYS_INLINE BindingList* search(const LayerKey& key, int* testDepth) const {
        if (key.fPipelineIndex == fPipeline) {
            // Search within children
            int index = this->index(key);
            BindingList* possibleMatch = fChildren[index].search(key, testDepth);
            if (possibleMatch) {
                return possibleMatch; // exact or partial w/ possibility of texture/uniform match, we can't get better in other child nodes
            } else {
                for (int i = 0; i < 4; ++i) {
                    if (i == index) { continue; }
                    BindingList* secondary = fChildren[i].search(key, testDepth);
                    if (secondary && secondary->isBetterMatch(key, possibleMatch)) {
                        possibleMatch = secondary;
                    }
                }
                return possibleMatch;
            }
        } else if (fNext) {
            // Search for pipeline match
            return fNext->search(key, testDepth);
        } else {
            // No match
            return nullptr;
        }
    }

    SK_ALWAYS_INLINE bool remove(BindingList* binding) {
        const LayerKey& key = binding->fKey;
        if (key.fPipelineIndex == fPipeline) {
            // Remove from within children
            if (fChildren[this->index(key)].remove(binding)) {
                if (!fChildren[0].fBindings && !fChildren[1].fBindings && !fChildren[2].fBindings && !fChildren[3].fBindings) {
                    // This PipelineNode is empty, copy any next into itself
                    if (fNext) {
                        *this = *fNext; // now it's not empty anymore
                    } else {
                        fPipeline = GraphicsPipelineCache::kInvalidIndex; // unlock root node
                        return true; // tell preceding node to stop pointing to this one
                    }
                }
            }
        } else if (fNext) {
            if (fNext->remove(binding)) {
                fNext = nullptr;
            }
        }
        return false;
    }

    SK_ALWAYS_INLINE void add(SkArenaAlloc* alloc, BindingList* binding) {
        const LayerKey& key = binding->fKey;

        if (key.fPipelineIndex == fPipeline || fPipeline == GraphicsPipelineCache::kInvalidIndex) {
            // Place within children
            fPipeline = key.fPipelineIndex;
            fChildren[this->index(key)].add(alloc, binding);
            return;
        }

        if (!fNext) {
            // No more nodes to search, so create a new node in the chain
            fNext = alloc->make<PipelineNode>();
        }
        // Search for pipeline match and add there
        fNext->add(alloc, binding);
    }
};

struct BindingTree {
    std::array<PipelineNode, 8> fNodes{};

    int index(const LayerKey& key) const {
        return key.fPipelineIndex & 7;
    }

    SK_ALWAYS_INLINE BindingList* search(const LayerKey& key, int* testDepth) const {
        return fNodes[this->index(key)].search(key, testDepth);
    }

    SK_ALWAYS_INLINE void remove(BindingList* binding) {
        fNodes[this->index(binding->fKey)].remove(binding);
    }

    SK_ALWAYS_INLINE void add(SkArenaAlloc* alloc, BindingList* binding) {
        fNodes[this->index(binding->fKey)].add(alloc, binding);
    }
};

#endif


/**
 * Layer represents a collection of independent Draws that are organized by BindingLists. Within
 * a Layer, this allows draws to be ordered to minimize pipeline and state changes without impacting
 * painter's order visual correctness. Every draw stored in a Layer shares the same
 * CompressedPaintersOrder, which is a monotonically increasing sequence for each DrawList.
 *
 * Independence does not necessarily mean that all of the draws are disjoint from each other,
 * although that is frequently the case. The following caveats apply:
 *   1. Draws that share the same DrawParams (e.g. for a multi-step render) are assumed to overlap.
 *      The placement within a Layer is determined by the final shading Draw. The test flags used
 *      for search layers is the union of all Draws, so other than overlapping with its own steps
 *   2. Draws are only compared against other draws if the testMask has overlapping bits with their
 *      LayerKey's flags. In the case there are no shared bits, bounds intersections are irrelevant
 *      since the actual draw operations should be independent.
 *   3. Draws whoses flags contain kOverlapAllowed may overlap draws within their matched
 *      BindingList, but the preconditions for this ensure that the rasterization order on the GPU
 *      and hardware blending preserve painter's order.
 *   4. If a Layer is not the tail layer, its BindingLists are no longer forward-merge eligible
 *      (since that only pulls a list into a new layer). Since they are no longer forward-merge
 *      eligible, it is no longer critical to preserve disjointness between draws in one BindingList
 *      and those earlier than it. In this case, new Draws only need to be disjoint from
 *      BindingLists drawn after a match.
 *
 * A Layer keeps its single list of BindingLists organized to maintain the following properties:
 *   1. Non-shading BindingLists are ordered before every shading BindingList. This helps reduce the
 *      binding lists tested for a shading draw (can stop once a non-shading list is found). It also
 *      ensures depth-only clip draws are rendered before the shading draws that should be clipped.
 *   2. Shading BindingLists are ordered back-to-front if allowing overlaps within a BindingList or
 *      when it's not the tail Layer anymore.
 *   2. BindingLists that share the same pipeline are clustered together as best as possible so that
 *      moving between adjacent BindingLists is more likely to just be a buffer or texture bind
 *      than a more expensive pipeline bind.
 */


struct Layer {
    Layer(const CompressedPaintersOrder& order) : fOrder(order) {}

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Layer);

    const CompressedPaintersOrder fOrder;
    SkTInternalLList<BindingList> fBindings;

#if defined(BULK_BOUNDS)
    BoundsBlock fColorBounds;
    BoundsBlock fStencilBounds;
#endif

#if defined(USE_BINDING_TREE)
    BindingTree fTree;
#endif


#if defined(USE_QUICK_LOOKUP)
    std::array<BindingList*, 1> fQuickMatch{};
    int fNextBindingIdx = 0;
#endif

#if defined(USE_INLINE_BINDINGS)
    int fInlineBindingCount = 0;
    static constexpr int kInlineBindingSize = USE_INLINE_BINDINGS;
    std::array<BindingListStorage, kInlineBindingSize> fInlineBindings;
#endif

    struct Stats {
        int fTotalDrawCount = 0;
        int fBindingListCount = 0;
        int fMaxDrawsPerBindingList = 0;
        int fMaxTestDepth = 0;
        float fAvgDrawsPerBindingList = 0.f;
        float fAvgTestDepth = 0.f;

        int fStencilCount = 0;
    };

    Stats collectStats() const {
        Stats stats;

        for (const BindingList* list : fBindings) {
            stats.fBindingListCount++;

            int drawCount = 0;

            for (const Draw* d = list->fHead; d; d = d->fNext) {
                drawCount++;
            }
            stats.fTotalDrawCount += drawCount;
            if (list->fKey.fFlags & BoundsFlags::kStencil) {
                stats.fStencilCount += drawCount;
            }
            stats.fMaxDrawsPerBindingList = std::max(drawCount, stats.fMaxDrawsPerBindingList);
            // SkDebugf("binding change %d\n", drawCount);
        }
        stats.fMaxTestDepth = fMaxTestDepth;
        stats.fAvgDrawsPerBindingList = stats.fTotalDrawCount / (float) stats.fBindingListCount;
        stats.fAvgTestDepth = fTestDepth / (float) fTestCount;

        return stats;
    }

    // Performs no bounds checks, so can only be used when checks have already confirmed the Layer
    // is valid for adding a new draw into. This searches backwards from `startList` (exclusive) or
    // the tail BindingList if null.
    //
    // Return a BindingList matching `key` if one exists in the layer (or exists in the layer
    // at or before `startList`). If an exact match is not found, it attempts to return a
    // BindingList that has the same pipeline index.
    SK_ALWAYS_INLINE BindingList* searchBinding(const LayerKey& key,
                                                BindingList* startList=nullptr,
                                                bool forForwardMerge=false) {
#if defined(USE_QUICK_LOOKUP)
        if (!startList) {
            for (int i = 0; i < 1; ++i) {
                if (fQuickMatch[i] && fQuickMatch[i]->fKey.isEqual(key)) {
                    return fQuickMatch[i];
                }
            }
        }
#endif
#if defined(USE_BINDING_TREE)
        if (!startList) {
            int oldDepth = fTestDepth;
            BindingList* r = fTree.search(key, &fTestDepth);
            fMaxTestDepth = std::max(fTestDepth - oldDepth, fMaxTestDepth);
            return r;
        }
#endif

        // `startList` is exclusive, so if it's non-null the loop starts with fPrev.
        BindingList* pipelineMatch = nullptr;
        if (!forForwardMerge) {
            fTestCount++;
        }
        int oldDepth = fTestDepth;
        for (BindingList* list = startList ? startList->fPrev : fBindings.tail();
                list != nullptr; list = list->fPrev) {
            // if (forForwardMerge && list->fBlockForwardMerges) {
            //     break;
            // }
            fTestDepth++;

            if (list->fKey.isEqual(key)) {
                fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                return list;
            } else if (list->isBetterMatch(key, pipelineMatch)) {
                // Save pipeline while continuing to search for an exact match
                SkASSERT(list->fKey.fFlags == key.fFlags);
                pipelineMatch = list;
            } else if (key.performsShading() && !list->fKey.performsShading()) {
                // The BindingLists are split in two sections: a latter half with color (that is
                // check first because we start at the tail) and then anything else that is
                // non-shading (possibly with stencil). The depth-only and the stencil-only get
                // intermingled so we can't early out for those, but if `key` has color and `list`
                // does not, then a match is no longer possible.
                break;
            }
        }

        fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);

        // Even though an exact match wasn't found, any non-null pipelineMatch can be used to place
        // a new binding list, which helps shift from a pipeline switch to just a dynamic state.
        return pipelineMatch;
    }

    int fTestCount = 0;
    int fTestDepth = 0;
    int fMaxTestDepth = 0;

    SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, BindingList*> test(
            const Rect::ComplementRect drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {
#if !defined(BULK_BOUNDS)
        static constexpr SkEnumBitMask<BoundsTestResult> kFull = BoundsTestResult::kAllowedInLayer |
                                                 BoundsTestResult::kAllowedBeforeLayer;
        SkEnumBitMask<BoundsTestResult> result = kFull;
        BindingList* match = nullptr;

        for (BindingList* list = fBindings.tail(); list; list = list->fPrev) {
            auto aspect = testMask & list->fKey.fFlags;
            if (aspect && list->intersects(drawBounds)) {
                match = nullptr;
                result &= ~BoundsTestResult::kAllowedInLayer;
                if (aspect & BoundsFlags::kColor) {
                    result &= ~BoundsTestResult::kAllowedBeforeLayer;
                }

                if (!result) {
                    break;
                }
            } else if (result == kFull && list->isBetterMatch(key, match)) {
                match = list;
            }
        }

        return {result, match};
#endif

#if defined(BULK_BOUNDS)
        // FIXME play around with matching inside this function
        BindingList* match = nullptr;
        SkEnumBitMask<BoundsTestResult> result = BoundsTestResult::kBlocked;
        if (!(testMask & BoundsFlags::kColor) || !fColorBounds.intersects(drawBounds)) {
            result |= BoundsTestResult::kAllowedBeforeLayer;
            if (!(testMask & BoundsFlags::kStencil) || !fStencilBounds.intersects(drawBounds)) {
                result |= BoundsTestResult::kAllowedInLayer;
#if defined(SEARCH_IN_TEST)
                match = this->searchBinding(key);
#endif
            }
        } else if (!(testMask & BoundsFlags::kStencil)) {
            // We don't care about stencil, we could go into this layer *if* we push the binding
            // list in such a way that rasterization order saves our bacon.
#if defined(SEARCH_IN_TEST)
            if (fNext) {
                // This layer is not forward mergeable anymore, so as long as we can append, we're
                // fine. So we could a) return allowed-in-layer if we didn't search for a match later
                // Or we could require that it match the last binding list, or we need to iterate
                // over the binding list and check for detailed overlaps again.
            } else {
                // This layer is forward eligible, so we could do a forward merge.
                match = this->searchBinding(key, nullptr, true);
                if (match && !match->fPrev && !match->fNext) {
                    // This is the only binding list in the layer, so don't pull it foward
                    // FIXME only works for exact matches since compat match turns into a
                    // order inversion in the binding list.
                    result |= BoundsTestResult::kAllowedInLayer;
                }
            }
#endif
        }

        return {result, match};
#endif // BULK_BOUNDS
    }

    SK_ALWAYS_INLINE BindingList* addNewBinding(SkArenaAllocWithReset* alloc,
                                                BindingList* insertBefore,
                                                const LayerKey& key,
                                                const RenderStep* step,
                                                bool firstStep) {
        SkASSERT(!insertBefore || fBindings.isInList(insertBefore));

        BindingList* list;
#if defined(USE_INLINE_BINDINGS)
        if (fInlineBindingCount < kInlineBindingSize) {
            void* ptr = static_cast<void*>(&fInlineBindings[fInlineBindingCount++]);
            list = new (ptr) BindingList(step, key);
        } else
#endif
        {
            list = alloc->make<BindingList>(step, key);
        }

#if defined(USE_BINDING_TREE)
    if (firstStep) {
        fTree.add(alloc, list);
    }
#endif

        // We need to insert the new list in the right place to keep fBindings organized with all
        // non-shading layers before shading layers, while also ensuring that the new `list` comes
        // before `insertBefore` (when non-null).
        if (insertBefore && key.performsShading() == insertBefore->fKey.performsShading()) {
            // Since both keys' shading state matches, putting the new list right in front of
            // `insertBefore` will not split the two sections (regardless of whether it was in the
            // shading or non-shading section).
            fBindings.addBefore(list, insertBefore);
        } else if (key.performsShading()) {
            // Since a new shading binding can only be inserted before other shading bindings,
            // the only way to get to this branch is to not have an insertBefore target. As such,
            // the simplest way to maintain keeping shading bindings in the latter half is to add
            // to the tail.
            SkASSERT(!insertBefore);
            fBindings.addToTail(list);
        } else {
            // A non-shading draw can have an `insertBefore` target that is a shading binding (e.g.
            // where the final shading step was inserted in the layer). In that case, addBefore()
            // would possibly split the shading bindings section of `fBindings`. Adding it to the
            // head of the bindings' list preserves the guarantee that all non-shading bindings are
            // at the start and satisfies adding it before the `insertBefore` (if it were non-null).
            SkASSERT(!key.performsShading());
            SkASSERT(!insertBefore || insertBefore->fKey.performsShading());
            fBindings.addToHead(list);
        }

        return list;
    }

    SK_ALWAYS_INLINE void transfer(BindingList* binding, Layer* newLayer) {
        fBindings.remove(binding);
        newLayer->fBindings.addToHead(binding);
#if BOUNDS_MODE == LAYER_BLOCK
        // fTree.remove(binding);
        // newLayer->fTree.add(nullptr, binding);
#endif
    }

    SK_ALWAYS_INLINE void updateForDraw(BindingList* bindingList, Rect bounds, LayerKey key) {
#if defined(BULK_BOUNDS)
        if (key.fFlags & BoundsFlags::kColor) {
            fColorBounds.add(bounds);
        }
        if (key.fFlags & BoundsFlags::kStencil) {
            fStencilBounds.add(bounds);
        }
#endif

#if defined(USE_QUICK_LOOKUP)
        fQuickMatch[fNextBindingIdx] = bindingList;
        fNextBindingIdx = (fNextBindingIdx + 1) & 0;
#endif
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
