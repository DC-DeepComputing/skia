/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/TessellateWedgesComputeStep.h"

#include "include/private/SkVx.h"

#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/tessellate/MidpointContourParser.h"

namespace skgpu::graphite {
namespace {

using float2 = skvx::float2;
using float4 = skvx::float4;

// This does not return b when t==1, but it otherwise seems to get better precision than
// "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
// The responsibility falls on the caller to check that t != 1 before calling.
static float4 mix(float4 a, float4 b, float4 T) {
    SkASSERT(all((0 <= T) & (T < 1)));
    return (b - a)*T + a;
}

struct Verb {
    float4 p01;
    float4 p23;
};

static void write_line(float2 p0, float2 p1, Verb* outVerb) {
    outVerb->p01 = p0.xyxy();
    outVerb->p23 = p1.xyxy();
}

static void write_cubic_patch(float2 p0, float2 p1, float2 p2, float2 p3, Verb* outVerb) {
    outVerb->p01 = float4(p0, p1);
    outVerb->p23 = float4(p2, p3);
}

static void write_quad_patch(float2 p0, float2 p1, float2 p2, Verb* outVerb) {
    float4 p1p2 = mix(float4(p0, p2), p1.xyxy(), 2.f/3.f);
    write_cubic_patch(p0, p1p2.lo, p1p2.hi, p2, outVerb);
}

}  // namespace

TessellateWedgesComputeStep::TessellateWedgesComputeStep()
    : ComputeStep(/*handlesInputGeometry=*/true) {}

const char* TessellateWedgesComputeStep::computeSkSL() const {
    // TODO: SSBO index as a uniform. 0 happens to work for my slide example but it won't in the
    // general case.
    return R"(
        const float kDegree = 3;
        const float kPrecision = 4; // Must match skgpu::tess::kPrecision
        const float kLengthTerm = (kDegree * (kDegree - 1) / 8.0) * kPrecision;
        const float kLengthTermPow2 = ((kDegree * kDegree) * ((kDegree - 1) * (kDegree - 1)) / 64.0) *
                                       (kPrecision * kPrecision);

        // Returns the length squared of the largest forward difference from Wang's cubic formula.
        float wangs_formula_max_fdiff_p2(float2 p0, float2 p1, float2 p2, float2 p3,
                                                float2x2 matrix) {
            float2 d0 = matrix * (fma(float2(-2), p1, p2) + p0);
            float2 d1 = matrix * (fma(float2(-2), p2, p3) + p1);
            return max(dot(d0,d0), dot(d1,d1));
        }

        float wangs_formula_cubic(float2 p0, float2 p1, float2 p2, float2 p3,
                                         float2x2 matrix) {
            float m = wangs_formula_max_fdiff_p2(p0, p1, p2, p3, matrix);
            return max(ceil(sqrt(kLengthTerm * sqrt(m))), 1.0);
        }

        float wangs_formula_cubic_log2(float2 p0, float2 p1, float2 p2, float2 p3,
                                        float2x2 matrix) {
            float m = wangs_formula_max_fdiff_p2(p0, p1, p2, p3, matrix);
            return ceil(log2(max(kLengthTermPow2 * m, 1.0)) * .25);
        }

        layout(set=0, binding=0) uniform uniforms {
            float4x4 localToDevice;
        };

        struct Verb {
            float4 p01;
            float4 p23;
        };
        layout(set=0, binding=1) readonly buffer inputData {
            float2 fanPoint;
            uint verbCount;
            Verb verbs[];
        };

        layout(set=0, binding=2) buffer globalCounts {
            atomicUint globalVertexCount;
        };

        struct VerbExt {
            uint triangleCount;
            uint vertexOffset;
            uint indexOffset;
        };
        workgroup VerbExt localVerbCounts[16];

        // TODO: If I declare the first member as a float2 instead of two floats, then Metal pads
        // the struct with an extra 4 bytes to match 8 byte alignment for float2.
        //
        // However the vertex shader in VerticesRenderStep expects interleaved float2 and int
        // attributes with a stride that matches the following layout.
        struct VertexAttr {
            float px;
            float py;
            int ssboIndex;
        };
        layout(set=0, binding=3) buffer outputVertices {
            VertexAttr outVertices[];
        };
        layout(set=0, binding=4) buffer outputIndices {
            uint outIndices[];
        };

        // This indirect command layout is identical across Vulkan, Metal, WebGPU, and OpenGL.
        struct DrawIndexedCommand {
            atomicUint indexCount;
            uint instanceCount;
            uint firstIndex;
            int  vertexStart;
            uint baseInstance;
        };
        layout(set=0, binding=5) buffer indirectCommandBuffer {
            DrawIndexedCommand indirectCommands[];
        };

        const float kMaxParametricSegments = 64;

        float2 de_casteljau_cubic(float2 p0, float2 p1, float2 p2, float2 p3, float T) {
            float2 ab = mix(p0, p1, T);
            float2 bc = mix(p1, p2, T);
            float2 cd = mix(p2, p3, T);
            float2 abc = mix(ab, bc, T);
            float2 bcd = mix(bc, cd, T);
            float2 abcd = mix(abc, bcd, T);
            return abcd;
        }

        void main() {
            // Dedicate one thread to initialize the instance count in the draw command. The rest of
            // the command structure is expected to have been initialized to zero before this
            // compute kernel executes. The vertex count and offsets into the vertex buffer are
            // tallied in the segment count stage below.
            if (sk_GlobalInvocationID == uint3(0, 0, 0)) {
                indirectCommands[0].instanceCount = 1;
            }

            uint verbIdx = sk_GlobalInvocationID.x;
            uint localVerbIdx = sk_LocalInvocationID.x;
            uint triIdx = sk_GlobalInvocationID.y;
            float2x2 vectorXForm = float2x2(localToDevice[0].xy, localToDevice[1].xy);

            Verb c;
            float2 p0, p1, p2, p3;

            if (verbIdx < verbCount) {
                c = verbs[verbIdx];
                p0 = c.p01.xy;
                p1 = c.p01.zw;
                p2 = c.p23.xy;
                p3 = c.p23.zw;

                // Initialize counts on one of the per-verb threads.
                if (triIdx == 0) {
                    uint triCount;
                    if (p0 == p1 && p2 == p3) {
                        // This is a line which is drawn as a single triangle around the fan point.
                        triCount = 1;
                    } else {
                        // Calculate a segment count using Wang's Formula. Round it up to the next
                        // power of 2 as this makes triangulating the middle-out topology much
                        // easier.
                        // TODO(armansito): It would be nice if this pattern could be adapted to
                        // arbitrary numbers of segments (e.g. odd). This would remove the need to
                        // chop curves.
                        float wf = wangs_formula_cubic_log2(p0, p1, p2, p3, vectorXForm);
                        wf = pow(2, ceil(wf));

                        // The number of triangles we produce to fill the curve equals wf-1. Omit
                        // the subtraction to account for the fan triangle.
                        triCount = uint(min(wf, kMaxParametricSegments));
                    }

                    localVerbCounts[localVerbIdx].triangleCount = triCount;
                    localVerbCounts[localVerbIdx].indexOffset =
                            atomicAdd(indirectCommands[0].indexCount, 3 * triCount);

                    // The number of vertices computed for each verb equals the triangle count + 2,
                    // accounting for the fan vertex. For example, 16 segments along the curve
                    // constitutes 15 triangles and 17 vertices (including the start and end control
                    // points).
                    localVerbCounts[localVerbIdx].vertexOffset = atomicAdd(globalVertexCount, triCount + 2);
                }
            }

            // Ensure that all threads in the workgroup see the segment count and offset assigned
            // above.
            workgroupBarrier();

            if (verbIdx < verbCount) {
                // Re-load the counts from workgroup memory for this thread.
                uint triCount = localVerbCounts[localVerbIdx].triangleCount;
                uint vertexOffset = localVerbCounts[localVerbIdx].vertexOffset;
                uint indexOffset = localVerbCounts[localVerbIdx].indexOffset;
                if (triIdx < uint(triCount)) {
                    // The vertex indices of the fan point, p0, and p3 occupy the first 3 slots.
                    uint p0Idx = vertexOffset + 1;
                    uint p3Idx = vertexOffset + 2;
                    if (triIdx == 0) {
                        // Triangle 0 is always the fan triangle. We specially handle the vertices
                        // and indices here.
                        outVertices[vertexOffset] = VertexAttr(fanPoint.x, fanPoint.y, 0);
                        outVertices[p0Idx]        = VertexAttr(p0.x, p0.y, 0);
                        outVertices[p3Idx]        = VertexAttr(p3.x, p3.y, 0);

                        outIndices[indexOffset]     = vertexOffset;
                        outIndices[indexOffset + 1] = vertexOffset + 1;
                        outIndices[indexOffset + 2] = vertexOffset + 2;
                    } else {
                        // Every triangle contributes exactly one new vertex to the vertex list. The
                        // other 2 triangles always come from a lower resolve level.
                        //
                        // The vertices are laid out sequentially in the order of resolve level. We
                        // reserve level -1 for the fan point and the control points. For example,
                        // 7 triangles with up to resolve level 2 will be laid out as follows:
                        //
                        //   Buffer idx (-3 for fan):  0   1   2   3   4   5   6   7   8   9
                        //             Resolve level: -1  -1  -1   0   1   1   2   2   2   2
                        //     Triangle idx in level:  -   -   -   0   0   1   0   1   2   3
                        //     Curve-space coord (T):  -   0   1  1/2 1/4 3/4 1/8 3/8 5/8 7/8
                        float resolveLevel = floor(log2(float(triIdx)));
                        float triIdxInResolveLevel = float(triIdx - (1 << uint(resolveLevel)));
                        float T = ldexp(2 * triIdxInResolveLevel + 1, -int(resolveLevel + 1));
                        float2 v = de_casteljau_cubic(p0, p1, p2, p3, T);

                        // The vertex index must account for the 3 fan-triangle vertices.
                        uint vertexIdx = vertexOffset + 2 + triIdx;
                        outVertices[vertexIdx] = VertexAttr(v.x, v.y, 0);

                        uint i0 = vertexIdx, i1, i2;
                        if (resolveLevel == 0) {
                            // At resolve level 0 the "parent" vertices are the outer edges of the
                            // curve.
                            i1 = p3Idx;
                            i2 = p0Idx;
                        } else {
                            // The next vertex is always the apex vertex of the "parent" triangle,
                            // i.e. the triangle from the previous resolve level that this triangle
                            // shares an edge with.
                            uint parentLevel = uint(floor(resolveLevel - 1));
                            float parentIdxInLevel = floor(triIdxInResolveLevel * 0.5);
                            uint parentTriIdx = uint(parentIdxInLevel) + (1 << parentLevel) - 1;

                            // The final vertex can belong to any resolve level. To calculate the
                            // exact index position we first compute its T value (this is T_gp for
                            // "grandparent). Next, we search through each preceding resolve level
                            // until we find the level that T_gp belongs to.
                            //
                            // We do this by converting T_gp into a fraction (using the resolve
                            // level to determine the divisor, e.g. level 0 -> 2, 1 -> 4, 2 -> 8,
                            // etc). Even dividends can belong to multiple levels but odd dividends
                            // always belong to a specific level.
                            float T_parent = ldexp(2 * parentIdxInLevel + 1, -int(parentLevel + 1));
                            float T_gp = 2 * T - T_parent;
                            uint gpTriIdx;
                            int gpLevel = int(parentLevel) - 1;
                            while (gpLevel > -1) {
                                uint dividend = uint(floor(ldexp(T_gp, gpLevel + 1) + .5));
                                if (dividend % 2 != 0) { break; }
                                gpLevel--;
                            }
                            if (gpLevel > -1) {
                                uint gpIdxInLevel = uint(floor(.5 * (ldexp(T_gp, gpLevel + 1) - 1)));
                                gpTriIdx = uint(gpIdxInLevel) + (1 << uint(gpLevel)) - 1 + vertexOffset + 3;
                            } else {
                                gpTriIdx = (T_gp > .5) ? p3Idx : p0Idx;
                            }
                            if (T < T_parent) {
                                i1 = vertexOffset + 3 + parentTriIdx;
                                i2 = gpTriIdx;
                            } else {
                                i1 = gpTriIdx;
                                i2 = vertexOffset + 3 + parentTriIdx;
                            }
                        }

                        uint indexIdx = indexOffset + 3 * triIdx;
                        outIndices[indexIdx]     = i0;
                        outIndices[indexIdx + 1] = i1;
                        outIndices[indexIdx + 2] = i2;
                    }
                }
            }
        }
    )";
}

void TessellateWedgesComputeStep::writeInputGeometry(
        const DrawParams& params, int ssboIndex, void* inputBuffer) const {
    SkPath path = params.geometry().shape().asPath();
    uint8_t* inputBytes = static_cast<uint8_t*>(inputBuffer);
    Verb* verbs = reinterpret_cast<Verb*>(inputBytes + 16);
    uint32_t verbCount = 0;

    tess::MidpointContourParser parser{path};
    while (parser.parseNextContour()) {
        SkPoint lastPoint = {0, 0};
        SkPoint startPoint = {0, 0};
        (*reinterpret_cast<float2*>(inputBytes)) = skvx::bit_pun<float2>(parser.currentMidpoint());
        for (auto [verb, pts, weights] : parser.currentContour()) {
            switch (verb) {
                case SkPathVerb::kMove:
                    startPoint = lastPoint = pts[0];
                    break;
                case SkPathVerb::kLine: {
                    // Unlike curve tessellation, wedges have to handle lines as part of the patch,
                    // effectively forming a single triangle with the fan point.
                    write_line(
                            skvx::bit_pun<float2>(pts[0]), skvx::bit_pun<float2>(pts[1]), verbs);
                    lastPoint = pts[1];
                    verbs++;
                    verbCount++;
                    break;
                }
                case SkPathVerb::kQuad: {
                    write_quad_patch(skvx::bit_pun<float2>(pts[0]),
                                     skvx::bit_pun<float2>(pts[1]),
                                     skvx::bit_pun<float2>(pts[2]),
                                     verbs);
                    lastPoint = pts[2];
                    verbs++;
                    verbCount++;
                    break;
                }
                case SkPathVerb::kCubic: {
                    write_cubic_patch(skvx::bit_pun<float2>(pts[0]),
                                      skvx::bit_pun<float2>(pts[1]),
                                      skvx::bit_pun<float2>(pts[2]),
                                      skvx::bit_pun<float2>(pts[3]),
                                      verbs);
                    lastPoint = pts[3];
                    verbs++;
                    verbCount++;
                    break;
                }
                default:
                    break;
            }
        }

        // Explicitly close the verb with another line segment, which also differs from curve
        // tessellation since that approach's triangle step automatically closes the verb.
        if (lastPoint != startPoint) {
            write_line(
                    skvx::bit_pun<float2>(lastPoint), skvx::bit_pun<float2>(startPoint), verbs);
            verbCount++;
        }
    }
    (*reinterpret_cast<uint32_t*>(inputBytes + 8)) = verbCount;
}

}  // namespace skgpu::graphite
