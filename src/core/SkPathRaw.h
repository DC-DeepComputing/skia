/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRaw_DEFINED
#define SkPathRaw_DEFINED

#include "include/core/SkPathTypes.h" // IWYU pragma: keep
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkPoint_impl.h"
#include "src/core/SkPathMakers.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

struct SkPathRaw {
    SkSpan<const SkPoint>    fPoints;
    SkSpan<const SkPathVerb> fVerbs;
    SkSpan<const float>      fConics;
    SkRect                   fBounds;
    SkPathFillType           fFillType;
    bool                     fIsConvex;

    bool empty() const { return fVerbs.empty(); }

    struct IterRec {
        SkSpan<const SkPoint> pts;
        float                 w;
        SkPathVerb            vrb;
    };

    class Iter {
    public:
        Iter(const SkPathRaw& p) : fPoints(p.fPoints), fVerbs(p.fVerbs), fConics(p.fConics) {
            vIndex = pIndex = cIndex = 0;
        }

        Iter(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs, SkSpan<const float> cns)
            : fPoints(pts), fVerbs(vbs), fConics(cns) {
            vIndex = pIndex = cIndex = 0;
        }

        /*  Holds the current verb, and its associated points
         *  move:  pts[0]
         *  line:  pts[0..1]
         *  quad:  pts[0..2]
         *  conic: pts[0..2] w
         *  cubic: pts[0..3]
         *  close: pts[0..1] ... as if close were a line from pts[0] to pts[1]
         */
        std::optional<IterRec> next();

    private:
        size_t                   vIndex, pIndex, cIndex;
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
        std::array<SkPoint, 2>   fPointStorage;
    };

    Iter iter() const {
        return Iter(*this);
    }

    struct ContourRec {
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
    };

    class ContourIter {
    public:
        ContourIter(const SkPathRaw&);

        std::optional<ContourRec> next();

    private:
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
    };
};

struct Close {
    static constexpr SkPathVerb kVerb = SkPathVerb::kClose;
    std::array<SkPoint, 0> pts;
    std::array<float  , 0> wts;

    constexpr Close() {}
};

struct MoveTo {
    static constexpr SkPathVerb kVerb = SkPathVerb::kMove;
    std::array<SkPoint, 1> pts;
    std::array<float  , 0> wts;

    constexpr MoveTo(SkPoint p) : pts{p} {}
};

struct LineTo {
    static constexpr SkPathVerb kVerb = SkPathVerb::kLine;
    std::array<SkPoint, 1> pts;
    std::array<float  , 0> wts;

    constexpr LineTo(SkPoint p) : pts{p} {}
};

struct QuadTo {
    static constexpr SkPathVerb kVerb = SkPathVerb::kQuad;
    std::array<SkPoint, 2> pts;
    std::array<float  , 0> wts;

    constexpr QuadTo(SkPoint p0, SkPoint p1) : pts{p0, p1} {}
};

struct CubicTo {
    static constexpr SkPathVerb kVerb = SkPathVerb::kCubic;
    std::array<SkPoint, 3> pts;
    std::array<float  , 0> wts;

    constexpr CubicTo(SkPoint p0, SkPoint p1, SkPoint p2) : pts{p0, p1, p2} {}
};

struct ConicTo {
    static constexpr SkPathVerb kVerb = SkPathVerb::kConic;
    std::array<SkPoint, 2> pts;
    std::array<float  , 1> wts;

    constexpr ConicTo(SkPoint p0, SkPoint p1, float w) : pts{p0, p1}, wts{w} {}
};

template <typename... Verbs>
struct SkSTPathRaw {
    constexpr SkSTPathRaw(Verbs... verbs) {
        size_t vidx = 0, pidx = 0, widx = 0;

        ([&]() {
            fVerbStorage[vidx++] = verbs.kVerb;
            for (const auto& pt : verbs.pts) { *(fPointStorage.data() + pidx++) = pt; }
            for (const auto& w  : verbs.wts) { *(fConicStorage.data() + widx++) = w;  }
        }(), ...);
    }

    constexpr operator SkPathRaw() const {
        return {
            fPointStorage,
            fVerbStorage,
            fConicStorage,
            SkRect::MakeEmpty(),      // TODO
            SkPathFillType::kWinding, // TODO
            false,                    // TODO
        };
    }

private:
    static constexpr size_t CountPoints() {
        size_t count = 0;

        ([&]() { count += sizeof(Verbs::pts) / sizeof(SkPoint); }(), ...);

        return count;
    }

    static constexpr size_t CountWeights() {
        size_t count = 0;

        ([&]() { count += sizeof(Verbs::wts) / sizeof(float); }(), ...);

        return count;
    }

    std::array<SkPathVerb, sizeof...(Verbs)> fVerbStorage  = {};
    std::array<SkPoint, CountPoints()>       fPointStorage = {};
    std::array<float, CountWeights()>        fConicStorage = {};
};

#endif
