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

// template <typename T, int BlockSize>
// class ArenaBlockList {
// public:
//     ArenaBlockList() : fHead(), fTail(&fHead) {}

//     T& push_back(SkArenaAlloc* arena, const T& value) {
//         if (fTail->fCount == BlockSize) {
//             Block* newBlock = arena->make<Block>();
//             fTail->fNext = newBlock;
//             newBlock->fPrev = fTail;
//             fTail = newBlock;
//         }
//         T* nextItem = new (&fTail->fItems[sizeof(T)*fTail->fCount++]) T(value);
//         return *nextItem;
//     }

// private:
//     struct Block {
//         Block() = default;

//         const T& operator[](int index) const { return *reinterpret_cast<const T*>(&fItems[index * sizeof(T)]); }
//         T& operator[](int index) { return *reinterpret_cast<T*>(&fItems[index * sizeof(T)]); }

//         alignas(T) std::array<uint8_t, sizeof(T)*BlockSize> fItems;
//         Block* fPrev = nullptr;
//         Block* fNext = nullptr;
//         int fCount = 0;
//     };
//     static_assert(std::is_trivially_destructible<Block>::value);

//     Block fHead;
//     Block* fTail;

// public:
//     template <bool Forward>
//     struct Iter {
//         struct Item {
//             Item() : fBlock(nullptr), fIndex(
//             explicit operator bool() const { return SkToBool(fBlock); }

//             bool operator!=(const Item& other) const {
//                 return other.fBlock != fBlock || (fBlock && other.fIndex != fIndex);
//             }

//             const T& operator*() const {
//                 SkASSERT(fBlock && fIndex < fBlock->fCount);
//                 return (*fBlock)[fIndex];
//             }

//             const T* operator->() const {
//                 SkASSERT(fBlock && fIndex < fBlock->fCount);
//                 return &(*fBlock)[fIndex];
//             }

//             Item& operator++() {
//                 if constexpr (Forward) {
//                     fIndex++;
//                     if (fIndex >= fBlock->fCount) {
//                         fBlock = fBlock->fNext;
//                         fIndex = 0;
//                     }
//                 } else {
//                     fIndex--;
//                     if (fIndex < 0) {
//                         fBlock = fBlock->fPrev;
//                         fIndex = fBlock ? fBlock->fCount - 1 : 0;
//                     }
//                 }

//                 return *this;
//             }

//         private:
//             friend struct Iter;

//             Item(const Block* block, int index) : fBlock(block), fIndex(index) {}

//             const Block* fBlock;
//             int fIndex;
//         };

//         Item begin() const { return Item(fStart, Forward ? 0 : fStart->fCount - 1); }
//         Item end() const { return Item(nullptr, 0); }

//     private:
//         friend class ArenaBlockList;

//         Iter(const Block* start) : fStart(start) {}
//         const Block* fStart;
//     };

//     Iter<true> items() const { return Iter<true>(&fHead); }
//     Iter<false> ritems() const { return Iter<false>(fTail); }
// };


#define BOUNDS_FROM_DRAWS 1
#define BINDING_LIST_BLOCK 2
#define LAYER_BLOCK 3
#define LAYER_BLOCK2 4

#define BOUNDS_MODE LAYER_BLOCK


#define INTERLEAVED 1

struct BoundsBlock {

    BoundsBlock() {
        fBounds = Rect::InfiniteInverted();

#if INTERLEAVED
        fRects.fill(Rect::InfiniteInverted());
#else
        constexpr static float infinity = std::numeric_limits<float>::infinity();
        fLefts.fill(infinity);
        fTops.fill(infinity);
        fNegRights.fill(infinity); // effectively -infinity
        fNegBots.fill(infinity); // ""
#endif
    }

    bool intersects(Rect::ComplementRect test) const {
        if (!fBounds.intersects(test)) {
            return false;
        }

        const int count = std::min(kN, fJoinIndex);

#if INTERLEAVED
        for (int i = 0; i < count; i++) {
            if (fRects[i].intersects(test)) {
                return true;
            }
        }
#else
        auto comp = test.fVals;
        for (int i = 0; i < count; i += 4) {
            auto l = skvx::float4::Load(fLefts.data() + i);
            auto t = skvx::float4::Load(fTops.data() + i);
            auto nr = skvx::float4::Load(fNegRights.data() + i);
            auto nb = skvx::float4::Load(fNegBots.data() + i);

            if (any((l < comp[0]) &
                    (t < comp[1]) &
                    (nr < comp[2]) &
                    (nb < comp[3]))) {
                return true;
            }
        }
#endif
        return false;

    }

    void add(Rect rect) {
        int join = (fJoinIndex++) % kN;

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

#if INTERLEAVED
        fRects[join].join(rect);
#else
        auto vals = rect.vals();
        fLefts[join] = std::min(fLefts[join], vals[0]);
        fTops[join] = std::min(fTops[join], vals[1]);
        fNegRights[join] = std::min(fNegRights[join], vals[2]);
        fNegBots[join] = std::min(fNegBots[join], vals[3]);
#endif
    }

private:
    static constexpr int kN = 16;

    Rect fBounds;

#if INTERLEAVED
    alignas(Rect) std::array<Rect, kN> fRects;
#else
    alignas(Rect) std::array<float, kN> fLefts;
    alignas(Rect) std::array<float, kN> fTops;
    alignas(Rect) std::array<float, kN> fNegRights;
    alignas(Rect) std::array<float, kN> fNegBots;
#endif

    int fJoinIndex = 0;
};

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

#if BOUNDS_MODE != BINDING_LIST_BLOCK
    // Rect fBounds = Rect::InfiniteInverted();
#endif

    const RenderStep* fStep;
    Draw* fHead = nullptr;
    Draw* fTail = nullptr;

    Draw fFirstDraw;

    const LayerKey fKey;


    // ArenaBlockList<Draw, 16> fDraws;
#if BOUNDS_MODE == BINDING_LIST_BLOCK
    BoundsBlock fBounds;
#endif



    // If this is true, it means that a draw was added to it without having been tested against
    // the earlier BindingLists so they are not eligible for being moved forward to a new layer.
    // bool fBlockForwardMerges = false;

    // int fDrawCount = 0; // SkTInternalLList doesn't maintain a count for us :/

    // int fInlineDrawCount = 0;

    // static constexpr int kInlineDrawSize = 4;
    // std::array<DrawStorage, kInlineDrawSize> fInlineDraws;

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

    SK_ALWAYS_INLINE bool intersects(const Rect::ComplementRect drawBounds) const {
#if BOUNDS_MODE == BINDING_LIST_BLOCK
        return fBounds.intersects(drawBounds);
#else
        // if (!fBounds.intersects(drawBounds)) {
        //     return false;
        // }

        // When stencil is involved (or depth-only draws), a larger limit is useful because it
        // creates a shallower layer list. This in turn makes it more likely to find BindingList
        // matches. In particular, since stencils have to be disjoint from everything and depth-only
        // clip draws prevent further layer searches, checking more bounding boxes leads to a net
        // improvement in complex scenes.
        // const int limit = 64; //fKey.isSimpleShading() ? 512 : 512;
        // all the way down to 32 didn't seem to impact batching at layers=64, but limit=16 finally started to
        // if (fDrawCount > limit) {
        //     return true;
        // }
        for (const Draw* d = fHead; d; d = d->fNext) {
        // for (const auto& d : fDraws.items()) {
            if (d->fDrawParams->drawBounds().intersects(drawBounds)) {
                return true;
            }
        }
        return false;
#endif
    }

    SK_ALWAYS_INLINE void addDraw(SkArenaAlloc* alloc, DrawParams* drawParams, UniformDataCache::Index uniformIndex, bool backToFront) {
#if BOUNDS_MODE == BINDING_LIST_BLOCK
    fBounds.add(drawParams->drawBounds());
#else
    //  fBounds.join(drawParams->drawBounds());
#endif

        // int inlineIdx = fDrawCount++;

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

        // Draw* draw;
        // // if (inlineIdx < kInlineDrawSize) {
        // //     void* ptr = static_cast<void*>(&fInlineDraws[inlineIdx]);
        // //     draw = new (ptr) Draw{drawParams, uniformIndex};
        // // } else
        // {
        //     // layer->makeNewDraw(alloc, drawParams, uniformIndex);
        //     draw = alloc->make<Draw>(drawParams, uniformIndex);
        // }

        // if (backToFront) {
        //     fDraws.addToTail(draw);
        // } else {
        //     fDraws.addToHead(draw);
        // }
    }
};

using BindingListStorage = std::array<uint8_t, sizeof(BindingList)>;

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
        // if (fTexture == key.fTextureIndex || fUniform == )

        // if (fNext) {
        //     // Continue searching in other nodes
        //     return fNext->search(key, testDepth);
        // } else {
        //     // Nothing left to search
        //     return nullptr;
        // }
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

    // SK_ALWAYS_INLINE void addDraw(SkArenaAlloc* alloc, const DrawParams* draw, UniformDataCache::Index uniformIndex, const LayerKey& key, bool backToFront) {
    //     if (fHead) {
    //         // The node has been locked into a specific texture and uniform index, so see if they match the key
    //         if (fTexture != key.fTextureIndex || fUniform != key.fUniformIndex) {
    //             // Not a match, so move to the next node in the list
    //             if (!fNext) {
    //                 fNext = alloc->make<BindingNode>();
    //             }
    //             fNext->addDraw(alloc, draw, uniformIndex, key, backToFront);
    //             return;
    //         }

    //         // Have a match, so add the draw to this node's list
    //         Draw* newDraw = alloc->make<Draw>(draw, uniformIndex);
    //         if (backToFront) {
    //             // The new draw goes to the end of the tail
    //             fTail->fNext = newDraw;
    //             fTail = newDraw;
    //         } else {
    //             // The new draw becomes the new head
    //             newDraw->fNext = fHead;
    //             fHead = newDraw;
    //         }
    //     } else {
    //         // First draw, so lock in the texture and uniform index as well as using inline store
    //         fTexture = key.fTextureIndex;
    //         fUniform = key.fUniformIndex;
    //         fFirstDraw = {draw, uniformIndex};
    //         fHead = fTail = &fFirstDraw;
    //     }
    // }
};

struct PipelineNode {
    GraphicsPipelineCache::Index fPipeline = GraphicsPipelineCache::kInvalidIndex;
    PipelineNode* fNext = nullptr;

    std::array<BindingNode, 4> fChildren{};

    // const RenderStep* fStep;
    // BoundsFlags fLayerFlags;

    int index(const LayerKey& key) const {
        return (key.fTextureIndex ^ key.fUniformIndex) & 3;
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
    std::array<PipelineNode, 4> fNodes{};

    int index(const LayerKey& key) const {
        return key.fPipelineIndex & 3;
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

#if BOUNDS_MODE == LAYER_BLOCK
    BoundsBlock fColorBounds;
    BoundsBlock fStencilBounds;
#elif BOUNDS_MODE == LAYER_BLOCK2
    BoundsBlock fBounds;
#endif

    // This isn't any different than just looking through the back N binding lists with a stop.
    // Except that the memory is right here.
    // We could have it shuffle around based on last N binding lists with a draw added to it?
    // std::array<int32_t, 4> fBindingHash;

    // int fNextBindingHashIdx = 0;

#if BOUNDS_MODE == LAYER_BLOCK
    // BindingTree fTree;

    std::array<BindingList*, 4> fQuickMatch{};
    int fNextBindingIdx = 0;

#endif

    int fInlineBindingCount = 0;
    // int fInlineDrawCount = 0;




    static constexpr int kInlineBindingSize = 16;
    std::array<BindingListStorage, kInlineBindingSize> fInlineBindings;

    // static constexpr int kInlineDrawSize = 64;
    // std::array<DrawStorage, kInlineDrawSize> fInlineDraws;

    BindingList* makeNewBinding(SkArenaAlloc* alloc, const RenderStep* step, const LayerKey& key) {
        if (fInlineBindingCount < kInlineBindingSize) {
            void* ptr = static_cast<void*>(&fInlineBindings[fInlineBindingCount++]);
            return new (ptr) BindingList(step, key);
        } else
        {
            return alloc->make<BindingList>(step, key);
        }
    }

    // Draw* makeNewDraw(SkArenaAlloc* alloc, const DrawParams* drawParams, UniformDataCache::Index uniformIndex) {
    //     // if (fInlineDrawCount < kInlineDrawSize) {
    //     //     void* ptr = static_cast<void*>(&fInlineDraws[fInlineDrawCount++]);
    //     //     return new (ptr) Draw{drawParams, uniformIndex};
    //     // } else
    //     {
    //         return alloc->make<Draw>(drawParams, uniformIndex);
    //     }
    // }

    // ArenaBlockList<BindingList, 16> fBindings;


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
#if BOUNDS_MODE == LAYER_BLOCK
        // if (!startList) {
        //     int oldDepth = fTestDepth;
        //     BindingList* r = fTree.search(key, &fTestDepth);
        //     fMaxTestDepth = std::max(fTestDepth - oldDepth, fMaxTestDepth);
        //     return r;
        // }

        if (!startList) {
            for (int i = 0; i < 4; ++i) {
                if (fQuickMatch[i] && fQuickMatch[i]->fKey.isEqual(key)) {
                    return fQuickMatch[i];
                }
            }
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

#if BOUNDS_MODE == LAYER_BLOCK
    SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, BindingList*> test(
            const Rect::ComplementRect drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {

        bool allowedBeforeLayer = !(testMask & BoundsFlags::kColor) || !fColorBounds.intersects(drawBounds);
        bool allowedInLayer = allowedBeforeLayer && (!(testMask & BoundsFlags::kStencil) || !fStencilBounds.intersects(drawBounds));

        BindingList* match = nullptr;
        if (allowedInLayer) {
            // match = this->searchBinding(key);
            // keep it null so we just go as deep as possible???
            // if (this->mightHaveMatch(key)) {
                // match = this->searchBinding(key);
            // }
        } else if (!fNext && key.isSimpleShading()) {
            // FIXME need a way to remove binding lists from the tree for forward merges
            // match = this->searchBinding(key, nullptr, true);
            // return this->testDetailed(drawBounds, key, testMask);
        }

        return {(allowedBeforeLayer ? BoundsTestResult::kAllowedBeforeLayer : BoundsTestResult::kBlocked) |
                (allowedInLayer ? BoundsTestResult::kAllowedInLayer : BoundsTestResult::kBlocked),
                match};
    }
#endif

#if BOUNDS_MODE == LAYER_BLOCK2
    SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, BindingList*> test(
            const Rect::ComplementRect drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {

        if (!fBounds.intersects(drawBounds)) {
            BindingList* match = this->searchBinding(key);
            return {BoundsTestResult::kAllowedBeforeLayer | BoundsTestResult::kAllowedInLayer, match};
        } else {
            // return this->testDetailed(drawBounds, key, testMask);
            BindingList* match = !fNext && key.isSimpleShading() ? this->searchBinding(key, nullptr, true) : nullptr;
            return {BoundsTestResult::kBlocked, match};
        }
    }
#endif

#if BOUNDS_MODE == LAYER_BLOCK2 || BOUNDS_MODE ==LAYER_BLOCK /* unused */
    SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, BindingList*> testDetailed(
            const Rect::ComplementRect drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {
#else
    // Test the draw with the given bounds and LayerKey against the draws already collected in
    // this Layer, limiting checks to those that overlap with `testMask`. Returns whether or not
    // the draw is allowed in the layer, allowed before the layer, or must be in a later layer.
    // Any BindingList that the draw can be appended to directly, or pulled forward with the draw
    // is also returned.
    SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, BindingList*> test(
            const Rect::ComplementRect drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {
#endif
        // If the test mask is kNone, the caller should just use searchBinding() directly since it
        // won't overlap with anything anyways. Or (rare) it's some opaque multistep draw that
        // can't overlap itself so is going through the regular `test` process to find a layer.
        SkASSERT(testMask != BoundsFlags::kNone ||
                 SkToBool(key.fFlags & BoundsFlags::kMustBeDisjoint));
        // Overlaps being allowed or not is a property of the BindingList and shouldn't be put in
        // the test mask (otherwise it'd trigger testing against every BindingList that needed to be
        // internally disjoint, not just a matching list).
        SkASSERT(!SkToBool(testMask & BoundsFlags::kMustBeDisjoint));

        // Forward merging attempts to pull an earlier, compatible draw out of the current layer and
        // push it into a newly created layer to improve pipeline/texture batching.
        //
        // 1. Draw Type Restrictions (Single Renderstep & No Depth-Only):
        //    Forward merging is strictly limited to single-renderstep shading draws. We explicitly
        //    forbid depth-only draws, and the single-step requirement inherently excludes stencil
        //    draws. If we allowed multi-step renderers to forward merge, we would risk pulling a
        //    parent renderstep forward and over its already-inserted child.
        //
        // 2. Directional & Spatial Validity:
        //    The new draw searches backwards, tail to head, so during the search, any binding
        //    matches found *before* a layer execute *after* that draw during rendering. Because
        //    existing shading draws (has kColor) in a layer are guaranteed to be non-intersecting
        //    with other kColor BindingLists, it is visually safe to extract this match and defer
        //    its execution to the new layer without violating the Painter's Algorithm.
        //
        // 3. The Tail-Only Restriction:
        //    We strictly limit forward merging to the *tail* of the layer list. If we allowed
        //    forward merging from a middle layer, we would be forced to insert the newly generated
        //    target layer into the middle of the list. This would break the structural invariant
        //    that `Layer::fOrder` strictly increases with the physical list order; this invariant
        //    necessary to ensure that a draw is inserted after *ALL* depth-only clip draws that
        //    affect it.
        bool layerIsForwardMergeEligible = !fNext && key.isSimpleShading();
        const bool overlapInMatchesAllowed = key.isSimpleShading() || key.isDepthOnly();

        // Always iterate backwards from the tail, we do this because most draws (including depth-
        // only clip draws) must maintain painter's order so we can early out if they overlap with
        // a more recent draw. In the event that there isn't any color dependency, we're just
        // searching for a disjoint binding match and then whether or not to start from the front or
        // the back is arbitrary
        fTestCount++;
        int oldDepth = fTestDepth;
        BindingList* match = nullptr; // remember a good insertion point as bindings are tested
        for (BindingList* list = fBindings.tail(); list != nullptr; list = list->fPrev) {
            fTestDepth++;
            const bool exactMatch = list->fKey.isEqual(key);
            // What aspects interfere between the new draw and this binding list in the layer
            auto aspectOverlap = (list->fKey.fFlags & testMask) |
                     (exactMatch ? BoundsFlags::kMustBeDisjoint : BoundsFlags::kNone);

            if ((aspectOverlap && list->intersects(drawBounds)) || (fTestDepth - oldDepth) > 1024) {
                // It is important for complex scene performance that we restrict the exactMatch
                // early-out to be after doing intersection testing
                if (exactMatch && overlapInMatchesAllowed) {
                    if (layerIsForwardMergeEligible &&
                        list->fPrev && list->fPrev->fKey.performsShading()) {
                        // The layer is forward-merge compatible and since there are prior
                        // shading bindings, the draw can't just be appended without risking a
                        // painter's order inversion for the later binding on a future draw.
                        // Returning `list` will pull it into a new layer instead.
                        SkASSERT(!key.isDepthOnly());
                        fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                        return {BoundsTestResult::kBlocked, list};
                    } else {
                        // else the layer isn't forward-merge compatible so it doesn't matter if
                        // we have any overlaps with earlier bindings, but since we have an match
                        // to append to, let's exit early then (assume color aspect overlap to
                        // remove kAllowedBeforeLayer).
                        fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                        return {BoundsTestResult::kAllowedInLayer, list};
                    }
                }

                if (SkToBool(aspectOverlap & (BoundsFlags::kStencil |
                                              BoundsFlags::kMustBeDisjoint))) {
                    // The draw can't be in this layer because either
                    // - a stencil overlap must be fully disjoint in the layer for all draws
                    // - or this is an exact match with overlap not allowed
                    //
                    // We can also always stop searching because if we depend on kColor, our
                    // result would be kBlocked; and if we don't depend on kColor then no
                    // further test will restrict kAllowedBeforeLayer from the result.
                    fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                    return {aspectOverlap & BoundsFlags::kColor
                                    ? BoundsTestResult::kBlocked
                                    : BoundsTestResult::kAllowedBeforeLayer,
                            /*match=*/nullptr};
                }

                // At this point, it's just a color overlap, so the draw cannot go before this
                // layer. We try to put it in the layer or in the new layer with a binding match
                // to improve batching.
                SkASSERT(aspectOverlap == BoundsFlags::kColor);
                if (!match) {
                    // If we haven't found a good insertion point before this intersection, the
                    // draw must go in a new layer (aspectOverlap is kColor), however, if it's
                    // forward-merge eligible we can search for a compatible match to pull
                    // forward with it.
                    if (key.isSimpleShading() && layerIsForwardMergeEligible) {
                        match = this->searchBinding(key, list, /*forForwardMerge=*/true);
                    }
                    fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                    return {BoundsTestResult::kBlocked, match};
                } else if (!key.usesStencil()) {
                    // We have a good insertion point in the layer that will be drawn after this
                    // conflicting BindingList, so put the new draw there. Because we haven't fully
                    // checked all the previous BindingLists, we need to block forward merges as the
                    // new draw could now be overlapping with early lists in the layer.
                    // list->fBlockForwardMerges = true;
                    fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                    return {BoundsTestResult::kAllowedInLayer, match};
                } else {
                    // This comes up in a rare case where the current draw is stencil+shading and
                    // we'd found a previous match, but had to continue searching to ensure there is
                    // no stencil overlap. If we got here, aspectOverlap didn't have stencil so
                    // it's just overlapping with a regular shading draw. We *could* continue
                    // going through the BindingLists to confirm no stencil overlap and then
                    // return {kAllowedInLayer, match} at the end of the loop, but we'd have to
                    // track the result modifications over time and prevent updates to `match`.
                    // Pushing it to a new layer seems to work well in practice and let's the
                    // outer if (intersect) block always return early.
                    fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                    return {BoundsTestResult::kBlocked, /*match=*/nullptr};
                }
            }

            // layerIsForwardMergeEligible &= !list->fBlockForwardMerges;

            // NOTE: It's possible for the same key to be in a layer multiple times due to
            // some of the early-out rules. If we get here, the draw hasn't overlapped any later
            // drawn BindingList, so update the match to be the best, earliest match found so far.
            if (exactMatch || list->isBetterMatch(key, match)) {
                match = list;
            }

            if ((key.performsShading() && !key.usesStencil()) && !list->fKey.performsShading()) {
                // Since we guarantee that all non-shading draws are ordered *before* shading ones,
                // the remaining BindingLists won't perform shading and the new draw doesn't use
                // the stencil, so all remaining lists will have `aspectOverlap == kNone`.
                break;
            }
        }

        fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
        return {BoundsTestResult::kAllowedBeforeLayer | BoundsTestResult::kAllowedInLayer, match};
    }

    // bool mightHaveMatch(const LayerKey& key) const {
    //     int32_t h = key.hash();
    //     return fBindingHash[0] == h || fBindingHash[1] == h || fBindingHash[2] == h || fBindingHash[3] == h;
    // }

    SK_ALWAYS_INLINE BindingList* addNewBinding(SkArenaAllocWithReset* alloc,
                                                BindingList* insertBefore,
                                                const LayerKey& key,
                                                const RenderStep* step,
                                                bool firstStep) {
        SkASSERT(!insertBefore || fBindings.isInList(insertBefore));

        BindingList* list = this->makeNewBinding(alloc, step, key); //alloc->make<BindingList>(step, key);

        // fBindingHash[fNextBindingHashIdx] = key.hash();
        // fNextBindingHashIdx = (fNextBindingHashIdx + 1) & 3;

#if BOUNDS_MODE == LAYER_BLOCK
    // if (firstStep) {
        // fTree.add(alloc, list);
    // }
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
#if BOUNDS_MODE == LAYER_BLOCK
        if (key.fFlags & BoundsFlags::kColor) {
            fColorBounds.add(bounds);
        }
        if (key.fFlags & BoundsFlags::kStencil) {
            fStencilBounds.add(bounds);
        }

        fQuickMatch[fNextBindingIdx] = bindingList;
        fNextBindingIdx = (fNextBindingIdx + 1) & 3;

#elif BOUNDS_MODE == LAYER_BLOCK2
        fBounds.add(bounds);
#endif
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
