/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/WebGPUCaptureView.h"

#include "imgui.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <span>
#include <sstream>
#include <unordered_map>

#if defined(SK_VIEWER_WEBGPU_CAPTURE)

#include <webgpu/webgpu_cpp_print.h>
// Dawn's internal headers aren't written for Skia's warning flags.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-template"
#include "src/dawn/replay/CaptureWalker.h"
#include "src/dawn/replay/ReadHead.h"
#pragma clang diagnostic pop

namespace {

using Node = WebGPUCaptureView::Node;
using ObjectInfo = WebGPUCaptureView::ObjectInfo;
namespace schema = dawn::replay::schema;
using dawn::replay::VisitResult;
using dawn::replay::VisitStatus;

std::string Fmt(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
std::string Fmt(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return buffer;
}

template <typename E>
std::string EnumStr(E e) {
    std::ostringstream o;
    o << e;
    std::string s = o.str();
    // webgpu_cpp_print prints e.g. "TextureFormat::RGBA8Unorm" or, for bitmasks,
    // "BufferUsage::CopyDst|BufferUsage::Vertex"; drop the type prefixes.
    auto pos = s.find("::");
    if (pos == std::string::npos) {
        return s;
    }
    const std::string prefix = s.substr(0, pos + 2);
    for (size_t p; (p = s.find(prefix)) != std::string::npos;) {
        s.erase(p, prefix.size());
    }
    return s;
}

const char* ObjectTypeName(schema::ObjectType type) {
    switch (type) {
#define CASE(NAME)                   \
    case schema::ObjectType::NAME:   \
        return #NAME;
        DAWN_REPLAY_OBJECT_TYPES(CASE)
#undef CASE
        default:
            return "Invalid";
    }
}

// Tracks the current label of every GPU object seen so far in the stream.
class LabelTracker {
public:
    void set(schema::ObjectId id, schema::ObjectType type, const std::string& label) {
        auto& info = fObjects[id];
        if (info.fId == 0) {
            info.fId = id;
            fOrder.push_back(id);
        }
        info.fType = ObjectTypeName(type);
        info.fLabel = label;
    }

    // Formats a reference to a GPU object, e.g. `Buffer#12 "Uniforms"`.
    std::string ref(schema::ObjectId id) const {
        if (id == 0) {
            return "null";
        }
        auto it = fObjects.find(id);
        if (it == fObjects.end()) {
            return Fmt("#%llu", (unsigned long long)id);
        }
        const ObjectInfo& info = it->second;
        if (info.fLabel.empty()) {
            return Fmt("%s#%llu", info.fType.c_str(), (unsigned long long)id);
        }
        return Fmt("%s#%llu \"%s\"",
                   info.fType.c_str(),
                   (unsigned long long)id,
                   info.fLabel.c_str());
    }

    std::vector<ObjectInfo> objects() const {
        std::vector<ObjectInfo> result;
        result.reserve(fOrder.size());
        for (auto id : fOrder) {
            result.push_back(fObjects.at(id));
        }
        return result;
    }

private:
    std::unordered_map<schema::ObjectId, ObjectInfo> fObjects;
    std::vector<schema::ObjectId> fOrder;
};

// ---- Per-command detail strings -------------------------------------------------------------

class Describer {
public:
    explicit Describer(const LabelTracker& labels) : fL(labels) {}

    // Fallback for commands without extra details.
    template <typename T>
    std::string operator()(const T&) const {
        return {};
    }

    std::string operator()(const schema::CommandBufferCommandBeginRenderPassCmdData& d) const {
        std::string s = Fmt("\"%s\"", d.label.c_str());
        for (size_t i = 0; i < d.colorAttachments.size(); ++i) {
            const auto& c = d.colorAttachments[i];
            s += Fmt(" color[%zu]=%s (%s/%s)",
                     i,
                     fL.ref(c.viewId).c_str(),
                     EnumStr(c.loadOp).c_str(),
                     EnumStr(c.storeOp).c_str());
            if (c.resolveTargetId) {
                s += Fmt(" resolve=%s", fL.ref(c.resolveTargetId).c_str());
            }
        }
        if (d.depthStencilAttachment.viewId) {
            s += Fmt(" depthStencil=%s", fL.ref(d.depthStencilAttachment.viewId).c_str());
        }
        return s;
    }
    std::string operator()(const schema::CommandBufferCommandBeginComputePassCmdData& d) const {
        return Fmt("\"%s\"", d.label.c_str());
    }
    std::string operator()(const schema::CommandBufferCommandPushDebugGroupCmdData& d) const {
        return Fmt("\"%s\"", d.groupLabel.c_str());
    }
    std::string operator()(const schema::CommandBufferCommandInsertDebugMarkerCmdData& d) const {
        return Fmt("\"%s\"", d.markerLabel.c_str());
    }
    std::string operator()(const schema::CommandBufferCommandSetRenderPipelineCmdData& d) const {
        return fL.ref(d.pipelineId);
    }
    std::string operator()(const schema::CommandBufferCommandSetComputePipelineCmdData& d) const {
        return fL.ref(d.pipelineId);
    }
    std::string operator()(const schema::CommandBufferCommandSetBindGroupCmdData& d) const {
        std::string s = Fmt("[%u] %s", d.index, fL.ref(d.bindGroupId).c_str());
        if (!d.dynamicOffsets.empty()) {
            s += " offsets=";
            for (size_t i = 0; i < d.dynamicOffsets.size(); ++i) {
                s += Fmt(i ? ",%u" : "%u", d.dynamicOffsets[i]);
            }
        }
        return s;
    }
    std::string operator()(const schema::CommandBufferCommandSetVertexBufferCmdData& d) const {
        return Fmt("[%u] %s offset=%llu size=%llu",
                   d.slot,
                   fL.ref(d.bufferId).c_str(),
                   (unsigned long long)d.offset,
                   (unsigned long long)d.size);
    }
    std::string operator()(const schema::CommandBufferCommandSetIndexBufferCmdData& d) const {
        return Fmt("%s %s offset=%llu size=%llu",
                   fL.ref(d.bufferId).c_str(),
                   EnumStr(d.format).c_str(),
                   (unsigned long long)d.offset,
                   (unsigned long long)d.size);
    }
    std::string operator()(const schema::CommandBufferCommandDrawCmdData& d) const {
        return Fmt("vertices=%u instances=%u firstVertex=%u firstInstance=%u",
                   d.vertexCount, d.instanceCount, d.firstVertex, d.firstInstance);
    }
    std::string operator()(const schema::CommandBufferCommandDrawIndexedCmdData& d) const {
        return Fmt("indices=%u instances=%u firstIndex=%u baseVertex=%d firstInstance=%u",
                   d.indexCount, d.instanceCount, d.firstIndex, d.baseVertex, d.firstInstance);
    }
    std::string operator()(const schema::CommandBufferCommandDrawIndirectCmdData& d) const {
        return Fmt("%s offset=%llu",
                   fL.ref(d.indirectBufferId).c_str(),
                   (unsigned long long)d.indirectOffset);
    }
    std::string operator()(const schema::CommandBufferCommandDrawIndexedIndirectCmdData& d) const {
        return Fmt("%s offset=%llu",
                   fL.ref(d.indirectBufferId).c_str(),
                   (unsigned long long)d.indirectOffset);
    }
    std::string operator()(const schema::CommandBufferCommandDispatchCmdData& d) const {
        return Fmt("%u x %u x %u", d.x, d.y, d.z);
    }
    std::string operator()(const schema::CommandBufferCommandDispatchIndirectCmdData& d) const {
        return Fmt("%s offset=%llu", fL.ref(d.bufferId).c_str(), (unsigned long long)d.offset);
    }
    std::string operator()(const schema::CommandBufferCommandSetScissorRectCmdData& d) const {
        return Fmt("x=%u y=%u w=%u h=%u", d.x, d.y, d.width, d.height);
    }
    std::string operator()(const schema::CommandBufferCommandSetViewportCmdData& d) const {
        return Fmt("x=%g y=%g w=%g h=%g depth=[%g, %g]",
                   d.x, d.y, d.width, d.height, d.minDepth, d.maxDepth);
    }
    std::string operator()(const schema::CommandBufferCommandSetStencilReferenceCmdData& d) const {
        return Fmt("%u", d.reference);
    }
    std::string operator()(const schema::CommandBufferCommandSetBlendConstantCmdData& d) const {
        return Fmt("(%g, %g, %g, %g)", d.color.r, d.color.g, d.color.b, d.color.a);
    }
    std::string operator()(const schema::CommandBufferCommandSetImmediatesCmdData& d) const {
        return Fmt("offset=%u bytes=%zu", d.offset, d.data.size());
    }
    std::string operator()(const schema::CommandBufferCommandExecuteBundlesCmdData& d) const {
        return refList(d.bundleIds);
    }
    std::string operator()(const schema::CommandBufferCommandClearBufferCmdData& d) const {
        return Fmt("%s offset=%llu size=%llu",
                   fL.ref(d.bufferId).c_str(),
                   (unsigned long long)d.offset,
                   (unsigned long long)d.size);
    }
    std::string operator()(const schema::CommandBufferCommandWriteBufferCmdData& d) const {
        return Fmt("%s offset=%llu bytes=%zu",
                   fL.ref(d.bufferId).c_str(),
                   (unsigned long long)d.bufferOffset,
                   d.data.size());
    }
    std::string operator()(const schema::CommandBufferCommandCopyBufferToBufferCmdData& d) const {
        return Fmt("%s+%llu -> %s+%llu size=%llu",
                   fL.ref(d.srcBufferId).c_str(),
                   (unsigned long long)d.srcOffset,
                   fL.ref(d.dstBufferId).c_str(),
                   (unsigned long long)d.dstOffset,
                   (unsigned long long)d.size);
    }
    std::string operator()(const schema::CommandBufferCommandCopyBufferToTextureCmdData& d) const {
        return Fmt("%s -> %s %s",
                   fL.ref(d.source.bufferId).c_str(),
                   fL.ref(d.destination.textureId).c_str(),
                   extent(d.copySize).c_str());
    }
    std::string operator()(const schema::CommandBufferCommandCopyTextureToBufferCmdData& d) const {
        return Fmt("%s -> %s %s",
                   fL.ref(d.source.textureId).c_str(),
                   fL.ref(d.destination.bufferId).c_str(),
                   extent(d.copySize).c_str());
    }
    std::string operator()(const schema::CommandBufferCommandCopyTextureToTextureCmdData& d) const {
        return Fmt("%s -> %s %s",
                   fL.ref(d.source.textureId).c_str(),
                   fL.ref(d.destination.textureId).c_str(),
                   extent(d.copySize).c_str());
    }
    std::string operator()(const schema::CommandBufferCommandResolveQuerySetCmdData& d) const {
        return Fmt("%s [%u, +%u] -> %s",
                   fL.ref(d.querySetId).c_str(),
                   d.firstQuery,
                   d.queryCount,
                   fL.ref(d.destinationId).c_str());
    }
    std::string operator()(const schema::CommandBufferCommandWriteTimestampCmdData& d) const {
        return Fmt("%s [%u]", fL.ref(d.querySetId).c_str(), d.queryIndex);
    }

    std::string refList(const std::vector<schema::ObjectId>& ids) const {
        std::string s = "[";
        for (size_t i = 0; i < ids.size(); ++i) {
            s += (i ? ", " : "") + fL.ref(ids[i]);
        }
        return s + "]";
    }

    static std::string extent(const schema::Extent3D& e) {
        return Fmt("%ux%ux%u", e.width, e.height, e.depthOrArrayLayers);
    }

private:
    const LabelTracker& fL;
};

// ---- Command buffer / pass visitors -------------------------------------------------------------

// Appends command nodes to a tree. Passes and debug groups become nested nodes.
class TreeBuilder {
public:
    explicit TreeBuilder(const LabelTracker& labels) : fDescribe(labels) {}

    void begin(std::vector<Node>* root) {
        fStack.clear();
        fStack.push_back(root);
    }

    template <typename T>
    VisitResult emit(const char* name, const T& data) {
        if (fStack.empty()) {
            return VisitStatus::Continue;
        }
        if constexpr (std::is_same_v<T, schema::CommandBufferCommandPopDebugGroupCmdData>) {
            if (fStack.size() > fScopeBase.back() + 1) {
                fStack.pop_back();
                return VisitStatus::Continue;
            }
        }
        std::string details = fDescribe(data);
        Node node;
        node.fText = details.empty() ? std::string(name) : std::string(name) + " " + details;
        fStack.back()->push_back(std::move(node));

        if constexpr (std::is_same_v<T, schema::CommandBufferCommandPushDebugGroupCmdData>) {
            fStack.push_back(&fStack.back()->back().fChildren);
        } else if constexpr (std::is_same_v<T, schema::CommandBufferCommandEndCmdData>) {
            // End closes the current pass / encoder scope (and any unbalanced debug groups).
            if (fScopeBase.size() > 1) {
                fStack.resize(fScopeBase.back());
                fScopeBase.pop_back();
            }
        }
        return VisitStatus::Continue;
    }

    // Called for BeginRenderPass/BeginComputePass: the node becomes a new scope.
    template <typename T>
    void beginScope(const char* name, const T& data) {
        (void)emit(name, data);
        fScopeBase.push_back(fStack.size());
        fStack.push_back(&fStack.back()->back().fChildren);
    }

    void reset() { fScopeBase = {0}; }

private:
    Describer fDescribe;
    std::vector<std::vector<Node>*> fStack;
    std::vector<size_t> fScopeBase = {0};
};

#define EMIT_METHOD(NAME)                                                                       \
    VisitResult operator()(const schema::CommandBufferCommand##NAME##CmdData& data) override { \
        return fTree->emit(#NAME, data);                                                       \
    }

class RenderPassEmitter final : public dawn::replay::RenderPassVisitor {
public:
    explicit RenderPassEmitter(TreeBuilder* tree) : fTree(tree) {}
    DAWN_REPLAY_RENDER_PASS_COMMANDS(EMIT_METHOD)

private:
    TreeBuilder* fTree;
};

class ComputePassEmitter final : public dawn::replay::ComputePassVisitor {
public:
    explicit ComputePassEmitter(TreeBuilder* tree) : fTree(tree) {}
    DAWN_REPLAY_COMPUTE_PASS_COMMANDS(EMIT_METHOD)

private:
    TreeBuilder* fTree;
};

class RenderBundleEmitter final : public dawn::replay::RenderBundleVisitor {
public:
    explicit RenderBundleEmitter(TreeBuilder* tree) : fTree(tree) {}
    DAWN_REPLAY_RENDER_BUNDLE_COMMANDS(EMIT_METHOD)

private:
    TreeBuilder* fTree;
};

class EncoderEmitter final : public dawn::replay::EncoderVisitor {
public:
    explicit EncoderEmitter(TreeBuilder* tree)
            : fTree(tree), fRenderPass(tree), fComputePass(tree) {}

    dawn::replay::ResultOrError<dawn::replay::ComputePassVisitor*> BeginComputePass(
            const schema::CommandBufferCommandBeginComputePassCmdData& data) override {
        fTree->beginScope("BeginComputePass", data);
        return &fComputePass;
    }
    dawn::replay::ResultOrError<dawn::replay::RenderPassVisitor*> BeginRenderPass(
            const schema::CommandBufferCommandBeginRenderPassCmdData& data) override {
        fTree->beginScope("BeginRenderPass", data);
        return &fRenderPass;
    }

    DAWN_REPLAY_ENCODER_NON_CREATION_COMMANDS(EMIT_METHOD)

private:
    TreeBuilder* fTree;
    RenderPassEmitter fRenderPass;
    ComputePassEmitter fComputePass;
};

#undef EMIT_METHOD

// ---- Root command visitor -----------------------------------------------------------------

class ResourceDescriber final : public dawn::replay::ResourceVisitor {
public:
    using dawn::replay::ResourceVisitor::operator();

    ResourceDescriber(LabelTracker* labels) : fLabels(labels), fTree(*labels) {}

    void setTarget(Node* node) { fNode = node; }

    VisitResult operator()(const dawn::replay::DeviceData&) override {
        return VisitStatus::Continue;
    }
    VisitResult operator()(const dawn::replay::InvalidData&) override {
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::Buffer& d) override {
        append(Fmt("size=%llu usage=%s", (unsigned long long)d.size, EnumStr(d.usage).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::Texture& d) override {
        append(Fmt("%s %s mips=%u samples=%u usage=%s",
                   Describer::extent(d.size).c_str(),
                   EnumStr(d.format).c_str(),
                   d.mipLevelCount,
                   d.sampleCount,
                   EnumStr(d.usage).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::TextureView& d) override {
        append(Fmt("of %s %s", fLabels->ref(d.textureId).c_str(), EnumStr(d.format).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const dawn::replay::BindGroupData& d) override {
        append(Fmt("layout=%s entries=%u",
                   fLabels->ref(d.bg.layoutId).c_str(),
                   d.bg.numEntries));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const dawn::replay::BindGroupLayoutData& d) override {
        append(Fmt("entries=%u", d.bgl.numEntries));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::PipelineLayout& d) override {
        Describer describe(*fLabels);
        append("bindGroupLayouts=" + describe.refList(d.bindGroupLayoutIds));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RenderPipeline& d) override {
        append(Fmt("layout=%s vs=%s:%s fs=%s:%s",
                   fLabels->ref(d.layoutId).c_str(),
                   fLabels->ref(d.vertex.program.moduleId).c_str(),
                   d.vertex.program.entryPoint.c_str(),
                   fLabels->ref(d.fragment.program.moduleId).c_str(),
                   d.fragment.program.entryPoint.c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::ComputePipeline& d) override {
        append(Fmt("layout=%s cs=%s:%s",
                   fLabels->ref(d.layoutId).c_str(),
                   fLabels->ref(d.compute.moduleId).c_str(),
                   d.compute.entryPoint.c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::ShaderModule& d) override {
        append(Fmt("WGSL %zu bytes", d.code.size()));
        Node code;
        code.fText = d.code;
        fNode->fChildren.push_back(std::move(code));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::Sampler& d) override {
        append(Fmt("mag=%s min=%s", EnumStr(d.magFilter).c_str(), EnumStr(d.minFilter).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::QuerySet& d) override {
        append(Fmt("%s count=%u", EnumStr(d.type).c_str(), d.count));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const dawn::replay::CommandBufferData& d) override {
        // The encoder commands follow inline in the stream and must be consumed here.
        EncoderEmitter encoder(&fTree);
        fTree.reset();
        fTree.begin(&fNode->fChildren);
        return dawn::replay::ProcessEncoderCommands(d.readHead, &encoder);
    }
    VisitResult operator()(const dawn::replay::RenderBundleData& d) override {
        RenderBundleEmitter bundle(&fTree);
        fTree.reset();
        fTree.begin(&fNode->fChildren);
        return dawn::replay::ProcessRenderBundleCommands(d.readHead, &bundle);
    }

private:
    void append(const std::string& s) { fNode->fText += " " + s; }

    LabelTracker* fLabels;
    TreeBuilder fTree;
    Node* fNode = nullptr;
};

class RootEmitter final : public dawn::replay::RootCommandVisitor {
public:
    using dawn::replay::RootCommandVisitor::operator();

    RootEmitter(std::vector<Node>* out, LabelTracker* labels)
            : fOut(out), fLabels(labels), fResources(labels) {}

    void SetContentReadHead(dawn::replay::ReadHead*) override {}
    dawn::replay::ResourceVisitor& GetResourceVisitor() override { return fResources; }

    VisitResult operator()(const dawn::replay::CreateResourceData& data) override {
        const auto& r = data.resource;
        fLabels->set(r.id, r.type, r.label);
        Node& node = push(Fmt("Create%s %s", ObjectTypeName(r.type), fLabels->ref(r.id).c_str()));
        fResources.setTarget(&node);
        return std::visit(fResources, data.data);
    }
    VisitResult operator()(const schema::RootCommandQueueSubmitCmdData& d) override {
        Describer describe(*fLabels);
        push("QueueSubmit " + describe.refList(d.commandBuffers));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandWriteBufferCmdData& d) override {
        push(Fmt("WriteBuffer %s offset=%llu size=%llu",
                 fLabels->ref(d.bufferId).c_str(),
                 (unsigned long long)d.bufferOffset,
                 (unsigned long long)d.size));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandWriteTextureCmdData& d) override {
        push(Fmt("WriteTexture %s mip=%u %s",
                 fLabels->ref(d.destination.textureId).c_str(),
                 d.destination.mipLevel,
                 Describer::extent(d.size).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandInitTextureCmdData& d) override {
        push(Fmt("InitTexture %s mip=%u %s",
                 fLabels->ref(d.destination.textureId).c_str(),
                 d.destination.mipLevel,
                 Describer::extent(d.size).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandSetLabelCmdData& d) override {
        std::string before = fLabels->ref(d.id);
        fLabels->set(d.id, d.type, d.label);
        push(Fmt("SetLabel %s -> \"%s\"", before.c_str(), d.label.c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandSurfaceConfigureCmdData& d) override {
        push(Fmt("SurfaceConfigure %s %ux%u %s",
                 fLabels->ref(d.surfaceId).c_str(),
                 d.config.width,
                 d.config.height,
                 EnumStr(d.config.format).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandSurfaceUnconfigureCmdData& d) override {
        push("SurfaceUnconfigure " + fLabels->ref(d.surfaceId));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandSurfacePresentCmdData& d) override {
        push("SurfacePresent " + fLabels->ref(d.surfaceId));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandSurfaceGetCurrentTextureCmdData& d) override {
        push(Fmt("SurfaceGetCurrentTexture %s -> %s",
                 fLabels->ref(d.surfaceId).c_str(),
                 fLabels->ref(d.textureId).c_str()));
        return VisitStatus::Continue;
    }
    VisitResult operator()(const schema::RootCommandEndCmdData&) override {
        push("End");
        return VisitStatus::Continue;
    }

private:
    Node& push(std::string text) {
        Node node;
        node.fText = Fmt("%zu: ", fOut->size()) + text;
        fOut->push_back(std::move(node));
        return fOut->back();
    }

    std::vector<Node>* fOut;
    LabelTracker* fLabels;
    ResourceDescriber fResources;
};

}  // namespace

std::unique_ptr<WebGPUCaptureView> WebGPUCaptureView::MakeFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return nullptr;
    }
    std::vector<uint8_t> commands((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());

    std::unique_ptr<WebGPUCaptureView> view(new WebGPUCaptureView());
    view->fPath = path;
    view->fFileSize = commands.size();

    LabelTracker labels;
    RootEmitter emitter(&view->fCommands, &labels);
    dawn::replay::ReadHead readHead(commands);
    // Contents (buffer/texture data) are not needed to list the commands.
    std::vector<uint8_t> noContent;
    dawn::replay::ReadHead contentReadHead(noContent);

    // CaptureWalker::Walk only needs the read heads; the virtual getters are unused here.
    struct Walker final : dawn::replay::CaptureWalker {
        dawn::replay::ReadHead GetCommandReadHead() const override {
            return dawn::replay::ReadHead(std::span<const uint8_t>());
        }
        dawn::replay::ReadHead GetContentReadHead() const override {
            return dawn::replay::ReadHead(std::span<const uint8_t>());
        }
    } walker;
    auto result = walker.Walk(emitter, &readHead, &contentReadHead);
    if (result.IsError()) {
        view->fError = result.AcquireError()->GetMessage();
    }
    view->fObjects = labels.objects();
    return view;
}

#else  // !defined(SK_VIEWER_WEBGPU_CAPTURE)

std::unique_ptr<WebGPUCaptureView> WebGPUCaptureView::MakeFromFile(const std::string&) {
    return nullptr;
}

#endif  // defined(SK_VIEWER_WEBGPU_CAPTURE)

void WebGPUCaptureView::drawNode(const Node& node, int& id) {
    ImGui::PushID(id++);
    if (node.fChildren.empty()) {
        ImGui::BulletText("%s", node.fText.c_str());
    } else if (ImGui::TreeNode("##node", "%s", node.fText.c_str())) {
        for (const Node& child : node.fChildren) {
            this->drawNode(child, id);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

static bool node_matches(const WebGPUCaptureView::Node& node, const char* filter) {
    if (node.fText.find(filter) != std::string::npos) {
        return true;
    }
    for (const auto& child : node.fChildren) {
        if (node_matches(child, filter)) {
            return true;
        }
    }
    return false;
}

void WebGPUCaptureView::draw() {
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("WebGPU Commands")) {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("%s (%zu bytes)", fPath.c_str(), fFileSize);
    if (!fError.empty()) {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Parse error: %s", fError.c_str());
    }
    ImGui::InputText("Filter", fFilter, sizeof(fFilter));
    ImGui::Separator();

    if (ImGui::BeginTabBar("WebGPUCaptureTabs")) {
        if (ImGui::BeginTabItem("Commands")) {
            ImGui::Text("Root commands: %zu", fCommands.size());
            ImGui::BeginChild("WebGPUCommandList", ImVec2(0, 0), true);
            int id = 0;
            for (const Node& node : fCommands) {
                if (fFilter[0] && !node_matches(node, fFilter)) {
                    continue;
                }
                this->drawNode(node, id);
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Objects")) {
            ImGui::Text("GPU objects: %zu", fObjects.size());
            ImGui::BeginChild("WebGPUObjectList", ImVec2(0, 0), true);
            ImGui::Columns(3, "WebGPUObjectColumns");
            ImGui::SetColumnWidth(0, 70);
            ImGui::SetColumnWidth(1, 150);
            ImGui::Text("Id");
            ImGui::NextColumn();
            ImGui::Text("Type");
            ImGui::NextColumn();
            ImGui::Text("Label");
            ImGui::NextColumn();
            ImGui::Separator();
            for (const ObjectInfo& obj : fObjects) {
                if (fFilter[0] && obj.fType.find(fFilter) == std::string::npos &&
                    obj.fLabel.find(fFilter) == std::string::npos) {
                    continue;
                }
                ImGui::Text("%llu", (unsigned long long)obj.fId);
                ImGui::NextColumn();
                ImGui::Text("%s", obj.fType.c_str());
                ImGui::NextColumn();
                ImGui::Text("%s", obj.fLabel.empty() ? "<no label>" : obj.fLabel.c_str());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
