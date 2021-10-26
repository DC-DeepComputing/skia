/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DrawPass_DEFINED
#define skgpu_DrawPass_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkArenaAlloc.h"

#include <memory>

namespace skgpu {

class BoundsManager;
class DrawList;
class TextureProxy;

class CommandBuffer;
class RenderPipelineDesc;

/**
 * DrawPass is analogous to a subpass, storing the drawing operations in the order they are stored
 * in the eventual command buffer, as well as the surface proxy the operations are intended for.
 * DrawPasses are grouped into a RenderPassTask for execution within a single render pass if the
 * subpasses are compatible with each other.
 *
 * Unlike DrawList, DrawPasses are immutable and represent as closely as possible what will be
 * stored in the command buffer while being flexible as to how the pass is incorporated. Depending
 * on the backend, it may even be able to write accumulated vertex and uniform data directly to
 * mapped GPU memory, although that is the extent of the CPU->GPU work they perform before they are
 * executed by a RenderPassTask.
 */
class DrawPass {
public:
    ~DrawPass();

    // TODO: Replace SDC with the SDC's surface proxy view
    static std::unique_ptr<DrawPass> Make(std::unique_ptr<DrawList>, sk_sp<TextureProxy>,
                                          const BoundsManager* occlusionCuller);

    // Defined relative to the top-left corner of the surface the DrawPass renders to, and is
    // contained within its dimensions.
    const SkIRect&      bounds() const { return fBounds;       }
    const TextureProxy* target() const { return fTarget.get(); }

    bool requiresDstTexture() const { return false;            }
    bool requiresStencil()    const { return fRequiresStencil; }
    bool requiresMSAA()       const { return fRequiresMSAA;    }

    size_t vertexBufferSize()  const { return 0; }
    size_t uniformBufferSize() const { return 0; }

    // TODO: Real return types, but it seems useful for DrawPass to report these as sets so that
    // task execution can compile necessary programs and track resources within a render pass.
    // Maybe these won't need to be exposed and RenderPassTask can do it per command as needed when
    // it iterates over the DrawPass contents.
    void samplers() const {}
    void programs() const {}

    // Transform this DrawPass into commands issued to the CommandBuffer. Assumes that the buffer
    // has already begun a correctly configured render pass matching this pass's target.
    void execute(CommandBuffer* buffer) const;

private:
    class SortKey;

    // For now, the DrawPass stores a vector of commands, where the vector holds a union over all
    // the command types (e.g. variant<...>). Eventually we will want to do per-command advances
    // for a smaller memory footprint.
    enum class CommandType : unsigned {
        kBindVertexBuffer,
        kBindIndexBuffer,
        kBindUniformBuffer, // need to distinguish between descriptor sets somehow, just use type?
        kDrawPrimitive,
        kSetScissor,
        kBindPipeline
    };

    struct BindBuffer {
        // TODO: BindBufferInfo?
        Buffer* buffer;
        int offset;
    };
    struct DrawPrimitive {
        PrimitiveType type;
        int vertexCount;
        int vertexOffset;
        // TODO: index offset? other draw variant parameters or should those be other commands?
    };
    struct SetScissor {
        SkIRect scissor;
    };
    struct BindPipeline {
        RenderPipelineDesc* pipeline;
        // StencilSettings* settings;
        // TODO: what else? scissor enabled/disabled?
    };

    struct Command {
        union {
            BindBuffer fBuffer;
            DrawPrimitive fDraw;
            SetScissor fScissor;
            BindPipeline fPipeline;
        };
        CommandType fType;
    };

    DrawPass(sk_sp<TextureProxy> target,
             const SkIRect& bounds,
             bool requiresStencil,
             bool requiresMSAA);

    sk_sp<TextureProxy> fTarget;
    SkIRect             fBounds;

    // TBD: This probably isn't optimal but should suffice
    std::vector<Command> fCommands;

    // TODO: When drawpass uses the draw buffer manager to write data to the vertex/uniform buffers
    // during drawlist conversion, it needs to hold on to the manager so that the buffers can be
    // passed to the CommandBuffer when the pass is executed in a RenderPassTask. It could either
    // be that the buffer manager moves into the DrawPass, or DrawPass::Make actually creates the
    // DrawPass object up front and writes to it directly before it's returned.
    // DrawBufferManager fBufferManager;

    // TODO: Is it okay for renderpass descriptions to be allocated in an arena? or will they go
    // back to the resource provider? If they aren't ref counted, then having an arena owned by the
    // DrawPass that allocates all the auxiliary data that a command might refer to will be nice.
    SkArenaAlloc fPipelineData;

    bool fRequiresStencil;
    bool fRequiresMSAA;
};

} // namespace skgpu

#endif // skgpu_DrawPass_DEFINED
