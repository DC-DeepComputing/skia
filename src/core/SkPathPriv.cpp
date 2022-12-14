#include "src/core/SkPathPriv.h"

SkPathEdgeIter::SkPathEdgeIter(const SkPath& path) {
    fMoveToPtr = fPts = path.fPathRef->points();
    fVerbs = path.fPathRef->verbsBegin();
    fVerbsStop = path.fPathRef->verbsEnd();
    fConicWeights = path.fPathRef->conicWeights();
    if (fConicWeights) {
        fConicWeights -= 1;  // begin one behind
    }

    fNeedsCloseLine = false;
    fNextIsNewContour = false;
    SkDEBUGCODE(fIsConic = false;)
}
