/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ClipStack_DEFINED
#define skgpu_ClipStack_DEFINED

#include "experimental/graphite/src/geom/Rect.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkShader.h"
#include "src/core/SkTBlockList.h"

class SkMatrixProvider;

namespace skgpu {

class ClipStack {
    // Internally, a lot of clip reasoning is based on an op, outer bounds, and whether a shape
    // contains another (possibly just conservatively based on inner/outer device-space bounds).
    //
    // Element and SaveRecord store this information directly, but a draw fits the same definition
    // with an implicit intersect op and empty inner bounds. Draw, Element, and SaveRecord all
    // define a common set of functions used by internal templates to evaluate how they interact
    // (e.g. intersect, clipped-out entirely, etc.).
    class Draw;
    class SaveRecord;

public:
    // TODO: These ClipStates reflect what needs to be easily reported by the SkDevice/SkCanvas API
    // and what could be applied analytically or directly via geometry in Ganesh. Some of these
    // states will likely remain of value in Graphite, and we may need to add additional ones when
    // analytic clipping is re-implemented (e.g. a single rect or rrect, but *not* axis-aligned).
    enum class ClipState : uint8_t {
        kEmpty, kWideOpen, kDeviceRect, kDeviceRRect, kComplex
    };

    // All data describing a geometric modification to the clip
    class Element {
    public:
        Element(const Transform& localToDevice, const Shape& shape, SkClipOp op);

        ClipState        clipType()      const;

        const Rect&      bounds()        const { return fOuterBounds;   }
        const Transform& localToDevice() const { return fLocalToDevice; }
        const Shape&     shape()         const { return fShape;         }
        SkClipOp         op()            const { return fOp;            }

        // As new elements are pushed on to the stack, they may make older elements redundant.
        // The old elements are marked invalid so they are skipped during clip application, but may
        // become active again when a save record is restored.
        bool isInvalid() const { return fInvalidatedByIndex >= 0; }

    private:
        using Stack = SkTBlockList<Element, 1>;

        bool contains(const SaveRecord& s) const;
        bool contains(const Draw& d) const;
        bool contains(const Element& e) const;

        void markInvalid(const SaveRecord& current);
        void restoreValid(const SaveRecord& current);

        // 'added' represents a new op added to the element stack. Its combination with this element
        // can result in a number of possibilities:
        //  1. The entire clip is empty (signaled by both this and 'added' being invalidated).
        //  2. The 'added' op supersedes this element (this element is invalidated).
        //  3. This op supersedes the 'added' element (the added element is marked invalidated).
        //  4. Their combination can be represented by a single new op (in which case this
        //     element is be invalidated, and 'added' has been modified to reflect the combination).
        //  5. Or both elements remain needed to describe the clip (both are valid and unchanged).
        //
        // The calling element will only modify its invalidation index since it could belong
        // to part of the inactive stack (that might be restored later). All merged state/geometry
        // is handled by modifying 'added'.
        void updateForElement(Element* added, const SaveRecord& current);

        bool combine(const Element& other, const SaveRecord& current);

        Rect      fBounds; // Device space bounds
        Transform fLocalToDevice; // TODO: Make this a reference to a de-duplicated transform cache?
        Shape     fShape;
        SkClipOp  fOp;

        // Elements are invalidated by SaveRecords as the record is updated with new elements that
        // override old geometry. An invalidated element stores the index of the first element of
        // the save record that invalidated it. This makes it easy to undo when the save record is
        // popped from the stack, and is stable as the current save record is modified.
        int fInvalidatedByIndex;

        // As clips are applied to draws, the clip stack tracks which elements actually need to
        // be drawn into the depth attachment, and what minimal sub-region is actually required.
        // The first time an element affects a draw, it is assigned a compressed painter's order for
        // how it must be sorted into the final DrawList. Until the Device is flushed or the element
        // is popped off the clip stack, it tracks the highest PaintersDepth of all additional draws
        // it affects, which is then the depth value it writes into the depth attachment.
        Rect fUsageBounds;
        CompressedPaintersOrder fPaintOrder = DrawOrder::kNoIntersection; // no-usage
        PaintersDepth fMaxDrawDepth = DrawOrder::kClearDepth;
    };

    // The SkMatrixProvider must outlive the ClipStack.
    ClipStack(const SkIRect& deviceBounds, const SkMatrixProvider* matrixProvider);

    ClipStack(const ClipStack&) = delete;
    ClipStack& operator=(const ClipStack&) = delete;

    ClipState clipState() const { return this->currentSaveRecord().state(); }
    Rect conservativeBounds() const;

    class ElementIter;
    // Provides for-range over active, valid clip elements from most recent to oldest.
    // The iterator provides items as "const Element&".
    inline ElementIter begin() const;
    inline ElementIter end() const;

    // Clip stack manipulation
    void save();
    void restore();

    void clipShape(const Transform& localToDevice, const Shape& shape, SkClipOp op);

    void clipShader(sk_sp<SkShader> shader);

    // Applies the clip stack to a draw that is represented by the given local-to-device transform
    // and bounding box. On input, 'drawBounds' is the unclipped bounds of the draw's geometry in
    // the local coordinate system. Any style outsetting should have been accounted for locally
    // before applyClip() is calle.d On output, 'drawBonds' is in device space and is already
    // intersected with the returned scissor.
    //
    // Returns the CompressedPaintersOrder the draw must be drawn after, and the scissor bounds that
    // the draw contents must be restricted to be render correctly. If needed, the ClipStack will
    // record depth-only draws to the DrawContext.
    //
    // FIXME: This API is nice in that it makes it not necessary to deal with style at all, but
    // we can't then pass in the fact that the shape is a rect or rrect. Maybe we don't have drawBounds
    // be mutable, and instead pass in const Shape& and stroke style info. Then the bounds calculation
    // logic that's currently in Device moves here, we can handle hairlines, or shape-specific
    // contains checks internally, and then return the scissor and the draw bounds at the end as
    // a tuple with the painters order
    std::pair<CompressedPaintersOrder, SkIRect> applyClip(DrawContext* dc, const Transform& localToDevice, Rect* drawBounds, PaintersDepth drawZ);

private:
    // Represents a saved point in the clip stack, and manages the life time of elements added to
    // stack within the record's life time. Also provides the logic for determining active elements
    // given a draw query.
    class SaveRecord {
    public:
        using Stack = SkTBlockList<SaveRecord, 2>;

        explicit SaveRecord(const SkIRect& deviceBounds);

        SaveRecord(const SaveRecord& prior, int startingElementIndex);

        ClipState       state()       const;
        SkClipOp        op()          const { return fStackOp;      }
        const SkShader* shader()      const { return fShader.get(); }
        const Rect&     outerBounds() const { return fOuterBounds;  }
        const Rect&     innerBounds() const { return fInnerBounds;  }

        int  firstActiveElementIndex() const { return fStartingElementIndex;     }
        int  oldestElementIndex()      const { return fOldestValidIndex;         }
        bool canBeUpdated()            const { return (fDeferredSaveCount == 0); }

        bool contains(const Draw& d) const;
        bool contains(const RawElement& e) const;

        // Deferred save manipulation
        void pushSave() {
            SkASSERT(fDeferredSaveCount >= 0);
            fDeferredSaveCount++;
        }
        // Returns true if the record should stay alive. False means the ClipStack must delete it
        bool popSave() {
            fDeferredSaveCount--;
            SkASSERT(fDeferredSaveCount >= -1);
            return fDeferredSaveCount >= 0;
        }

        // Return true if the element was added to 'elements', or otherwise affected the save record
        // (e.g. turned it empty).
        bool addElement(Element&& toAdd, Element::Stack* elements);

        void addShader(sk_sp<SkShader> shader);

        // Remove the elements owned by this save record, which must happen before the save record
        // itself is removed from the clip stack.
        void removeElements(RawElement::Stack* elements);

        // Restore element validity now that this record is the new top of the stack.
        void restoreElements(RawElement::Stack* elements);

    private:
        // These functions modify 'elements' and element-dependent state of the record
        // (such as valid index and fState).
        bool appendElement(Element&& toAdd, Element::Stack* elements);
        void replaceWithElement(Element&& toAdd, Element::Stack* elements);

        // Inner bounds is always contained in outer bounds, or it is empty. All bounds will be
        // contained in the device bounds.
        Rect fInnerBounds; // Inside is full coverage (stack op == intersect) or 0 cov (diff)
        Rect fOuterBounds; // Outside is 0 coverage (op == intersect) or full cov (diff)

        // A save record can have up to one shader, multiple shaders are automatically blended
        sk_sp<SkShader> fShader;

        const int fStartingElementIndex;  // First element owned by this save record
        int       fOldestValidIndex; // Index of oldest element that remains valid for this record

        int       fDeferredSaveCount; // Number of save() calls without modifications (yet)

        // Will be kIntersect unless every valid element is kDifference, which is significant
        // because if kDifference then there is an implicit extra outer bounds at the device edges.
        SkClipOp  fStackOp;
        ClipState fState;
    };

    const SaveRecord& currentSaveRecord() const {
        SkASSERT(!fSaves.empty());
        return fSaves.back();
    }

    // Will return the current save record, properly updating deferred saves
    // and initializing a first record if it were empty.
    SaveRecord& writableSaveRecord(bool* wasDeferred);

    Element::Stack    fElements;
    SaveRecord::Stack fSaves; // always has one wide open record at the top

    const SkIRect           fDeviceBounds;
    const SkMatrixProvider* fMatrixProvider;
};

// Clip element iteration
// TODO: Do we need clip element iteration exposed to the public?
class ClipStack::ElementIter {
public:
    bool operator!=(const ElementIter& o) const {
        return o.fItem != fItem && o.fRemaining != fRemaining;
    }

    const Element& operator*() const { return (*fItem).asElement(); }

    ElementIter& operator++() {
        // Skip over invalidated elements
        do {
            fRemaining--;
            ++fItem;
        } while(fRemaining > 0 && (*fItem).isInvalid());

        return *this;
    }

    ElementIter(RawElement::Stack::CRIter::Item item, int r) : fItem(item), fRemaining(r) {}

    RawElement::Stack::CRIter::Item fItem;
    int fRemaining;

    friend class ClipStack;
};

ClipStack::ElementIter ClipStack::begin() const {
    if (this->currentSaveRecord().state() == ClipState::kEmpty ||
        this->currentSaveRecord().state() == ClipState::kWideOpen) {
        // No visible clip elements when empty or wide open
        return this->end();
    }
    int count = fElements.count() - this->currentSaveRecord().oldestElementIndex();
    return ElementIter(fElements.ritems().begin(), count);
}

ClipStack::ElementIter ClipStack::end() const {
    return ElementIter(fElements.ritems().end(), 0);
}

} // namespace skgpu::v1

#endif // ClipStack_DEFINED
