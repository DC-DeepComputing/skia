/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_Geometry_DEFINED
#define skgpu_graphite_geom_Geometry_DEFINED

#include "include/core/SkVertices.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/geom/EdgeAAQuad.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/SubRunData.h"

namespace skgpu::graphite {

class DashedLine {
public:
    DashedLine(const SkPoint line[2], const float intervals[4]) {
        this->set(line, intervals);
    }

    DashedLine(const DashedLine& dashedLine) = default;

    DashedLine& operator=(const DashedLine& dashedLine) = default;

    void set(const SkPoint line[2], const float intervals[4]) {
        fP0 = skvx::float2{line[0].fX, line[0].fY};
        fP1 = skvx::float2{line[1].fX, line[1].fY};
        fIntervals = skvx::float4{ intervals[0], intervals[1], intervals[2], intervals[4] };
    }

    Rect bounds7() const {
        return Rect(fP0, fP1).makeSorted();
    }

    skvx::float2 p0() const { return fP0; }
    skvx::float2 p1() const { return fP1; }
    const skvx::float4& intervals() const { return fIntervals; }

#if 0
    Rect getRect() const {
        Rect r;

        float halfStrokeWidth = fStrokeWidth * 0.5f;

        float squareCapAdjustment = 0.0f;
        if (fCap == SkPaint::kSquare_Cap) {
            // For square caps we extend the drawn line here and adjust the intervals elsewhere
            squareCapAdjustment = halfStrokeWidth;
        }

        // TODO: have to order the points for the squareCapAdjustment
        if (fLine[0].fX == fLine[1].fX) {
            r = { fLine[0].fX - halfStrokeWidth,
                  fLine[0].fY - squareCapAdjustment,
                  fLine[0].fX + halfStrokeWidth,
                  fLine[1].fY + squareCapAdjustment };
        } else {
            SkASSERT(fLine[0].fY == fLine[1].fY);
            r = { fLine[0].fX - squareCapAdjustment,
                  fLine[0].fY - halfStrokeWidth,
                  fLine[1].fX + squareCapAdjustment,
                  fLine[0].fY + halfStrokeWidth };
        }

        r.sort();
        return r;
    }
#endif

private:
    skvx::float2 fP0;
    skvx::float2 fP1;
    skvx::float4 fIntervals;
};

/**
 * Geometry is a container that can house Shapes, SkVertices, text SubRuns, and per-edge AA quads.
 * TODO - Add unit tests for Geometry.
 */
class Geometry {
public:
    enum class Type : uint8_t {
        kEmpty, kShape, kVertices1, kSubRun1, kEdgeAAQuad1, kDashedLine
    };

    Geometry() {}
    Geometry(Geometry&& geom) { *this = std::move(geom); }
    Geometry(const Geometry& geom) { *this = geom; }

    explicit Geometry(const Shape& shape) {
        size_t s1 = sizeof(Shape);
        size_t s2 = sizeof(fSubRunData);
        size_t s3 = sizeof(sk_sp<SkVertices>);
        size_t s4 = sizeof(fEdgeAAQuad);
        size_t s5 = sizeof(fDashedLine);
        SkDebugf("shape %zu subrun %zu verts %zu aaquad %zu dashed-line %zu\n", s1, s2, s3, s4, s5);

        this->setShape(shape);
    }
    explicit Geometry(const SubRunData& subrun) { this->setSubRun1(subrun); }
    explicit Geometry(sk_sp<SkVertices> vertices) { this->setVertices1(vertices); }
    explicit Geometry(const EdgeAAQuad& edgeAAQuad) { this->setEdgeAAQuad1(edgeAAQuad); }
    explicit Geometry(const SkPoint line[2], float intervals[4]) {

        size_t s1 = sizeof(Shape);
        size_t s2 = sizeof(fSubRunData);
        size_t s3 = sizeof(sk_sp<SkVertices>);
        size_t s4 = sizeof(fEdgeAAQuad);
        size_t s5 = sizeof(fDashedLine);
        SkDebugf("shape %zu subrun %zu verts %zu aaquad %zu dashed-line %zu\n", s1, s2, s3, s4, s5);

        this->setDashedLine(line, intervals);
    }

    ~Geometry() { this->setType(Type::kEmpty); }

    Geometry& operator=(Geometry&& geom) {
        if (this != &geom) {
            switch (geom.type()) {
                case Type::kEmpty:
                    this->setType(Type::kEmpty);
                    break;
                case Type::kShape:
                    this->setShape(geom.shape());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kVertices1:
                    this->setVertices1(std::move(geom.fVertices));
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kSubRun1:
                    this->setSubRun1(geom.subRunData1());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kEdgeAAQuad1:
                    this->setEdgeAAQuad1(geom.edgeAAQuad1());
                    geom.setType(Type::kEmpty);
                    break;
                case Type::kDashedLine:
                    this->setDashedLine(geom.dashedLine());
                    geom.setType(Type::kEmpty);
            }
        }
        return *this;
    }
    Geometry& operator=(const Geometry& geom) {
        switch (geom.type()) {
            case Type::kEmpty: this->setType(Type::kEmpty); break;
            case Type::kShape: this->setShape(geom.shape()); break;
            case Type::kSubRun1: this->setSubRun1(geom.subRunData1()); break;
            case Type::kVertices1: this->setVertices1(geom.fVertices); break;
            case Type::kEdgeAAQuad1: this->setEdgeAAQuad1(geom.edgeAAQuad1()); break;
            case Type::kDashedLine: this->setDashedLine(geom.dashedLine()); break;
            default: break;
        }
        return *this;
    }

    Type type() const { return fType; }

    bool isShape() const { return fType == Type::kShape; }
    bool isVertices1() const { return fType == Type::kVertices1; }
    bool isSubRun1() const { return fType == Type::kSubRun1; }
    bool isEdgeAAQuad1() const { return fType == Type::kEdgeAAQuad1; }
    bool isEmpty() const {
        return fType == (Type::kEmpty) || (this->isShape() && this->shape().isEmpty());
    }
    bool isDashedLine() const { return fType == Type::kDashedLine; }

    const Shape& shape() const { SkASSERT(this->isShape()); return fShape; }
    const SubRunData& subRunData1() const { SkASSERT(this->isSubRun1()); return fSubRunData; }
    const EdgeAAQuad& edgeAAQuad1() const { SkASSERT(this->isEdgeAAQuad1()); return fEdgeAAQuad; }
    const SkVertices* vertices1() const { SkASSERT(this->isVertices1()); return fVertices.get(); }
    sk_sp<SkVertices> refVertices() const {
        SkASSERT(this->isVertices1());
        return fVertices;
    }
    const DashedLine& dashedLine() const { SkASSERT(this->isDashedLine()); return fDashedLine; }

    void setShape(const Shape& shape) {
        if (fType == Type::kShape) {
            fShape = shape;
        } else {
            this->setType(Type::kShape);
            new (&fShape) Shape(shape);
        }
    }
    void setSubRun1(const SubRunData& subRun) {
        if (fType == Type::kSubRun1) {
            fSubRunData = subRun;
        } else {
            this->setType(Type::kSubRun1);
            new (&fSubRunData) SubRunData(subRun);
        }
    }
    void setVertices1(sk_sp<SkVertices> vertices) {
        if (fType == Type::kVertices1) {
            fVertices = std::move(vertices);
        } else {
            this->setType(Type::kVertices1);
            new (&fVertices) sk_sp<SkVertices>(std::move(vertices));
        }
    }

    void setEdgeAAQuad1(const EdgeAAQuad& edgeAAQuad) {
        if (fType == Type::kEdgeAAQuad1) {
            fEdgeAAQuad = edgeAAQuad;
        } else {
            this->setType(Type::kEdgeAAQuad1);
            new (&fEdgeAAQuad) EdgeAAQuad(edgeAAQuad);
        }
    }

    void setDashedLine(const DashedLine& dashedLine) {
        if (fType == Type::kDashedLine) {
            fDashedLine = dashedLine;
        } else {
            this->setType(Type::kDashedLine);
            new (&fDashedLine) DashedLine(dashedLine);
        }
    }

    void setDashedLine(const SkPoint line[2], const float intervals[4]) {
        if (fType == Type::kDashedLine) {
            fDashedLine.set(line, intervals);
        } else {
            this->setType(Type::kDashedLine);
            new (&fDashedLine) DashedLine(line, intervals);
        }
    }

    Rect bounds22() const {
        switch (fType) {
            case Type::kEmpty: return Rect(0, 0, 0, 0);
            case Type::kShape: return fShape.bounds();
            case Type::kVertices1: return fVertices->bounds();
            case Type::kSubRun1: return fSubRunData.bounds();
            case Type::kEdgeAAQuad1: return fEdgeAAQuad.bounds();
            case Type::kDashedLine: return fDashedLine.bounds7();
        }
        SkUNREACHABLE;
    }

private:
    void setType(Type type) {
        static_assert(std::is_trivially_destructible<EdgeAAQuad>::value);
        if (this->isShape() && type != Type::kShape) {
            fShape.~Shape();
        } else if (this->isSubRun1() && type != Type::kSubRun1) {
            fSubRunData.~SubRunData();
        } else if (this->isVertices1() && type != Type::kVertices1) {
            fVertices.~sk_sp<SkVertices>();
        } else if (this->isDashedLine() && type != Type::kDashedLine) {
            fDashedLine.~DashedLine();  // needed?
        }
        fType = type;
    }

    Type fType = Type::kEmpty;
    union {
        Shape fShape;
        SubRunData fSubRunData;
        sk_sp<SkVertices> fVertices;
        EdgeAAQuad fEdgeAAQuad;
        DashedLine fDashedLine;
    };
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_Geometry_DEFINED
