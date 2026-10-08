/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/WebGPUCaptureReplay.h"

#if defined(SK_VIEWER_WEBGPU_CAPTURE)

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/replay/Replay.h"
#include "webgpu/webgpu_cpp.h"

namespace {

std::string ToString(wgpu::StringView sv) {
    if (!sv.data) {
        return {};
    }
    return sv.length == wgpu::kStrlen ? std::string(sv.data) : std::string(sv.data, sv.length);
}

float HalfToFloat(uint16_t h) {
    const uint32_t sign = (h >> 15) & 1;
    const uint32_t exp = (h >> 10) & 0x1F;
    const uint32_t mant = h & 0x3FF;
    float v;
    if (exp == 0) {
        v = std::ldexp(static_cast<float>(mant), -24);
    } else if (exp == 31) {
        v = mant ? NAN : INFINITY;
    } else {
        v = std::ldexp(static_cast<float>(mant | 0x400), static_cast<int>(exp) - 25);
    }
    return sign ? -v : v;
}

uint8_t ToUnorm8(float v) {
    if (!(v > 0.f)) {
        return 0;
    }
    return static_cast<uint8_t>(std::lround(std::min(v, 1.f) * 255.f));
}

class ReplayImpl final : public WebGPUCaptureReplay {
public:
    bool init(const std::string& commandsPath, const std::string& contentPath, std::string* error) {
        fCommandsPath = commandsPath;
        fContentPath = contentPath;

        // The viewer may not be running on a Dawn backend, so make sure the procs are set.
        static DawnProcTable sProcs = dawn::native::GetProcs();
        dawnProcSetProcs(&sProcs);

        static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        wgpu::InstanceDescriptor instanceDesc;
        instanceDesc.requiredFeatureCount = 1;
        instanceDesc.requiredFeatures = &kTimedWaitAny;
        fNativeInstance = std::make_unique<dawn::native::Instance>(&instanceDesc);
        fInstance = wgpu::Instance(fNativeInstance->Get());

        wgpu::RequestAdapterOptions options;
        options.backendType = wgpu::BackendType::Metal;
        std::vector<dawn::native::Adapter> adapters = fNativeInstance->EnumerateAdapters(&options);
        if (adapters.empty()) {
            *error = "No Dawn Metal adapter found for replay";
            return false;
        }
        wgpu::Adapter adapter = adapters[0].Get();

        // Enable everything the adapter supports: the capture may use any of it.
        wgpu::SupportedFeatures supported;
        adapter.GetFeatures(&supported);
        std::vector<wgpu::FeatureName> features(supported.features,
                                                supported.features + supported.featureCount);
        wgpu::Limits limits = {};
        adapter.GetLimits(&limits);

        wgpu::DeviceDescriptor desc;
        desc.label = "Viewer WebGPU capture replay";
        desc.requiredFeatureCount = features.size();
        desc.requiredFeatures = features.data();
        desc.requiredLimits = &limits;
        desc.SetUncapturedErrorCallback(
                [](const wgpu::Device&,
                   wgpu::ErrorType,
                   wgpu::StringView message,
                   std::vector<std::string>* errors) {
                    errors->push_back(ToString(message));
                    std::fprintf(stderr, "WebGPU replay error: %s\n", errors->back().c_str());
                },
                &fErrors);
        fDevice = adapter.CreateDevice(&desc);
        if (!fDevice) {
            *error = "Could not create the replay Dawn device";
            return false;
        }
        return this->play(kNoDrawLimit, error);
    }

    bool play(uint64_t drawLimit, std::string* error) override {
        // Drop the previous replay (and all of its objects) first.
        fReplay.reset();
        fDrawLimit = 0;

        std::ifstream commands(fCommandsPath, std::ios::binary | std::ios::ate);
        std::ifstream content(fContentPath, std::ios::binary | std::ios::ate);
        if (!commands || !content) {
            *error = "Could not open " + (commands ? fContentPath : fCommandsPath);
            return false;
        }
        const size_t commandsSize = static_cast<size_t>(commands.tellg());
        const size_t contentSize = static_cast<size_t>(content.tellg());
        commands.seekg(0);
        content.seekg(0);

        auto capture =
                dawn::replay::Capture::Create(commands, commandsSize, content, contentSize);
        if (!capture) {
            *error = "Could not load the WebGPU capture";
            return false;
        }
        fReplay = dawn::replay::Replay::Create(fDevice, std::move(capture));
        if (!fReplay) {
            *error = "Could not create the WebGPU capture replay";
            return false;
        }
        fReplay->SetDrawLimit(drawLimit);
        const size_t errorsBefore = fErrors.size();
        if (!fReplay->Play()) {
            *error = "WebGPU capture replay failed";
            if (fErrors.size() != errorsBefore) {
                *error += ": " + fErrors[errorsBefore];
            }
            fReplay.reset();
            return false;
        }
        fDrawLimit = drawLimit;
        return true;
    }

    uint64_t drawLimit() const override { return fDrawLimit; }

    bool readDepth(uint64_t textureId, DepthImage* out, std::string* error) override {
        wgpu::Texture texture = this->texture(textureId, error);
        if (!texture) {
            return false;
        }
        if (!(texture.GetUsage() & wgpu::TextureUsage::TextureBinding)) {
            *error = "Depth texture is not bindable (re-capture with --track-draw-ids on a "
                     "current build)";
            return false;
        }
        const uint32_t width = texture.GetWidth();
        const uint32_t height = texture.GetHeight();
        const bool multisampled = texture.GetSampleCount() > 1;

        const std::string wgsl = std::string("@group(0) @binding(0) var tex : ") +
                                 (multisampled ? "texture_depth_multisampled_2d" : "texture_depth_2d") +
                                 R"(;
@group(0) @binding(1) var<storage, read_write> outDepth : array<f32>;
@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) id : vec3u) {
    let dims = textureDimensions(tex);
    if (id.x >= dims.x || id.y >= dims.y) {
        return;
    }
    outDepth[id.y * dims.x + id.x] = textureLoad(tex, vec2i(id.xy), 0);
}
)";
        wgpu::ShaderSourceWGSL wgslDesc;
        wgslDesc.code = wgsl.c_str();
        wgpu::ShaderModuleDescriptor moduleDesc;
        moduleDesc.nextInChain = &wgslDesc;
        wgpu::ShaderModule module = fDevice.CreateShaderModule(&moduleDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc;
        pipelineDesc.compute.module = module;
        pipelineDesc.compute.entryPoint = "main";
        wgpu::ComputePipeline pipeline = fDevice.CreateComputePipeline(&pipelineDesc);

        const uint64_t size = uint64_t(width) * height * sizeof(float);
        wgpu::BufferDescriptor storageDesc;
        storageDesc.size = size;
        storageDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer storage = fDevice.CreateBuffer(&storageDesc);
        wgpu::Buffer readback = this->makeReadbackBuffer(size);

        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        wgpu::TextureView view = texture.CreateView(&viewDesc);

        wgpu::BindGroupEntry entries[2];
        entries[0].binding = 0;
        entries[0].textureView = view;
        entries[1].binding = 1;
        entries[1].buffer = storage;
        entries[1].size = size;
        wgpu::BindGroupDescriptor bgDesc;
        bgDesc.layout = pipeline.GetBindGroupLayout(0);
        bgDesc.entryCount = 2;
        bgDesc.entries = entries;
        wgpu::BindGroup bindGroup = fDevice.CreateBindGroup(&bgDesc);

        const size_t errorsBefore = fErrors.size();
        wgpu::CommandEncoder encoder = fDevice.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups((width + 7) / 8, (height + 7) / 8);
        pass.End();
        encoder.CopyBufferToBuffer(storage, 0, readback, 0, size);
        wgpu::CommandBuffer commands = encoder.Finish();
        fDevice.GetQueue().Submit(1, &commands);

        if (!this->map(readback, size, errorsBefore, "Depth", error)) {
            return false;
        }
        const float* data = static_cast<const float*>(readback.GetConstMappedRange(0, size));
        out->fWidth = width;
        out->fHeight = height;
        out->fDepth.assign(data, data + size_t(width) * height);
        readback.Unmap();
        return true;
    }

    bool readColor(uint64_t textureId, ColorImage* out, std::string* error) override {
        wgpu::Texture texture = this->texture(textureId, error);
        if (!texture) {
            return false;
        }
        if (texture.GetSampleCount() > 1) {
            *error = "Cannot read back a multisampled color texture";
            return false;
        }
        const wgpu::TextureFormat format = texture.GetFormat();
        uint32_t bytesPerPixel = 0;
        switch (format) {
            case wgpu::TextureFormat::R8Unorm:
                bytesPerPixel = 1;
                break;
            case wgpu::TextureFormat::RG8Unorm:
                bytesPerPixel = 2;
                break;
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8UnormSrgb:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::BGRA8UnormSrgb:
            case wgpu::TextureFormat::RGB10A2Unorm:
                bytesPerPixel = 4;
                break;
            case wgpu::TextureFormat::RGBA16Float:
                bytesPerPixel = 8;
                break;
            default:
                *error = "Unsupported color format for readback";
                return false;
        }
        const uint32_t width = texture.GetWidth();
        const uint32_t height = texture.GetHeight();
        const uint32_t bytesPerRow = (width * bytesPerPixel + 255) & ~255u;
        const uint64_t size = uint64_t(bytesPerRow) * height;
        wgpu::Buffer readback = this->makeReadbackBuffer(size);

        const size_t errorsBefore = fErrors.size();
        wgpu::TexelCopyTextureInfo src = {};
        src.texture = texture;
        wgpu::TexelCopyBufferInfo dst = {};
        dst.buffer = readback;
        dst.layout.bytesPerRow = bytesPerRow;
        dst.layout.rowsPerImage = height;
        wgpu::Extent3D extent = {width, height, 1};
        wgpu::CommandEncoder encoder = fDevice.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&src, &dst, &extent);
        wgpu::CommandBuffer commands = encoder.Finish();
        fDevice.GetQueue().Submit(1, &commands);

        if (!this->map(readback, size, errorsBefore, "Color", error)) {
            return false;
        }
        const uint8_t* data = static_cast<const uint8_t*>(readback.GetConstMappedRange(0, size));
        out->fWidth = width;
        out->fHeight = height;
        out->fRGBA.resize(size_t(width) * height * 4);
        for (uint32_t y = 0; y < height; ++y) {
            const uint8_t* row = data + size_t(y) * bytesPerRow;
            uint8_t* dstRow = out->fRGBA.data() + size_t(y) * width * 4;
            for (uint32_t x = 0; x < width; ++x) {
                const uint8_t* p = row + size_t(x) * bytesPerPixel;
                uint8_t* d = dstRow + size_t(x) * 4;
                switch (format) {
                    case wgpu::TextureFormat::R8Unorm:
                        d[0] = d[1] = d[2] = p[0];
                        d[3] = 255;
                        break;
                    case wgpu::TextureFormat::RG8Unorm:
                        d[0] = p[0];
                        d[1] = p[1];
                        d[2] = 0;
                        d[3] = 255;
                        break;
                    case wgpu::TextureFormat::BGRA8Unorm:
                    case wgpu::TextureFormat::BGRA8UnormSrgb:
                        d[0] = p[2];
                        d[1] = p[1];
                        d[2] = p[0];
                        d[3] = p[3];
                        break;
                    case wgpu::TextureFormat::RGB10A2Unorm: {
                        uint32_t v;
                        std::memcpy(&v, p, 4);
                        d[0] = static_cast<uint8_t>(((v >> 0) & 0x3FF) >> 2);
                        d[1] = static_cast<uint8_t>(((v >> 10) & 0x3FF) >> 2);
                        d[2] = static_cast<uint8_t>(((v >> 20) & 0x3FF) >> 2);
                        d[3] = static_cast<uint8_t>(((v >> 30) & 0x3) * 85);
                        break;
                    }
                    case wgpu::TextureFormat::RGBA16Float: {
                        uint16_t h[4];
                        std::memcpy(h, p, 8);
                        for (int c = 0; c < 4; ++c) {
                            d[c] = ToUnorm8(HalfToFloat(h[c]));
                        }
                        break;
                    }
                    default:  // RGBA8Unorm(Srgb)
                        std::memcpy(d, p, 4);
                        break;
                }
            }
        }
        readback.Unmap();
        return true;
    }

    const std::vector<std::string>& deviceErrors() const override { return fErrors; }

private:
    wgpu::Texture texture(uint64_t textureId, std::string* error) const {
        if (!fReplay) {
            *error = "The capture has not been replayed";
            return nullptr;
        }
        wgpu::Texture texture = fReplay->GetObjectById<wgpu::Texture>(textureId);
        if (!texture) {
            *error = "No replayed texture with capture id " + std::to_string(textureId);
        }
        return texture;
    }

    wgpu::Buffer makeReadbackBuffer(uint64_t size) const {
        wgpu::BufferDescriptor readDesc;
        readDesc.size = size;
        readDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        return fDevice.CreateBuffer(&readDesc);
    }

    bool map(wgpu::Buffer buffer, uint64_t size, size_t errorsBefore, const char* what,
             std::string* error) {
        struct MapResult {
            bool fOk = false;
            std::string fMessage;
        } result;
        wgpu::Future future = buffer.MapAsync(
                wgpu::MapMode::Read,
                0,
                size,
                wgpu::CallbackMode::WaitAnyOnly,
                [](wgpu::MapAsyncStatus status, wgpu::StringView message, MapResult* r) {
                    r->fOk = status == wgpu::MapAsyncStatus::Success;
                    r->fMessage = ToString(message);
                },
                &result);
        fInstance.WaitAny(future, UINT64_MAX);
        if (!result.fOk || fErrors.size() != errorsBefore) {
            *error = std::string(what) + " readback failed: " +
                     (fErrors.size() != errorsBefore ? fErrors.back() : result.fMessage);
            return false;
        }
        return true;
    }

    std::string fCommandsPath;
    std::string fContentPath;
    std::unique_ptr<dawn::native::Instance> fNativeInstance;
    wgpu::Instance fInstance;
    wgpu::Device fDevice;
    std::unique_ptr<dawn::replay::Replay> fReplay;
    uint64_t fDrawLimit = 0;
    std::vector<std::string> fErrors;
};

}  // namespace

std::unique_ptr<WebGPUCaptureReplay> WebGPUCaptureReplay::Make(const std::string& commandsPath,
                                                               const std::string& contentPath,
                                                               std::string* error) {
    auto replay = std::make_unique<ReplayImpl>();
    if (!replay->init(commandsPath, contentPath, error)) {
        return nullptr;
    }
    return replay;
}

#else  // !defined(SK_VIEWER_WEBGPU_CAPTURE)

std::unique_ptr<WebGPUCaptureReplay> WebGPUCaptureReplay::Make(const std::string&,
                                                               const std::string&,
                                                               std::string* error) {
    *error = "Viewer was built without Dawn (skia_use_dawn=false)";
    return nullptr;
}

#endif  // defined(SK_VIEWER_WEBGPU_CAPTURE)
