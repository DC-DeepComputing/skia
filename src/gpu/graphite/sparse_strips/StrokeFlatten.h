/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_StrokeFlatten_DEFINED
#define skgpu_graphite_sparse_strips_StrokeFlatten_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/private/SkAssert.h"
#include "include/private/SkAttributes.h"
#include "include/private/SkMalloc.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsTypes.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

class SkPath;
class SkStrokeRec;

namespace skgpu::graphite {

class Polyline;

/*
 * StrokeFlatten fuses stroking and flattening: it converts a stroked path directly into the
 * flattened outline (Polyline) consumed by the sparse strips rasterizer. It replaces the two-step
 * pipeline of SkStroke (path -> SkPath of offset quads) followed by Flatten (SkPath -> Polyline).
 *
 * 1. Exact offsets:
 *    Every emitted vertex lies exactly on the true offset curve, P(t) +/- r * N(t), with P and N
 *    evaluated on the original curve. Only *where* we sample is approximated. In particular, the
 *    quads that Flatten lowers cubics into are never evaluated for output: their tangents differ
 *    from the cubic's, and the offset magnifies that difference by r.
 *
 * 2. Offset-aware sample distribution:
 *    For a short segment, the chord error of an offset curve is e_center + e_radial: the
 *    centerline's own chord error plus r * dTheta^2 / 8 from the rotation of the normal. The
 *    optimal sample density is therefore sqrt(|k| * (1 + r * |k|)) per unit arc length (k is the
 *    curvature), i.e. the parametric and radial requirements add in quadrature. We reuse Flatten's
 *    parabola-integral distribution and scale each quad's curvature integral by
 *    sqrt(1 + r * kMax). Quads are split at fixed normalized angles (every 15 degrees) when
 *    r * kMax is large, which keeps that per-piece estimate tight. Samples are distributed jointly
 *    over runs of G1 continuous cubics (the cubics of a conic, or the pieces of a chopped cubic),
 *    so no samples are forced at their seams.
 *
 * 3. Robustness:
 *    Cubics are chopped at inflections and 180-degree rotation points (FindCubicConvex180Chops,
 *    as in the GPU tessellating stroker). Cusps emit a round dot, matching SkStroke. The quads that
 *    steer the distribution can smooth away sub-tolerance near-cusps, and their curvature only
 *    approximates the curve's. A post-check on the exact normals of consecutive samples therefore
 *    subdivides the intervals whose rotation exceeds the radial limit 2 * acos(1 - tol / r).
 *
 * 4. Output topology and fill rule:
 *    Per contour, the left offset is emitted forward and the right offset in reverse. Open
 *    contours become one closed loop that includes the caps; closed contours become two closed
 *    loops. Inner joins go through the pivot point (SkStroke's HandleInnerJoin), except between
 *    two lines long enough for their inner offsets to intersect. No overlap resolution is
 *    needed: the outline must be rasterized with the nonzero (winding) fill rule
 *    (kInverseWinding for inverse fills).
 *
 * 5. Performance:
 *    With FlattenMode::kSimd, the hot loops run 8 (samples, lines) or 4 (quads) wide:
 *    - The quads that steer a run's distribution are mapped onto the parabola 4 at a time.
 *    - The samples of a run are evaluated in a separate pass that writes both offsets straight
 *      into the output chains, followed by a vectorized rotation check.
 *    - Runs of consecutive lines are batched: their tangents, lengths and join terms (the only
 *      sqrt and divisions) are computed up front, leaving branches and stores per join.
 *    - Circular arcs (circles, round rects, arcTo) are sampled at uniform angles, each sample
 *      evaluated independently with short polynomials.
 *    Scratch storage uses Buffer, whose push_back() inlines, and the outline is written straight
 *    into spare capacity of the Polyline.
 *
 * 6. Culling:
 *    When the stroke is partially outside the clip, a curve whose control points, outset by the
 *    stroke's reach (SkStrokeRec::getInflationRadius()), lie entirely beyond one edge of the clip
 *    is stroked as its chord, joined like a curve. The chord's stroke, and the joins and caps at
 *    its ends, stay within that outset hull, so the outline only changes in a half-plane outside
 *    the clip: by loops that wind zero times around every point of the clip.
 *
 * NOTE (weak correctness, matches SkStroke):
 *    This outline is only "weakly correct", exactly like SkStroke. Where the stroke radius exceeds
 *    the radius of curvature on the inner side of a curve (r * k > 1), the inner offset folds into
 *    a swallowtail whose winding can cancel to zero, leaving small holes inside the stroke. We
 *    intentionally match SkStroke (CPU raster and the previous SkStroke + Flatten sparse strips
 *    path) rather than the GPU tessellated strokes, which have union semantics and are strongly
 *    correct. The fold is detectable for free per distribution piece (r * kMax > 1, see
 *    addQuadPieces()) should this ever need fixing: either via Nehab 2020 ("Converting stroked
 *    primitives to filled primitives": evolutes plus connecting segments) or by routing those
 *    strokes to another renderer.
 *
 * NOTE (shared sampling):
 *    Both offset sides share the same samples, i.e. one curve evaluation per sample, with the
 *    count chosen for the outer side. The inner side needs fewer samples on thick strokes, so
 *    sampling the sides separately would save ~3-15% of edges for thick strokes at ~2x the
 *    curve evaluations.
 */
class StrokeFlatten {
public:
    // Device space tolerance, matches Flatten::kQuadErrTolerance.
    static constexpr float kTolerance = 0.25f;

    StrokeFlatten();

    // Appends the outline of 'path', stroked with 'stroke' in local space and transformed by
    // 'ctm', to 'polyline'. Returns false, leaving 'polyline' untouched, if the stroke can't be
    // handled: non-stroke styles (fill, hairline, stroke-and-fill), perspective, or non-finite
    // input. Returns true with no output if the stroke is entirely outside 'clip'.
    template <FlattenMode kMode = FlattenMode::kSimd>
    bool processStroke(const SkPath& path,
                       const SkMatrix& ctm,
                       const SkStrokeRec& stroke,
                       const SkIRect& clip,
                       Polyline* polyline) {
        return this->processStrokeImpl(
                path, ctm, stroke, clip, polyline, kMode == FlattenMode::kSimd);
    }

private:
    // The most cubics in a run of G1 continuous curves: a conic splits into up to 16.
    static constexpr int kMaxRunCubics = 16;
    // The number of fixed normalized angles that quads are split at, see addQuadPieces().
    static constexpr int kNumSplits = 11;

    // A growable array of trivially copyable elements for the scratch storage. Unlike SkTDArray,
    // only growing is out of line, which matters for the per-sample and per-join appends.
    template <typename T> class Buffer {
        static_assert(std::is_trivially_copyable_v<T>);

    public:
        Buffer() = default;
        ~Buffer() { sk_free(fData); }
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        int size() const { return fSize; }
        bool empty() const { return fSize == 0; }
        T* data() { return fData; }
        const T* data() const { return fData; }
        T& operator[](int i) {
            SkASSERT(i >= 0 && i < fSize);
            return fData[i];
        }
        const T& operator[](int i) const {
            SkASSERT(i >= 0 && i < fSize);
            return fData[i];
        }
        T& back() {
            SkASSERT(fSize > 0);
            return fData[fSize - 1];
        }

        void clear() { fSize = 0; }
        // Ensures room for 'minCapacity' elements in total.
        void reserve(int minCapacity) {
            if (minCapacity > fCapacity) {
                this->grow(minCapacity);
            }
        }
        // Growing leaves the new elements uninitialized.
        void resize(int newSize) {
            SkASSERT(newSize >= 0);
            this->reserve(newSize);
            fSize = newSize;
        }
        // Takes 'value' by value, so pushing an element of this buffer is safe.
        SK_ALWAYS_INLINE void push_back(T value) {
            if (fSize == fCapacity) {
                this->grow(fSize + 1);
            }
            fData[fSize++] = value;
        }
        void swap(Buffer& other) {
            std::swap(fData, other.fData);
            std::swap(fSize, other.fSize);
            std::swap(fCapacity, other.fCapacity);
        }

    private:
        // The largest capacity whose size in bytes fits in a size_t.
        static constexpr int kMaxCapacity = static_cast<int>(std::min(
                static_cast<size_t>(std::numeric_limits<int>::max()), SIZE_MAX / sizeof(T)));

        // Grows to at least 'minCapacity', doubling the capacity (at least 32). Allocation
        // failures abort in sk_realloc_throw().
        SK_NEVER_INLINE void grow(int minCapacity) {
            SkASSERT_RELEASE(minCapacity >= 0 && minCapacity <= kMaxCapacity);
            const int doubled = fCapacity <= kMaxCapacity / 2 ? 2 * fCapacity : kMaxCapacity;
            const int newCapacity = std::max({minCapacity, doubled, 32});
            fData = static_cast<T*>(
                    sk_realloc_throw(fData, sizeof(T) * static_cast<size_t>(newCapacity)));
            fCapacity = newCapacity;
        }

        T* fData = nullptr;
        int fSize = 0;
        int fCapacity = 0;
    };

    // A sub-range of one of the quads that steer the sample distribution of a run.
    struct Piece {
        float fA0;     // parabola integral at the start of the piece
        float fSlope;  // change of the parabola integral per unit of weight
        float fAlpha;  // curve t = fAlpha + fBeta * approx_parabola_inv_integral(a)
        float fBeta;
        float fWeight;  // share of the samples: curvature integral * sqrt(1 + r * kMax)
        int fCubic;     // index of the piece's cubic within the run
    };

    // A quad that approximates the cubic fCubic of a run over [fT0, fT1].
    struct Quad {
        SkPoint fP0;
        SkPoint fP1;
        SkPoint fP2;
        float fT0;
        float fT1;
        int fCubic;
    };

    // A quad mapped onto the normalized parabola y = x^2, x in [fX0, fX2], see addQuadPieces().
    struct QuadMap {
        float fX0;
        float fT0;
        float fTScale;  // dt / dx
        float fSqrtScale;
        float fKappaScale;   // curvature at x is fKappaScale / (1 + 4 x^2)^1.5
        float fVertexScale;  // Levien's adjustment for pieces with the vertex, < 0 until needed
        int fCubic;
    };

    // A sample of the slow path, see strokeRunSlow().
    struct Sample {
        SkPoint fPt;
        SkVector fNormal;  // unit length
        float fT;
        int fCubic;
    };

    bool processStrokeImpl(const SkPath& path,
                           const SkMatrix& ctm,
                           const SkStrokeRec& stroke,
                           const SkIRect& clip,
                           Polyline* polyline,
                           bool simd);
    void setup(float radius,
               float tolerance,
               SkPaint::Join joinType,
               SkPaint::Cap capType,
               float miterLimit);

    // Contour state machine.
    void resetContour();
    void beginContour(SkPoint pt);
    void finishContour(bool closed);
    void lineTo(SkPoint pt, bool forceRoundJoin = false);
    // Strokes the chord of a culled curve: like a line, but joined like a curve, so that its joins
    // stay within the stroke's reach of its end points (see "Culling" above).
    void chordTo(SkPoint pt);
    // Strokes the pending lines of fLinePts, with their tangents and joins computed 8 wide.
    void flushLines() {
        if (!fLinePts.empty()) {
            this->flushLinesImpl();
        }
    }
    void flushLinesImpl();
    void quadTo(const SkPoint pts[3]);
    void conicTo(const SkPoint pts[3], float weight);
    // Strokes a conic that is a circular arc (circles, round rects, arcs) analytically, with
    // uniformly spaced samples. Returns false, doing nothing, if the conic isn't circular.
    bool strokeCircularArc(const SkPoint pts[3], float weight);
    void cubicTo(const SkPoint pts[4]);
    void segmentStart(SkVector unitTangent, bool isLine, float length);
    void join(SkVector u0,
              SkVector u1,
              bool prevIsLine,
              bool currIsLine,
              float prevLength,
              float currLength,
              bool closing);
    // join() with d = dot(u0, u1), c = cross(u0, u1) and invOnePlusD = 1 / (1 + d) precomputed.
    void joinTerms(SkVector u0,
                   SkVector u1,
                   float d,
                   float c,
                   float invOnePlusD,
                   bool prevIsLine,
                   bool currIsLine,
                   float prevLength,
                   float currLength,
                   bool closing);

    // Curves. A run is a sequence of 'count' G1 continuous cubics, each convex and rotating at
    // most 180 degrees. Cubic j of the run is cubics[j * stride .. j * stride + 3].
    void strokeRun(const SkPoint* cubics, int count, int stride);
    // strokeRun() for a single quad, which steers its own samples: no lowering to quads needed.
    void strokeQuad(const SkPoint pts[3]);
    // The second half of a run, once its samples are generated: evaluates them, checks their
    // rotation, falls back to strokeRunSlow() if needed, and emits the run's end offsets.
    void finishRun(const SkPoint* cubics,
                   int count,
                   int stride,
                   int numInterior,
                   SkVector u0,
                   SkVector u1);
    // Lowers the run's cubics to quads (fQuads) and turns those into pieces (fPieces).
    void addRunPieces(const SkPoint* cubics, int count, int stride);
    void addQuadPieces(const Quad& quad);
    // addQuadPieces() for fQuads[first .. first + lanes), lanes <= 4.
    void addQuadPiecesSimd(int first, int lanes);
    // Splits a mapped quad at fixed angles (or not at all, if 'split' is false).
    void addMappedQuadPieces(QuadMap* map,
                             float x2,
                             float i0,
                             float i2,
                             float u0,
                             float u2,
                             float kNearest,
                             bool split);
    void pushPiece(
            QuadMap* map, float xa, float xb, float ia, float ib, float ua, float ub, float kMax);
    // Fills the per-sample arrays and fCubicSampleEnd, returns the number of interior samples.
    int generateSamples(int count, float sumWeights);
    // Evaluate interior samples [begin, end) of one cubic, writing their offsets to left[k] and
    // right[k], and their normals to fNormalX/Y[k + 1]. evaluateSimd() returns false if a
    // derivative was too short to normalize.
    void evaluateScalar(const SkPoint pts[4], int begin, int end, SkPoint* left, SkPoint* right);
    bool evaluateSimd(const SkPoint pts[4], int begin, int end, SkPoint* left, SkPoint* right);
    void strokeRunSlow(const SkPoint* cubics, int count, int stride, SkVector u0, SkVector u1);
    // For a run that failed the rotation check: keeps its evaluated samples (written past the
    // ends of fLeft and fRight) and subdivides only the intervals that rotate too much.
    void refineRun(const SkPoint* cubics,
                   int count,
                   int stride,
                   int leftBase,
                   int rightBase,
                   SkVector u0,
                   SkVector u1);
    // Subdivides the intervals between consecutive fSamples that rotate more than fRadialDelta.
    void refineRotation(const SkPoint* cubics, int stride);
    // Appends samples strictly between a and b, which lie on the cubic 'pts', to fRefined until
    // no interval rotates more than fRadialDelta.
    void subdivide(const SkPoint pts[4], const Sample& a, const Sample& b, int depth);

    // Output.
    // Contours are written straight into spare capacity of the Polyline, [fOut, fOutEnd), which
    // reserveOutput() provides. commitOutput() appends the written points [fOutStart, fOut) to
    // the Polyline, mapping them by fCTM if fMapOutput.
    SkPoint* reserveOutput(int maxCount) {
        return fOutEnd - fOut >= maxCount ? fOut : this->growOutput(maxCount);
    }
    SkPoint* growOutput(int maxCount);
    void commitOutput();
    void appendArc(
            Buffer<SkPoint>* chain, SkPoint center, SkVector from, SkVector to, float dir) const;
    void appendCap(Buffer<SkPoint>* chain, SkPoint pivot, SkVector unitTangent, bool atEnd) const;
    void appendDot(SkPoint center, bool round);

    // Parabola integrals, inverse integrals and cos^3 of the split angles, see addQuadPieces().
    float fSplitIntegral[kNumSplits];
    float fSplitInvIntegral[kNumSplits];
    float fSplitCos3[kNumSplits];

    // Per-draw parameters, see setup().
    float fRadius = 0.0f;
    float fTolerance = 0.0f;
    float fSqrtTolerance = 0.0f;
    float fCountScale = 0.0f;   // 0.5 / sqrt(tolerance): Flatten's segment count scale
    float fInvCubicErr = 0.0f;  // cubic -> quad count threshold
    float fInvConicTol = 0.0f;  // conic -> cubic bisection threshold
    float fRadialDelta = 0.0f;  // max normal rotation per segment: 2 * acos(1 - tol / r)
    float fCosDelta = 1.0f;
    float fSinDelta = 0.0f;
    float fMinDot = -2.0f;         // fCosDelta if the rotation is limited, else always passes
    float fMiterThreshold = 0.0f;  // miter iff 1 + dot(n0, n1) >= 2 / miterLimit^2
    int fMaxArcSteps = 0;
    int fCapSegments = 1;           // segments in a round cap (semicircle)
    bool fRotationLimited = false;  // false when r <= tol: any rotation stays within tolerance
    bool fMapOutput = false;        // stroke in local space and map the output by fCTM
    bool fSimd = true;
    // Curves whose control points lie entirely beyond one edge of fCullRect (the clip, outset by
    // the stroke's reach, in the space curves are stroked in) are stroked as chords.
    bool fCull = false;
    SkRect fCullRect = SkRect::MakeEmpty();
    SkPaint::Join fJoin = SkPaint::kMiter_Join;
    SkPaint::Join fActiveJoin = SkPaint::kMiter_Join;
    SkPaint::Cap fCap = SkPaint::kButt_Cap;
    SkMatrix fCTM;

    // Contour state.
    SkPoint fFirstPt = {0.0f, 0.0f};
    SkPoint fPrevPt = {0.0f, 0.0f};
    SkVector fFirstU = {1.0f, 0.0f};
    SkVector fPrevU = {1.0f, 0.0f};
    float fFirstLength = 0.0f;
    float fPrevLength = 0.0f;
    int fSegmentCount = 0;
    bool fFirstIsLine = false;
    bool fPrevIsLine = false;
    bool fSawDegenerate = false;
    bool fContourStarted = false;

    // Scratch storage, reused across calls to avoid allocations.
    Buffer<SkPoint> fLeft;
    Buffer<SkPoint> fRight;  // generated forward, emitted reversed
    Buffer<SkPoint> fScratch;
    // Pending lines: fLinePts[0] is their start (fPrevPt), then one end point per line.
    Buffer<SkPoint> fLinePts;
    // Per pending line k (index k, padded for SIMD): unit tangent and length, and the join terms
    // with line k - 1: dot and cross of the tangents, and 1 / (1 + dot).
    Buffer<float> fLineUX;
    Buffer<float> fLineUY;
    Buffer<float> fLineLength;
    Buffer<float> fLineDot;
    Buffer<float> fLineCross;
    Buffer<float> fLineInvOnePlusDot;
    Buffer<Quad> fQuads;
    Buffer<Piece> fPieces;
    // Per interior sample of a run (SoA, padded for SIMD): the parabola integral 'a' and the
    // affine map of its piece, t = alpha + beta * approx_parabola_inv_integral(a).
    Buffer<float> fSampleA;
    Buffer<float> fSampleAlpha;
    Buffer<float> fSampleBeta;
    // Unit normals of a run: its start, its interior samples, and its end (then padding).
    Buffer<float> fNormalX;
    Buffer<float> fNormalY;
    Buffer<Sample> fSamples;
    Buffer<Sample> fRefined;
    Buffer<float> fCapCos;
    Buffer<float> fCapSin;
    int fCubicSampleEnd[kMaxRunCubics] = {};  // end of each cubic's interior samples
    Polyline* fPolyline = nullptr;
    // Output session, see reserveOutput().
    SkPoint* fOutStart = nullptr;
    SkPoint* fOut = nullptr;
    SkPoint* fOutEnd = nullptr;
    SkPoint fOutLast = {0.0f, 0.0f};  // the last point written, for de-duplication
    int fOutHint = 0;                 // the size of the next reservation
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_StrokeFlatten_DEFINED
