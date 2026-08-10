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

// #define USE_BINDING_TREE
// #define USE_BINDING_TREE2
// #define USE_QUICK_LOOKUP
// #define USE_INLINE_BINDINGS 8
#define SEARCH_IN_TEST
#define BULK_BOUNDS 16
// #define BULK_BINDING_BOUNDS 16
// #define USE_TREE2_SKIPLIST

// FIXME one more shot, but if I' doing bulk bounds, then the layers and every draw within the
// layer are disjoint over the aspects that matter to them, so there's no need for the complex
// binding list insertion logic. That means I could create a binding tree that actually stored the
// draw details inside of the tree directly instead of just using it for a search accelerator.
//
// So that seems worth pursuing, because that might give us that extra perf w/o just adding a lot
// of complexity. Need to make clipping exclusive though

// Other things to note/try: keep one base layer that's not allocated in the arena for opaque/unclipped things?
// Could we also find a way to organize the binding tree such that back-to-front pipelines are
// stored first?

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

        // int join = fJoinIndex++;//fJoinIndex++;
        // if (join >= kN) {
        //     float area = std::numeric_limits<float>::infinity();
        //     for (int i = 0; i < kN; ++i) {
        //         Rect opt = fRects[i].makeJoin(rect);
        //         float optArea = opt.area();
        //         if (optArea < area) {
        //             join = i;
        //         }
        //     }
        //     fRects[join].join(rect);
        // } else {
        //     fRects[join] = rect;
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
#if !defined(USE_BINDING_TREE2)
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

#if defined(USE_QUICK_LOOKUP)
    BindingList* fSkipNext = nullptr;
#endif

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
#endif

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


#if defined(USE_BINDING_TREE2)

// FIXME how to cluster non-exact matches by Texture index? Maybe we make a TextureNode and a
// UniformNode... alternatively just return partial match from find() and use that to insert
// exact BindingNode in PipelineNode::add()
struct BindingNode {
    BindingNode() = default;
    BindingNode(TextureDataCache::Index texture, UniformDataCache::Index uniform)
            : fTexture(texture)
            , fUniform(uniform)
            , fHead(nullptr)
            , fTail(nullptr)
            , fNext(nullptr)
#if defined(USE_TREE2_SKIPLIST)
            , fSkipNext(nullptr)
#endif
            {}

    TextureDataCache::Index fTexture;
    UniformDataCache::Index fUniform;

    Draw* fHead;
    Draw* fTail;

    Draw fFirstNode;

    BindingNode* fNext;
#if defined(USE_TREE2_SKIPLIST)
    BindingNode* fSkipNext;
#endif

    void add(SkArenaAlloc* alloc,
             DrawParams* draw,
             const LayerKey& key,
             UniformDataCache::Index uniformIndex,
             bool backToFront) {
        SkASSERT(key.fUniformIndex == fUniform && key.fTextureIndex == fTexture);
        if (fHead) {
            Draw* target = alloc->make<Draw>(draw, uniformIndex);
            if (backToFront) {
                fTail->fNext = target;
                fTail = target;
            } else {
                target->fNext = fHead;
                fHead = target;
            }
        } else {
            fFirstNode = Draw(draw, uniformIndex);
            fHead = fTail = &fFirstNode;
        }
    }
};

struct PipelineNode {
    PipelineNode() = default;
    PipelineNode(const LayerKey& key, const RenderStep* step)
            : fPipeline(key.fPipelineIndex)
            , fStep(step)
            , fPerformsShading(key.performsShading())
            , fHead(nullptr)
            , fTail(nullptr)
            , fNext(nullptr) {}

#if defined(BULK_BINDING_BOUNDS)
    BoundsBlock fBounds;
#endif

    GraphicsPipelineCache::Index fPipeline;
    const RenderStep* fStep;
    bool fPerformsShading;

#if defined (USE_TREE2_SKIPLIST)
    std::array<BindingNode*, 4> fSkipList{};
#endif

    BindingNode* fHead;
    BindingNode* fTail;

    BindingNode fFirstNode;

    // PipelineNode* fNext;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(PipelineNode);

    void add(SkArenaAlloc* alloc,
             DrawParams* draw,
             const LayerKey& key,
             UniformDataCache::Index uniformIndex,
             bool backToFront,
             int* testDepth) {
        BindingNode* target;
        if (fHead) {
            target = this->find(key, testDepth);
            if (!target) {
                target = alloc->make<BindingNode>(key.fTextureIndex, key.fUniformIndex);
                if (backToFront) {
                    fTail->fNext = target;
                    fTail = target;
                } else {
                    target->fNext = fHead;
                    fHead = target;
                }

#if defined(USE_TREE2_SKIPLIST)
                int index = this->bindingIndex(key);
                target->fSkipNext = fSkipList[index];
                fSkipList[index] = target;
#endif
            }
        } else {
            // Initialize first node in the tree
            fFirstNode = BindingNode(key.fTextureIndex, key.fUniformIndex);
            fHead = fTail = &fFirstNode;
#if defined(USE_TREE2_SKIPLIST)
            fSkipList[this->bindingIndex(key)] = &fFirstNode;
#endif
            target = fHead;
        }

        target->add(alloc, draw, key, uniformIndex, backToFront);
#if defined(BULK_BINDING_BOUNDS)
        fBounds.add(draw->drawBounds());
#endif
    }

    int bindingIndex(const LayerKey& key) const {
        return (key.fTextureIndex ^ key.fUniformIndex) & 3;
    }

    BindingNode* find(const LayerKey& key, int* testDepth) const {

#if defined(USE_TREE2_SKIPLIST)
        int index = this->bindingIndex(key);
        for (BindingNode* b = fSkipList[index]; b; b = b->fSkipNext) {
            (*testDepth)++;
            if (b->fTexture == key.fTextureIndex && b->fUniform == key.fUniformIndex) {
                return b;
            }
        }
        return nullptr;
#else
        for (BindingNode* b = fHead; b; b = b->fNext) {
            (*testDepth)++;
            if (b->fTexture == key.fTextureIndex && b->fUniform == key.fUniformIndex) {
                return b;
            }
        }
        return nullptr;
#endif
    }
};

struct BindingTree {
    // PipelineNode* fHead = nullptr;
    // PipelineNode* fTail = nullptr;

    SkTInternalLList<PipelineNode> fNodes;

    PipelineNode fFirstNode;

    PipelineNode* add(SkArenaAlloc* alloc,
            DrawParams* draw,
            const LayerKey& key,
            const RenderStep* step,
            UniformDataCache::Index uniformIndex,
            PipelineNode* insertBefore,
            bool backToFront,
            int* testDepth) { // FIXME make layer key flag?
        PipelineNode* target = nullptr;
        if (!fNodes.isEmpty()) {
        // if (fHead) {
            // target = this->find(key);
            // if (!target) {
            //     target = alloc->make<PipelineNode>(key.fPipelineIndex);
            //     if (backToFront) {
            //         fTail->fNext = target;
            //         fTail = target;
            //     } else {
            //         target->fNext = fHead;
            //         fHead = target;
            //     }
            // }

            for (PipelineNode* p = insertBefore ? insertBefore->fPrev : fNodes.tail();
                  p != nullptr; p = p->fPrev) {
                (*testDepth)++;
                if (p->fPipeline == key.fPipelineIndex) {
                    // p->add(alloc, draw, key, step, uniformIndex, backToFront);
                    // return p;
                    target = p;
                    break;
                }
            }

            if (!target) {
                target = alloc->make<PipelineNode>(key, step);
                if (insertBefore && key.performsShading() == insertBefore->fPerformsShading) {
                    // Since both keys' shading state matches, putting the new list right in front of
                    // `insertBefore` will not split the two sections (regardless of whether it was in the
                    // shading or non-shading section).
                    fNodes.addBefore(target, insertBefore);
                } else if (key.performsShading()) {
                    // Since a new shading binding can only be inserted before other shading bindings,
                    // the only way to get to this branch is to not have an insertBefore target. As such,
                    // the simplest way to maintain keeping shading bindings in the latter half is to add
                    // to the tail.
                    SkASSERT(!insertBefore);
                    fNodes.addToTail(target);
                } else {
                    // A non-shading draw can have an `insertBefore` target that is a shading binding (e.g.
                    // where the final shading step was inserted in the layer). In that case, addBefore()
                    // would possibly split the shading bindings section of `fBindings`. Adding it to the
                    // head of the bindings' list preserves the guarantee that all non-shading bindings are
                    // at the start and satisfies adding it before the `insertBefore` (if it were non-null).
                    SkASSERT(!key.performsShading());
                    SkASSERT(!insertBefore || insertBefore->fPerformsShading);
                    fNodes.addToHead(target);
                }
            }

            /*
            if (key.performsShading()) {
                // Search backwards and stop once reaching the !performsShading() pipelines
                // PipelineNode* shadingBoundary = nullptr;
                for (PipelineNode* p = insertBefore ? insertBefore->fPrev : fNodes.tail();
                     p != nullptr; p = p->fPrev) {
                    (*testDepth)++;
                    if (p->fPipeline == key.fPipelineIndex) {
                        target = p;
                        break;
                    } else if (!p->fPerformsShading) {
                        // shadingBoundary = p->fNext;
                        if (!backToFront) {
                            insertBefore = p->fNext;
                        }
                        break;
                    }
                }

                if (!target) {
                    target = alloc->make<PipelineNode>(key, step);
                    if (insertBefore) {
                        fNodes.addBefore(target, insertBefore);
                    } else if (backToFront) {
                        fNodes.addToTail(target);
                    } else {
                        SkASSERT(fNodes.head()->fPerformsShading); // otherwise we should have set insertBefore
                        fNodes.addToHead(target);
                    }
                }
            } else {
                // Search forwards and stop once reaching the performsShading() pipelines
                PipelineNode* stop = insertBefore ? insertBefore : nullptr;
                for (PipelineNode* p = fNodes.head(); p != stop; p = p->fNext) {
                    (*testDepth)++;
                    if (p->fPipeline == key.fPipelineIndex) {
                        target = p;
                        break;
                    } else if (p->fPerformsShading) {
                        if (backToFront) {
                            insertBefore = p;
                        } else {
                            insertBefore = nullptr;
                        }
                        break;
                    }
                }
                if (!target) {
                    target = alloc->make<PipelineNode>(key, step);
                    if (insertBefore) {
                        fNodes.addBefore(target, insertBefore);
                    } else if (backToFront) {
                        SkASSERT(!fNodes.tail()->fPerformsShading); // otherwise we should have set insertBefore
                        fNodes.addToTail(target);
                    } else {
                        fNodes.addToHead(target);
                    }
                }
            }
            */

        } else {
            // Initialize first node in the tree
            fFirstNode = PipelineNode(key, step);
            fNodes.addToHead(&fFirstNode);
            target = &fFirstNode;
            // fHead = fTail = &fFirstNode;
            // target = fHead;
        }
        target->add(alloc, draw, key, uniformIndex, backToFront, testDepth);
        return target;
    }

    // PipelineNode* find(const LayerKey& key) {
    //     // for (PipelineNode* p = fHead; p; p = p->fNext) {
    //     for (PipelineNode* p : fNodes) {
    //         if (p->fPipeline == key.fPipelineIndex) {
    //             return p;
    //         }
    //     }
    //     return nullptr;
    // }
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
#if defined(USE_BINDING_TREE2)
    BindingTree fTree;

#if defined(BULK_BINDING_BOUNDS)
    BoundsBlock fStencilBounds;
#endif

#else
    SkTInternalLList<BindingList> fBindings;
#endif

#if defined(BULK_BOUNDS)
    BoundsBlock fColorBounds;
    BoundsBlock fStencilBounds;
#endif

#if defined(USE_BINDING_TREE)
    BindingTree fTree;
#endif


#if defined(USE_QUICK_LOOKUP)
    std::array<BindingList*, 8> fSkipList{};

    int skipIndex(const LayerKey& key) const {
        // I only have 3 bits, so what should I put in there?
        // return (key.fTextureIndex ^ key.fUniformIndex) & 7;
        return (((key.fPipelineIndex & 1) << 2) |
                ((key.fTextureIndex & 1) << 1) |
                ((key.fUniformIndex & 1) << 0)) & 7;
    }
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

#if defined(USE_BINDING_TREE2)
        int fPipelineCount = 0;
        int fMaxBindingsPerPipeline = 0;
        int fMaxDrawsPerPipeline = 0;
        float fAvgBindingsPerPipeline = 0;
        float fAvgDrawsPerPipeline = 0;
#endif

        int fStencilCount = 0;
    };

    Stats collectStats() const {
        Stats stats;

#if defined(USE_BINDING_TREE2)
        for (const PipelineNode* p : fTree.fNodes) {
            stats.fPipelineCount++;

            int bindingCount = 0;
            int pipelineDraws = 0;
            for (const BindingNode* b = p->fHead; b; b = b->fNext) {
                bindingCount++;
                int bindingDraws = 0;
                for (const Draw* d = b->fHead; d; d = d->fNext) {
                    bindingDraws++;
                }
                pipelineDraws += bindingDraws;
                stats.fMaxDrawsPerBindingList = std::max(bindingDraws, stats.fMaxDrawsPerBindingList);
            }

            stats.fTotalDrawCount += pipelineDraws;
            stats.fBindingListCount += bindingCount;

            stats.fMaxDrawsPerPipeline = std::max(pipelineDraws, stats.fMaxDrawsPerPipeline);
            stats.fMaxBindingsPerPipeline = std::max(bindingCount, stats.fMaxBindingsPerPipeline);
            stats.fBindingListCount += bindingCount;
        }

        stats.fAvgBindingsPerPipeline = stats.fBindingListCount / (float) stats.fPipelineCount;
        stats.fAvgDrawsPerPipeline = stats.fTotalDrawCount / (float) stats.fPipelineCount;
#else
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
#endif
        stats.fMaxTestDepth = fMaxTestDepth;
        stats.fAvgDrawsPerBindingList = stats.fTotalDrawCount / (float) stats.fBindingListCount;
        stats.fAvgTestDepth = fTestDepth / (float) fTestCount;

        return stats;
    }


#if defined(USE_BINDING_TREE2)

int fTestCount = 0;
int fTestDepth = 0;
int fMaxTestDepth = 0;

SK_ALWAYS_INLINE PipelineNode* add(SkArenaAlloc* alloc,
            DrawParams* draw,
            const LayerKey& key,
            const RenderStep* step,
            UniformDataCache::Index uniformIndex,
            PipelineNode* insertBefore,
            bool backToFront) { // FIXME make layer key flag?)
    int oldDepth = fTestDepth;
    fTestCount++;
    PipelineNode* node = fTree.add(alloc, draw, key, step, uniformIndex, insertBefore, backToFront, &fTestDepth);
    fMaxTestDepth = std::max(fTestDepth - oldDepth, fMaxTestDepth);

#if !defined(BULK_BINDING_BOUNDS)
    if (key.fFlags & BoundsFlags::kColor) {
        fColorBounds.add(draw->drawBounds());
    }
#endif
    if (key.fFlags & BoundsFlags::kStencil) {
        fStencilBounds.add(draw->drawBounds());
    }

    return node;
}

#if defined(BULK_BINDING_BOUNDS)


SK_ALWAYS_INLINE void addMatched(SkArenaAlloc* alloc,
            PipelineNode* match,
            DrawParams* draw,
            const LayerKey& key,
            UniformDataCache::Index uniformIndex,
            bool backToFront) { // FIXME make layer key flag?)
    int oldDepth = fTestDepth;
    fTestCount++;
    match->add(alloc, draw, key, uniformIndex, backToFront, &fTestDepth);
    fMaxTestDepth = std::max(fTestDepth - oldDepth, fMaxTestDepth);

    if (key.fFlags & BoundsFlags::kStencil) {
        fStencilBounds.add(draw->drawBounds());
    }
}

SK_ALWAYS_INLINE std::pair<SkEnumBitMask<BoundsTestResult>, PipelineNode*> test(
            const Rect::ComplementRect drawBounds,
            const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {
        static constexpr SkEnumBitMask<BoundsTestResult> kFull = BoundsTestResult::kAllowedInLayer |
                                                 BoundsTestResult::kAllowedBeforeLayer;
        SkEnumBitMask<BoundsTestResult> result = kFull;

    if ((testMask & BoundsFlags::kStencil) && fStencilBounds.intersects(drawBounds)) {
        result &= ~BoundsTestResult::kAllowedInLayer;
    }

    PipelineNode* match = nullptr;

    for (PipelineNode* p = fTree.fNodes.tail(); p; p = p->fPrev) {
        if ((testMask & BoundsFlags::kColor)) {
            if (p->fPerformsShading && p->fBounds.intersects(drawBounds)) {
                if (!match) {
                    result = BoundsTestResult::kBlocked;
                }
                break;
            }
        }
        // NOTE: we couldn't do this first because we may end up adding to a binding node
        // that draws before the thing we intersected with. At that point we violated the
        // presumed rasterization order that the pure bindinglist technique was relying on
         if (p->fPipeline == key.fPipelineIndex) {
            match = p;
            // FIXME kMustBeDisjoint...
        }
        if (key.performsShading() && !p->fPerformsShading) {
            break;
        }
    }

    return {result, match};
}
#else
SK_ALWAYS_INLINE SkEnumBitMask<BoundsTestResult> test(
            const Rect::ComplementRect drawBounds,
            // const LayerKey& key,
            SkEnumBitMask<BoundsFlags> testMask) {
    // BindingList* match = nullptr;
    SkEnumBitMask<BoundsTestResult> result = BoundsTestResult::kBlocked;
    if (!(testMask & BoundsFlags::kColor) || !fColorBounds.intersects(drawBounds)) {
        result |= BoundsTestResult::kAllowedBeforeLayer;
        if (!(testMask & BoundsFlags::kStencil) || !fStencilBounds.intersects(drawBounds)) {
            result |= BoundsTestResult::kAllowedInLayer;
        }
    }

    return result; //{result, match};
}
#endif

// Okay, but one thing that the BindingList approach did have going for it was that we could
// prepend a BindingList and depend on it to run the preceding RenderStep in the right order.
// If we have a single BindingTree that all the multi-render steps need to go into, how do we
// control those sequences?

//  -> we push those pipelines as backToFront? That could work, but it's a bit brittle if
//     we would have a match on pipeline that was added naturally after* the last step's pipeline
//     and then we have to skip past it or similar. Or if a renderer used multiple steps with the
//     same pipeline(???), then we'd have to push the ordering through to the deeper nodes.

//  -> we have a phase or step root for the binding tree, where we push based on distance to last
//     step, although that gets tricky for clips because we can't just give them a single phase.

//  -> we make the nodes doubly-linked and when we add the main draw, we return the path through
//     the tree, so that it's possible to then add relative to that one.   Actually that should
//     be possible by just making the pipeline nodes doubly-linked and having them follow the
//     same insert rules as BindingList.
//    -> although then we lose the back-to-front ordering convenience, but that's probably best
//    controlled with a base Layer that is kept occluder only
//    -> we could still preserve back-to-front shading of pipelines if we remember where the
//     shading vs. non-shading split is and use that as the "front" target for shading draws and
//     the head is the target for non-shading draws. But that would also require that we
//     remember the tail of the non-shading section to be able to splice the lists together
//    -> or we just have two lists again?
//    -> or we remember if a layer has non-shading in it and use that to not shuffle things around?

//  -> wellll actually, during the add() we are searching for the match already so we could keep
//     track of the non-shading and shading splits.  This would just have to coordinate with the
//     doubly-linked list business for insertBefore.  But when we wnat to find the pipeline and
//     the right back-to-front spot, its when we have a null insertBefore!!
#else
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
        BindingList* pipelineMatch = nullptr;
#if defined(USE_QUICK_LOOKUP)
        if (!startList) {
            int oldDepth = fTestDepth;
            int index = this->skipIndex(key);
            for (BindingList* list = fSkipList[index]; list; list = list->fSkipNext) {
                fTestCount++;
                if (list->fKey.isEqual(key)) {
                    fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
                    return list;
                } else if (list->isBetterMatch(key, pipelineMatch)) {
                    pipelineMatch = list;
                }
            }
                    fMaxTestDepth = std::max(fMaxTestDepth, fTestDepth - oldDepth);
            return pipelineMatch;
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
                // match = this->searchBinding(key);
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
                if (fBindings.tail()->fKey.isEqual(key)) {
                    result |= BoundsTestResult::kAllowedInLayer;
                    match = fBindings.tail();
                    // SkDebugf("append overlap\n");
                }
            } else {
                // This layer is forward eligible, so we could do a forward merge.
                match = this->searchBinding(key, nullptr, true);
                if (match && !match->fPrev && !match->fNext) {
                    // This is the only binding list in the layer, so don't pull it foward
                    // FIXME only works for exact matches since compat match turns into a
                    // order inversion in the binding list.
                    result |= BoundsTestResult::kAllowedInLayer;
                    // SkDebugf("found singleton legacy forward merge\n");
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

#if defined(USE_QUICK_LOOKUP)
        int index = this->skipIndex(key);
        list->fSkipNext = fSkipList[index];
        fSkipList[index] = list;
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

#if defined(BULK_BOUNDS)
        for (const Draw* d = binding->fHead; d; d = d->fNext) {
            newLayer->fColorBounds.add(d->fDrawParams->drawBounds());
        }
#endif

#if defined(USE_QUICK_LOOKUP)
        int index = this->skipIndex(binding->fKey);
        if (fSkipList[index] == binding) {
            fSkipList[index] = binding->fSkipNext;
        } else {
            for (BindingList* p = fSkipList[index]; p; p = p->fSkipNext) {
                if (p->fSkipNext == binding) {
                    p->fSkipNext = binding->fSkipNext;
                    break;
                }
            }
        }

        binding->fSkipNext = nullptr;
        newLayer->fSkipList[index] = binding;
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
    }
#endif // !USE_BINDING_TREE2
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DrawListTypes_DEFINED
