/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PaintParamsKey_DEFINED
#define skgpu_graphite_PaintParamsKey_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/BuiltInCodeSnippetID.h"

#include <limits>
#include <optional>

class SkArenaAlloc;

namespace skgpu::graphite {

class PaintParamsKeyBuilder;
class ShaderCodeDictionary;
class ShaderNode;

// This class is a compact representation of the shader needed to implement a given
// PaintParams. Its structure is a series of nodes where each node consists of:
//   4 bytes: code-snippet ID
//   N child nodes, where N is the constant number of children defined by the ShaderCodeDictionary
//     for the block's snippet ID.
//
// All children of a child node are stored in the key before the next child is encoded in the key,
// e.g. iterating the data in a key is a depth-first traversal of the node tree.
class PaintParamsKey {
public:
    // PaintParamsKey can only be created by using a PaintParamsKeyBuilder or by cloning the key
    // data from a Builder-owned key.
    PaintParamsKey(const PaintParamsKey&) = default;

    ~PaintParamsKey() {
        if (fOriginatingBuilder) {
            fOriginatingBuilder->unlock();
        }
    }

    // Converts the key into a forest of ShaderNode trees. If the key is valid this will return at
    // least one root node. If the key contains unknown shader snippet IDs an empty span is
    // returned. All shader nodes, and the returned span, are owned by the provided arena.
    SkSpan<const ShaderNode*> getRootNodes(const ShaderCodeDictionary*, SkArenaAlloc*) const;

    // Return a PaintParamsKey whose data is owned by the provided arena and is not attached to
    // a PaintParamsKeyBuilder. The caller must ensure that the SkArenaAlloc remains alive longer
    // than the returned key.
    PaintParamsKey clone(SkArenaAlloc*) const;

    bool operator==(const PaintParamsKey& that) const;
    bool operator!=(const PaintParamsKey& that) const { return !(*this == that); }

#ifdef SK_DEBUG
    void dump(const ShaderCodeDictionary*) const;
#endif

private:
    friend class PaintParamsKeyBuilder;   // for the parented-data ctor

    // PaintParamsKey never owns its memory. If 'originatingBuilder' is non-null it is sharing the
    // Builder's memory. It is assumed the non-null Builder is already locked and it will be
    // unlocked by this Key's destructor. If the builder is null, it is assumed the key data is
    // owned externally (e.g. clone(SkArenaAlloc*).
    PaintParamsKey(SkSpan<const int32_t> span, PaintParamsKeyBuilder* originatingBuilder=nullptr)
            : fData(span)
            , fOriginatingBuilder(originatingBuilder) {
        SkASSERT(!fData.empty());
        SkASSERT(!fOriginatingBuilder || fOriginatingBuilder->fLocked);
    }

    // Returns null if the node or any of its children are invalid. Modifies 'currentIndex' in
    // depth-first traveersal of the key's effect tree.
    const ShaderNode* createNode(const ShaderCodeDictionary*,
                                 int* currentIndex,
                                 SkArenaAlloc* arena) const;

    // The memory referenced in 'fData' is always owned by someone else.
    // If 'fOriginatingBuilder' is null, the dictionary's SkArena owns the 'fData' memory and no
    // explicit freeing is required.
    // If 'fOriginatingBuilder' is non-null then the 'fData' memory must be explicitly locked (in
    // the ctor) and unlocked (in the dtor) on the 'fOriginatingBuilder' object.
    SkSpan<const int32_t> fData;

    // This class should only ever access the 'lock' and 'unlock' calls on 'fOriginatingBuilder'
    PaintParamsKeyBuilder* fOriginatingBuilder;
};

// The PaintParamsKeyBuilder and the PaintParamsKeys snapped from it share the same
// underlying block of memory. When an PaintParamsKey is snapped from the builder it 'locks'
// the memory and 'unlocks' it in its destructor. Because of this relationship, the builder
// can only have one extant key and that key must be destroyed before the builder can be reused
// to create another one.
//
// This arrangement is intended to improve performance in the expected case, where a builder is
// being used in a tight loop to generate keys which can be recycled once they've been used to
// find the dictionary's matching uniqueID. We don't expect the cost of copying the key's memory
// into the dictionary to be prohibitive since that should be infrequent.
class PaintParamsKeyBuilder {
public:
    PaintParamsKeyBuilder(const ShaderCodeDictionary* dict) {
        SkDEBUGCODE(fDict = dict;)
    }

    ~PaintParamsKeyBuilder() { SkASSERT(!fLocked); }

    void beginBlock(BuiltInCodeSnippetID id) { this->beginBlock(static_cast<int32_t>(id)); }
    void beginBlock(int32_t codeSnippetID) {
        SkASSERT(fDict->isValidID(codeSnippetID));
        SkASSERT(!fLocked);

        SkDEBUGCODE(this->pushStack(codeSnippetID);)
        fData.push_back(codeSnippetID);
    }

    // TODO: Have endBlock() be handled automatically with RAII, in which case we could have it
    // validate the snippet ID being popped off the stack frame.
    void endBlock() {
        SkDEBUGCODE(this->popStack();)
    }

    // Returns a view of this builder as a PaintParamsKey. The Builder cannot be used until the
    // returned Key goes out of scope.
    PaintParamsKey lockAsKey() {
        SkASSERT(!fLocked);       // lockAsKey() is not re-entrant
        SkASSERT(!fData.empty()); // Must have code IDs added to the key
        SkASSERT(fStack.empty()); // All beginBlocks() had a matching endBlock()

        SkDEBUGCODE(fLocked = true;)
        return PaintParamsKey({fData.data(), fData.size()}, this);
    }

#ifdef SK_DEBUG
    // Check that the builder has been reset to its initial state prior to creating a new key.
    void checkReset();
#endif

private:
    friend class PaintParamsKey; // for unlock()

    void unlock() {
        SkASSERT(fLocked);
        fData.clear();

        SkDEBUGCODE(fLocked = false;)
        SkDEBUGCODE(fStack.clear();)
        SkDEBUGCODE(this->checkReset();)
    }

    // The data array uses clear() on unlock so that it's underlying storage and repeated use of the
    // builder will hit a high-water mark and avoid lots of allocations when recording draws.
    SkTDArray<int32_t> fData;

#ifdef SK_DEBUG
    void pushStack(int32_t codeSnippetID);
    void popStack();

    // Information about the current block being written
    struct StackFrame {
        int fCodeSnippetID;
        int fNumExpectedChildren;
        int fNumActualChildren = 0;
    };

    const ShaderCodeDictionary* fDict;
    SkTDArray<StackFrame> fStack;
    bool fLocked = false;
#endif
};

} // skgpu::graphite

#endif // skgpu_graphite_PaintParamsKey_DEFINED
