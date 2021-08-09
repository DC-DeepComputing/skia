/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterClipStack_DEFINED
#define SkRasterClipStack_DEFINED

#include "include/core/SkClipOp.h"
#include "src/core/SkRasterClip.h"
#include <stack>

class SkRasterClipStack : SkNoncopyable {
public:
    SkRasterClipStack(int width, int height)
        : fRootBounds(SkIRect::MakeWH(width, height))
    {
        fStack.push({SkRasterClip(fRootBounds), 0});
        SkASSERT(fStack.size() == 1);
    }

    void setNewSize(int w, int h) {
        fRootBounds.setXYWH(0, 0, w, h);

        SkASSERT(fStack.size() == 1);
        Rec& rec = fStack.top();
        SkASSERT(rec.fDeferredCount == 0);
        rec.fRC.setRect(fRootBounds);
    }

    const SkRasterClip& rc() const { return fStack.top().fRC; }

    void save() {
        fCounter += 1;
        SkASSERT(fStack.top().fDeferredCount >= 0);
        fStack.top().fDeferredCount += 1;
    }

    void restore() {
        fCounter -= 1; SkASSERT(fCounter >= 0);
        if (--fStack.top().fDeferredCount < 0) {
            SkASSERT(fStack.top().fDeferredCount == -1);
            SkASSERT(fStack.size() > 1);
            fStack.pop();
        }
    }

    void clipRect(const SkMatrix& ctm, const SkRect& rect, SkClipOp op, bool aa) {
        this->writable_rc().op(rect, ctm, op, aa);
        this->validate();
    }

    void clipRRect(const SkMatrix& ctm, const SkRRect& rrect, SkClipOp op, bool aa) {
        this->writable_rc().op(rrect, ctm, op, aa);
        this->validate();
    }

    void clipPath(const SkMatrix& ctm, const SkPath& path, SkClipOp op, bool aa) {
        this->writable_rc().op(path, ctm, op, aa);
        this->validate();
    }

    void clipShader(sk_sp<SkShader> sh) {
        this->writable_rc().op(std::move(sh));
        this->validate();
    }

    void clipRegion(const SkRegion& rgn, SkClipOp op) {
        this->writable_rc().op(rgn, op);
        this->validate();
    }

    void replaceClip(const SkIRect& rect) {
        SkIRect devRect = rect;
        if (!devRect.intersect(fRootBounds)) {
            this->writable_rc().setEmpty();
        } else {
            this->writable_rc().setRect(devRect);
        }
    }

    void validate() const {
#ifdef SK_DEBUG
        const SkRasterClip& clip = this->rc();
        if (fRootBounds.isEmpty()) {
            SkASSERT(clip.isEmpty());
        } else if (!clip.isEmpty()) {
            SkASSERT(fRootBounds.contains(clip.getBounds()));
        }
#endif
    }

private:
    struct Rec {
        SkRasterClip fRC;
        int          fDeferredCount; // 0 for a "normal" entry
    };

    // fStack wraps a std::deque, which uses a sequence of fixed-size arrays to store Recs, and
    // does not need to copy elements when it expands.
    std::stack<Rec> fStack;
    SkIRect         fRootBounds;
    SkDEBUGCODE(int fCounter = 0);

    SkRasterClip& writable_rc() {
        SkASSERT(fStack.top().fDeferredCount >= 0);
        if (fStack.top().fDeferredCount > 0) {
            fStack.top().fDeferredCount -= 1;
            fStack.push(fStack.top());
            fStack.top().fDeferredCount = 0;
        }
        return fStack.top().fRC;
    }
};

#endif
