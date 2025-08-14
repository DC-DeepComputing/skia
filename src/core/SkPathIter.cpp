/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathIter.h"
#include "src/core/SkPathRaw.h"

SkPathIter::SkPathIter(const SkPathRaw& raw) : SkPathIter(raw.fPoints, raw.fVerbs, raw.fConics) {}

std::optional<SkPathIter::Rec> SkPathIter::next() {
    if (vIndex >= fVerbs.size()) {
        return {};
    }

    size_t n = 0;

    float w = -1;
    SkPathVerb v;
    switch (v = (SkPathVerb)fVerbs[vIndex++]) {
        case SkPathVerb::kMove:
            fClosePointStorage[1] = fPoints[pIndex++]; // remember for close
            return Rec{{&fClosePointStorage[1], 1}, w, v};
        case SkPathVerb::kLine:  n = 1; break;
        case SkPathVerb::kQuad:  n = 2; break;
        case SkPathVerb::kConic: n = 2; w = fConics[cIndex++]; break;
        case SkPathVerb::kCubic: n = 3; break;
        case SkPathVerb::kClose:
            SkASSERT(pIndex > 0);
            fClosePointStorage[0] = fPoints[pIndex-1];   // the last point we saw
            return Rec{fClosePointStorage, w, v};
    }

    SkASSERT(pIndex > 0);
    auto start = pIndex - 1;
    SkASSERT(n >= 1 && n <= 3);
    pIndex += n;
    return Rec{{&fPoints[start], n+1}, w, v};

}

///////////////////////////////////////////////

SkPathContourIter::SkPathContourIter(const SkPathRaw& raw)
    : SkPathContourIter(raw.fPoints, raw.fVerbs, raw.fConics)
{}

std::optional<SkPathRaw::ContourRec> SkPathRaw::ContourIter::next() {
    if (fVerbs.empty()) {
        return {};
    }

    SkASSERT(fVerbs[0] == SkPathVerb::kMove);
    size_t npts = 1, nvbs = 1, nws = 0;

    for (size_t i = 1; i < fVerbs.size(); ++i) {
        switch ((SkPathVerb)fVerbs[i]) {
            case SkPathVerb::kMove: goto DONE;
            case SkPathVerb::kLine:  npts += 1; break;
            case SkPathVerb::kQuad:  npts += 2; break;
            case SkPathVerb::kConic: npts += 2; nws += 1; break;
            case SkPathVerb::kCubic: npts += 3; break;
            case SkPathVerb::kClose: nvbs += 1; goto DONE;
        }
        nvbs += 1;
    }
DONE:
    ContourRec rec = {
        fPoints.subspan(0, npts),
        fVerbs.subspan(0, nvbs),
        fConics.subspan(0, nws),
    };
    fPoints = fPoints.last(fPoints.size() - npts);
    fVerbs  = fVerbs.last( fVerbs.size()  - nvbs);
    fConics = fConics.last(fConics.size() - nws);
    return rec;
}
