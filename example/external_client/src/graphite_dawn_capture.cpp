/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Same as graphite_metal_capture.cpp, but uses Graphite's Dawn backend
// (DawnBackendContext) instead of the native Metal backend.
//
// Draws either a small built-in demo scene, or replays existing SKP files, and writes:
//   <out>.capt                Skia SkCapture (viewable in viewer)
//   <out>.capt.png            the rendered result (pixel readback)
//   <out>.capt.dawn_commands  Dawn WebGPU capture command stream  (--adapter=webgpu only)
//   <out>.capt.dawn_content   Dawn WebGPU capture content stream  (--adapter=webgpu only)

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/codec/SkCodec.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/ports/SkFontMgr_mac_ct.h"
#include "src/capture/SkCapture.h"

#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_cpp.h"

// Defined by the BUILD rule when Dawn is built with --@dawn//:dawn_enable_webgpu_on_webgpu.
#if defined(SK_DAWN_WEBGPU_ON_WEBGPU)
#include <fstream>
#include "dawn/native/WebGPUBackend.h"
#endif

namespace fs = std::filesystem;

namespace {

constexpr int kDemoWidth = 200;
constexpr int kDemoHeight = 400;

struct BackendChoice {
    const char* fName;
    wgpu::BackendType fType;
};

constexpr BackendChoice kBackends[] = {
        {"metal", wgpu::BackendType::Metal},
        {"webgpu", wgpu::BackendType::WebGPU},
};

const char* BackendName(wgpu::BackendType type) {
    switch (type) {
        case wgpu::BackendType::Null:     return "Null";
        case wgpu::BackendType::WebGPU:   return "WebGPU";
        case wgpu::BackendType::D3D11:    return "D3D11";
        case wgpu::BackendType::D3D12:    return "D3D12";
        case wgpu::BackendType::Metal:    return "Metal";
        case wgpu::BackendType::Vulkan:   return "Vulkan";
        case wgpu::BackendType::OpenGL:   return "OpenGL";
        case wgpu::BackendType::OpenGLES: return "OpenGLES";
        default:                          return "Unknown";
    }
}

std::string ToString(wgpu::StringView sv) {
    return sv.data ? std::string(sv.data, sv.length == wgpu::kStrlen ? strlen(sv.data) : sv.length)
                   : std::string();
}

void PrintUsage(const char* argv0) {
    printf("Usage:\n"
           "  %s [--adapter=metal|webgpu] <output.capt>\n"
           "      Capture the built-in demo scene.\n"
           "  %s [--adapter=metal|webgpu] --skp=<file.skp|dir> [--skp=...] <output>\n"
           "      Replay SKPs. <output> is a .capt file if there is exactly one SKP file,\n"
           "      otherwise a directory that receives <skp name>.capt (+ .png/.dawn_*) per SKP.\n",
           argv0, argv0);
}

// Counts Dawn validation / internal errors reported through the uncaptured error callback.
int gDawnErrorCount = 0;

// A unit of work: something to draw into a surface of a given size, captured to fOutputPath.
struct Job {
    std::string fName;
    std::string fOutputPath;  // <...>.capt
    SkISize fSize;
    // Draws the scene. The recorder is provided so a job can make extra surfaces; any extra
    // surfaces must be returned so they outlive the recording snap and the capture.
    std::function<std::vector<sk_sp<SkSurface>>(skgpu::graphite::Recorder*, SkSurface*)> fDraw;
};

std::vector<sk_sp<SkSurface>> DrawDemo(skgpu::graphite::Recorder* recorder, SkSurface* surface) {
    sk_sp<SkSurface> surfaceB = SkSurfaces::RenderTarget(recorder, surface->imageInfo());
    if (!surfaceB) {
        printf("Could not make second surface from Dawn Recorder\n");
        return {};
    }
    SkCanvas* canvas = surface->getCanvas();
    SkCanvas* canvasB = surfaceB->getCanvas();

    // Canvas A
    canvas->clear(SK_ColorCYAN);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    // Triggers a breakpoint in the capture
    auto contentImg = surface->makeImageSnapshot();

    // Do some more draws
    canvas->drawCircle(50, 50, 30, paint);
    paint.setColor(SK_ColorBLUE);
    canvas->drawCircle(30, 30, 10, paint);

    // Trigger another breakpoint
    auto circlesImg = surface->makeImageSnapshot();

    // Canvas B
    canvasB->clear(SK_ColorMAGENTA);
    paint.setColor(SK_ColorBLACK);
    canvasB->drawImage(contentImg, 0, 0);
    canvasB->drawImage(circlesImg, 20, 20);
    canvasB->drawCircle(10, 10, 5, paint);
    return {surfaceB};
}

// ---- SKP loading ----------------------------------------------------------------------------

// Graphite does not upload raster SkImages (e.g. decoded SKP images) on its own; a Recorder needs
// an ImageProvider to turn them into textures. This mirrors tools/graphite's TestingImageProvider.
class UploadingImageProvider : public skgpu::graphite::ImageProvider {
public:
    sk_sp<SkImage> findOrCreate(skgpu::graphite::Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties props) override {
        const uint64_t key = (uint64_t(image->uniqueID()) << 1) | (props.fMipmapped ? 1 : 0);
        if (auto it = fCache.find(key); it != fCache.end()) {
            return it->second;
        }
        sk_sp<SkImage> texture = SkImages::TextureFromImage(recorder, image, props);
        if (texture) {
            fCache[key] = texture;
        }
        return texture;
    }

private:
    std::unordered_map<uint64_t, sk_sp<SkImage>> fCache;
};

sk_sp<SkFontMgr> FontMgr() {
    static sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_CoreText(nullptr);
    return fontMgr;
}

SkDeserialProcs SkpDeserialProcs() {
    SkDeserialProcs procs;
    procs.fImageDataProc = [](sk_sp<SkData> data,
                              std::optional<SkAlphaType> alphaType,
                              void*) -> sk_sp<SkImage> {
        std::unique_ptr<SkCodec> codec;
        if (SkPngDecoder::IsPng(data->data(), data->size())) {
            codec = SkPngDecoder::Decode(data, nullptr);
        } else if (SkJpegDecoder::IsJpeg(data->data(), data->size())) {
            codec = SkJpegDecoder::Decode(data, nullptr);
        } else if (SkWebpDecoder::IsWebp(data->data(), data->size())) {
            codec = SkWebpDecoder::Decode(data, nullptr);
        }
        if (!codec) {
            printf("  WARNING: SKP contains an image in an unsupported format\n");
            return nullptr;
        }
        sk_sp<SkImage> lazy = SkCodecs::DeferredImage(std::move(codec), alphaType);
        return lazy ? lazy->makeRasterImage(nullptr) : nullptr;
    };
    procs.fTypefaceStreamProc = [](SkStream& stream, void*) -> sk_sp<SkTypeface> {
        return SkTypeface::MakeDeserialize(&stream, FontMgr());
    };
    return procs;
}

sk_sp<SkPicture> LoadSkp(const std::string& path) {
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path.c_str());
    if (!stream) {
        return nullptr;
    }
    SkDeserialProcs procs = SkpDeserialProcs();
    return SkPicture::MakeFromStream(stream.get(), &procs);
}

// ---- Dawn setup -----------------------------------------------------------------------------

wgpu::Device CreateDevice(dawn::native::Instance* instance, wgpu::BackendType backend) {
    wgpu::RequestAdapterOptions adapterOptions;
    adapterOptions.featureLevel = wgpu::FeatureLevel::Core;
    // WebGPU-on-WebGPU is selected by chaining RequestAdapterWebGPUBackendOptions. In that case
    // backendType is passed through to the *inner* Dawn instance, so ask it for Metal.
    wgpu::RequestAdapterWebGPUBackendOptions webgpuBackendOptions;
    if (backend == wgpu::BackendType::WebGPU) {
        adapterOptions.nextInChain = &webgpuBackendOptions;
        adapterOptions.backendType = wgpu::BackendType::Metal;
    } else {
        adapterOptions.backendType = backend;
    }
    std::vector<dawn::native::Adapter> adapters = instance->EnumerateAdapters(&adapterOptions);
    if (adapters.empty()) {
        printf("ERROR: No Dawn adapter found for backend %s\n", BackendName(backend));
#if !defined(SK_DAWN_WEBGPU_ON_WEBGPU)
        if (backend == wgpu::BackendType::WebGPU) {
            printf("Dawn was built without the WebGPU-on-WebGPU backend. Rebuild with "
                   "--@dawn//:dawn_enable_webgpu_on_webgpu\n");
        }
#endif
        return nullptr;
    }
    wgpu::Adapter adapter = adapters[0].Get();
    wgpu::AdapterInfo info;
    adapter.GetInfo(&info);
    printf("  Adapter: [%s] %s (%s)\n", BackendName(info.backendType),
           ToString(info.device).c_str(), ToString(info.description).c_str());

    wgpu::DeviceDescriptor deviceDesc;
    wgpu::Limits limits = {};
    adapter.GetLimits(&limits);
    deviceDesc.requiredLimits = &limits;
    deviceDesc.SetDeviceLostCallback(
            wgpu::CallbackMode::AllowSpontaneous,
            [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
                if (reason != wgpu::DeviceLostReason::Destroyed) {
                    ++gDawnErrorCount;
                    printf("DAWN ERROR: Device lost (reason %d): %s\n", static_cast<int>(reason),
                           ToString(message).c_str());
                }
            });
    deviceDesc.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
                ++gDawnErrorCount;
                printf("DAWN ERROR (type %d): %s\n", static_cast<int>(type),
                       ToString(message).c_str());
            });
    return adapter.CreateDevice(&deviceDesc);
}

bool WriteFile(const std::string& path, const void* data, size_t size) {
    SkFILEWStream out(path.c_str());
    if (!out.isValid()) {
        printf("  Cannot open output file %s\n", path.c_str());
        return false;
    }
    out.write(data, size);
    out.fsync();
    return true;
}

// Renders one job with a fresh Dawn device + Graphite context (so each capture is
// self-contained) and writes the capture files. Returns false on failure.
bool RunJob(dawn::native::Instance* instance, wgpu::BackendType backend, const Job& job) {
    printf("[%s] %dx%d -> %s\n", job.fName.c_str(), job.fSize.width(), job.fSize.height(),
           job.fOutputPath.c_str());
    gDawnErrorCount = 0;

    wgpu::Device device = CreateDevice(instance, backend);
    if (!device) {
        printf("  ERROR: Could not create Dawn device\n");
        return false;
    }

    skgpu::graphite::DawnBackendContext backendContext;
    backendContext.fInstance = wgpu::Instance(instance->Get());
    backendContext.fDevice = device;
    backendContext.fQueue = device.GetQueue();

    skgpu::graphite::ContextOptions options;
    options.fEnableCapture = true;
    std::unique_ptr<skgpu::graphite::Context> context =
            skgpu::graphite::ContextFactory::MakeDawn(backendContext, options);
    if (!context) {
        printf("  ERROR: Could not make Graphite Dawn context\n");
        return false;
    }
    skgpu::graphite::RecorderOptions recorderOptions;
    recorderOptions.fImageProvider = sk_make_sp<UploadingImageProvider>();
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder(recorderOptions);
    if (!recorder) {
        printf("  ERROR: Could not make recorder\n");
        return false;
    }
    SkImageInfo imageInfo = SkImageInfo::Make(job.fSize, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder.get(), imageInfo);
    if (!surface) {
        printf("  ERROR: Could not make %dx%d surface\n", job.fSize.width(), job.fSize.height());
        return false;
    }

#if defined(SK_DAWN_WEBGPU_ON_WEBGPU)
    // With the WebGPU-on-WebGPU backend, Dawn can also record its own WebGPU-level capture.
    const std::string dawnCommandsPath = job.fOutputPath + ".dawn_commands";
    const std::string dawnContentPath = job.fOutputPath + ".dawn_content";
    std::ofstream dawnCommands, dawnContent;
    const bool dawnCapture = backend == wgpu::BackendType::WebGPU;
    if (dawnCapture) {
        dawnCommands.open(dawnCommandsPath, std::ios::binary);
        dawnContent.open(dawnContentPath, std::ios::binary);
        dawn::native::webgpu::StartCapture(device.Get(), dawnCommands, dawnContent);
    }
#endif
    context->startCapture();

    /* DRAWING */
    std::vector<sk_sp<SkSurface>> extraSurfaces = job.fDraw(recorder.get(), surface.get());

    std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();
    if (!recording) {
        printf("  ERROR: Could not create a recording\n");
        return false;
    }
    skgpu::graphite::InsertRecordingInfo info;
    info.fRecording = recording.get();
    if (!context->insertRecording(info)) {
        printf("  ERROR: Context::insertRecording failed\n");
        return false;
    }

    struct Readback {
        SkImageInfo fInfo;
        sk_sp<SkImage> fImage;
    } readback{imageInfo, nullptr};
    auto callback = [](SkImage::ReadPixelsContext ctx,
                       std::unique_ptr<const SkImage::AsyncReadResult> result) {
        auto* rb = static_cast<Readback*>(ctx);
        if (!result || result->count() != 1) {
            printf("  WARNING: pixel readback failed\n");
            return;
        }
        SkPixmap pm(rb->fInfo, result->data(0), result->rowBytes(0));
        rb->fImage = SkImages::RasterFromPixmapCopy(pm);
    };
    context->asyncRescaleAndReadPixels(surface.get(), imageInfo,
                                       SkIRect::MakeSize(job.fSize),
                                       SkImage::RescaleGamma::kSrc,
                                       SkImage::RescaleMode::kNearest, callback, &readback);

    context->submit(skgpu::graphite::SyncToCpu::kYes);
    if (context->hasUnfinishedGpuWork()) {
        printf("  ERROR: Sync with GPU completion failed\n");
        return false;
    }

    sk_sp<SkCapture> capture = context->endCapture();
#if defined(SK_DAWN_WEBGPU_ON_WEBGPU)
    if (dawnCapture) {
        dawn::native::webgpu::EndCapture(device.Get());
        dawnCommands.flush();
        dawnContent.flush();
        printf("  Dawn WebGPU capture: %lld bytes commands, %lld bytes content\n",
               static_cast<long long>(dawnCommands.tellp()),
               static_cast<long long>(dawnContent.tellp()));
    }
#endif

    bool ok = true;
    sk_sp<SkData> serialized = capture ? capture->serializeCapture() : nullptr;
    if (serialized && WriteFile(job.fOutputPath, serialized->data(), serialized->size())) {
        auto md = SkCapture::MakeFromData(serialized);
        printf("  SkCapture: %zu bytes (%u assets, %u recording captures)\n", serialized->size(),
               md ? md->getMetadata().numAssets : 0, md ? md->getMetadata().numRecordingCaptures : 0);
    } else {
        printf("  ERROR: No SkCapture produced\n");
        ok = false;
    }

    if (readback.fImage) {
        SkPixmap pm;
        SkDynamicMemoryWStream png;
        if (readback.fImage->peekPixels(&pm) && SkPngEncoder::Encode(&png, pm, {})) {
            sk_sp<SkData> pngData = png.detachAsData();
            WriteFile(job.fOutputPath + ".png", pngData->data(), pngData->size());
        }
    }

    // Tear down Graphite before the device, then flush any remaining Dawn callbacks.
    recording.reset();
    extraSurfaces.clear();
    surface.reset();
    recorder.reset();
    context.reset();
    device.Destroy();
    backendContext.fInstance.ProcessEvents();

    if (gDawnErrorCount > 0) {
        printf("  Dawn reported %d error(s)\n", gDawnErrorCount);
        ok = false;
    }
    return ok;
}

}  // namespace

int main(int argc, char* argv[]) {
    wgpu::BackendType requestedBackend = wgpu::BackendType::Metal;
    std::vector<std::string> skpArgs;
    const char* outputPath = nullptr;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strncmp(arg, "--adapter=", 10) == 0) {
            const char* value = arg + 10;
            bool found = false;
            for (const auto& b : kBackends) {
                if (strcmp(value, b.fName) == 0) {
                    requestedBackend = b.fType;
                    found = true;
                }
            }
            if (!found) {
                printf("Unknown adapter '%s'.\n", value);
                PrintUsage(argv[0]);
                return 1;
            }
        } else if (strncmp(arg, "--skp=", 6) == 0) {
            skpArgs.push_back(arg + 6);
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            PrintUsage(argv[0]);
            return 0;
        } else if (!outputPath) {
            outputPath = arg;
        } else {
            PrintUsage(argv[0]);
            return 1;
        }
    }
    if (!outputPath) {
        PrintUsage(argv[0]);
        return 1;
    }

    /* BUILD THE LIST OF JOBS */

    std::vector<Job> jobs;
    if (skpArgs.empty()) {
        jobs.push_back({"demo", outputPath, {kDemoWidth, kDemoHeight}, DrawDemo});
    } else {
        std::vector<fs::path> skpFiles;
        for (const std::string& arg : skpArgs) {
            std::error_code ec;
            if (fs::is_directory(arg, ec)) {
                std::vector<fs::path> inDir;
                for (const auto& entry : fs::directory_iterator(arg)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".skp") {
                        inDir.push_back(entry.path());
                    }
                }
                std::sort(inDir.begin(), inDir.end());
                skpFiles.insert(skpFiles.end(), inDir.begin(), inDir.end());
            } else if (fs::is_regular_file(arg, ec)) {
                skpFiles.push_back(arg);
            } else {
                printf("ERROR: --skp=%s is not a file or directory\n", arg.c_str());
                return 1;
            }
        }
        if (skpFiles.empty()) {
            printf("ERROR: No .skp files found\n");
            return 1;
        }

        const bool singleFileOutput =
                skpFiles.size() == 1 && fs::path(outputPath).extension() == ".capt";
        if (!singleFileOutput) {
            std::error_code ec;
            fs::create_directories(outputPath, ec);
            if (ec) {
                printf("ERROR: Cannot create output directory %s\n", outputPath);
                return 1;
            }
        }

        for (const fs::path& skpPath : skpFiles) {
            sk_sp<SkPicture> picture = LoadSkp(skpPath.string());
            if (!picture) {
                printf("ERROR: Could not load SKP %s\n", skpPath.c_str());
                return 1;
            }
            SkIRect bounds = picture->cullRect().roundOut();
            if (bounds.isEmpty()) {
                printf("ERROR: SKP %s has an empty cull rect\n", skpPath.c_str());
                return 1;
            }
            std::string out = singleFileOutput
                                      ? std::string(outputPath)
                                      : (fs::path(outputPath) / skpPath.stem()).string() + ".capt";
            jobs.push_back({skpPath.filename().string(), out, bounds.size(),
                            [picture, bounds](skgpu::graphite::Recorder*, SkSurface* surface) {
                                SkCanvas* canvas = surface->getCanvas();
                                canvas->clear(SK_ColorWHITE);
                                canvas->translate(-bounds.left(), -bounds.top());
                                canvas->drawPicture(picture);
                                return std::vector<sk_sp<SkSurface>>();
                            }});
        }
    }

    /* SET UP DAWN */

    DawnProcTable procs = dawn::native::GetProcs();
    dawnProcSetProcs(&procs);

    static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
    wgpu::InstanceDescriptor instanceDesc{};
    instanceDesc.requiredFeatureCount = 1;
    instanceDesc.requiredFeatures = &kTimedWaitAny;
    auto dawnInstance = std::make_unique<dawn::native::Instance>(&instanceDesc);

    printf("Using Dawn backend %s, %zu job(s)\n", BackendName(requestedBackend), jobs.size());

    int failures = 0;
    for (const Job& job : jobs) {
        if (!RunJob(dawnInstance.get(), requestedBackend, job)) {
            ++failures;
        }
    }

    if (failures > 0) {
        printf("done, %d of %zu job(s) failed or reported Dawn errors\n", failures, jobs.size());
        return 2;
    }
    printf("done\n");
    return 0;
}
