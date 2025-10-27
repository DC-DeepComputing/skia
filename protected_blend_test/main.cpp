/*
 * Copyright 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <vulkan/vulkan.h>
#include "include/android/graphite/SurfaceAndroid.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "include/gpu/vk/VulkanBackendContext.h"

#include "src/base/SkTime.h"

#include "tools/fonts/TestFontMgr.h"
#include "tools/window/GraphiteDisplayParams.h"

// #include <android-base/stringprintf.h>
#include <android/hardware_buffer.h>
#include <android/native_window.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "tools/flags/CommandLineFlags.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

#include <chrono>
#include <thread>
#include <unistd.h>

#define LOG_TAG "skia"

#define ALOG(priority, tag, fmt, ...)                                        \
    do {                                                                     \
        __android_log_print(ANDROID_##priority, tag, "%s: " fmt "\n",        \
                            (tag)__VA_OPT__(, ) __VA_ARGS__);                \
        if constexpr (ANDROID_##priority == ANDROID_LOG_FATAL) std::abort(); \
    } while (false)
#define ALOGV(...) ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) ALOG(LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) ALOG(LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) ALOG(LOG_ERROR, LOG_TAG, __VA_ARGS__)

static DEFINE_string(fg, "0x40FFFFFF", "Foreground to render, either a color in hex format or path to image");
static DEFINE_int(numBlends, 1, "Number of foregrounds to render");
static DEFINE_int(allocateUpfrontMB, 0, "Size to allocate upfront, MB");
static DEFINE_int(allocateStepMB, 100, "Size to allocate on each step, MB");
static DEFINE_int(allocateIntervalMS, 1000, "Time between consecutive allocation steps, ms");
static DEFINE_string(backend, "graphite", "Skia GPU backend to use: graphite|ganesh|gles");

using BackendType = sk_app::Window::BackendType;

struct Config {
    const char *fg;
    int numBlends;
    int allocateUpfrontMB;
    int allocateStepMB;
    int allocateIntervalMS;
    BackendType backendType = BackendType::kGraphiteVulkan_BackendType;

    void parseArgs(int argc, char ** argv) {
        CommandLineFlags::Parse(argc, argv);

        fg = FLAGS_fg[0];
        numBlends = FLAGS_numBlends;
        allocateUpfrontMB = FLAGS_allocateUpfrontMB;
        allocateStepMB = FLAGS_allocateStepMB;
        allocateIntervalMS = FLAGS_allocateIntervalMS;
        if (!strcmp(FLAGS_backend[0], "graphite")) {
            backendType = BackendType::kGraphiteVulkan_BackendType;
        } else if (!strcmp(FLAGS_backend[0], "ganesh")) {
            backendType = BackendType::kVulkan_BackendType;
        } else if (!strcmp(FLAGS_backend[0], "gles")) {
            backendType = BackendType::kNativeGL_BackendType;
        } else {
            ALOGW("Unsupported GPU backend type: %s, falling back to Graphite", FLAGS_backend[0]);
        }
    }
};

struct BlendLayer : public sk_app::Window::Layer {
    Config fConfig;
    sk_app::Window* fParent = nullptr;
    sk_sp<SkImage> fForegroundImage = nullptr;
    uint32_t fForegroundColor = 0;
    sk_sp<SkFontMgr> fontMgr = nullptr;
    sk_sp<SkTypeface> typeface = nullptr;
    SkFont font;
    unsigned long long lastAllocTS = 0;
    size_t totalAllocatedMB = 0u;
    std::vector<void *> allocations;

    BlendLayer(const Config &config) : fConfig(config) {}

    ~BlendLayer() override {
        for (auto *allocation : allocations) {
            free(allocation);
        }
        allocations.clear();
    }

    BackendType getBackendType() const
    {
        return fConfig.backendType;
    }

    sk_sp<SkImage> loadImage(const char* path) {
        sk_sp<SkData> data = SkData::MakeFromFileName(path);
        if (!data) {
            ALOGE("Failed to load image %s", path);
            return nullptr;
        }
        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
        if (!codec) {
            ALOGE("Failed to create codec for %s", path);
            return nullptr;
        }
        SkImageInfo info = codec->getInfo();
        sk_sp<SkImage> image = std::get<0>(codec->getImage(info));
        if (!image) {
            ALOGE("Failed to decode image %s", path);
            return nullptr;
        }
        return image;
    }

    void init() {
        auto colorLong = strtol(fConfig.fg, nullptr, 16);
        if (colorLong) {
            fForegroundColor = static_cast<uint32_t>(colorLong);
        } else {
            ALOGW("Unable to parse %s as color, assuming file path", fConfig.fg);
            fForegroundImage = loadImage(fConfig.fg);
        }

        if (!fontMgr) {
            fontMgr = ToolUtils::MakePortableFontMgr();
            typeface = fontMgr->matchFamilyStyle("monospace", {}); // Try a specific font
            if (!typeface) {
                ALOGE("Warning: Typeface not found, using default");
                typeface = fontMgr->matchFamilyStyle(nullptr, {}); // Fallback to default
            }

            if (!typeface) {
                ALOGE("Error: no typeface");
                return;
            }

            font = SkFont(typeface, 40);
        }

        if (fConfig.allocateUpfrontMB) {
            allocate(fConfig.allocateUpfrontMB);
        }
    }

    bool allocate(size_t sizeMB)
    {
        unsigned long long numBytesULL = sizeMB * 1024 * 1024;
        if (numBytesULL > std::numeric_limits<size_t>::max()) {
            ALOGE("Requested size is too large and exceeds address space limits");
            return false;
        }
        size_t numBytes = static_cast<size_t>(numBytesULL);

        void* block = malloc(numBytes);

        if (block) {
            // Touch the memory to increase likelihood of backing by physical pages
            memset(block, 0, numBytes);
            auto page_size = getpagesize();
            uint8_t *ptr = (uint8_t *)block;
            while (ptr - (uint8_t *)block < numBytes) {
                ptr[0] = 'a';
                ptr += page_size;
            }
            allocations.push_back(block);
            totalAllocatedMB += sizeMB;
            ALOGI("SUCCESS: Allocated and zeroed %zu MB. Total allocated: %zu MB", sizeMB, totalAllocatedMB);
        } else {
            ALOGE("FAILURE: Could not allocate %zu MB", sizeMB);
            return false;
        }

        return true;
    }

    void onPaint(SkSurface* surface) override {
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorBLACK);

        auto w = canvas->imageInfo().width();
        auto h = canvas->imageInfo().height();

        auto next_w = w - 100;
        auto next_h = h - 100;

        SkPaint fgPaint;
        fgPaint.setBlendMode(SkBlendMode::kSrcOver);
        if (!fForegroundImage) {
            fgPaint.setColor(fForegroundColor); // needs non opaque alpha
        }

        // Blend foreground
        for (int i = 0; i < fConfig.numBlends; ++i) {
            if (fForegroundImage) {
                canvas->drawImageRect(fForegroundImage,
                            SkRect::MakeXYWH((w - next_w) / 2, (h - next_h) / 2, next_w, next_h),
                            SkSamplingOptions(), &fgPaint);
            } else {
                canvas->drawRect(
                    SkRect::MakeXYWH((w - next_w) / 2, (h - next_h) / 2, next_w, next_h),
                    fgPaint);
            }

            next_w -= 100;
            next_h -= 100;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        if (fConfig.allocateStepMB > 0 && timestamp - lastAllocTS > fConfig.allocateIntervalMS) {
            // Allocate some memory
            if (allocate(fConfig.allocateStepMB)) {
                lastAllocTS = timestamp;
            } else {
                ALOGE("%lld allocation failure!", (long long)timestamp);
            }
        }

        // Render text overlay
        if (typeface) {
            SkString timeText = SkStringPrintf("%lld total allocated: %zu MB", (long long)timestamp, totalAllocatedMB);

            ALOGI("Rendering text: %s", timeText.c_str());

            SkPaint paint;
            paint.setColor(SK_ColorGREEN);
            paint.setAntiAlias(true);
            canvas->drawString(timeText.c_str(), 20, 300, font, paint);
        } else {
            ALOGE("No typeface, skipping text render");
        }

    }

    void onAttach(sk_app::Window* w) override { fParent = w; }
};


struct BlendApplication : public sk_app::Application {
    std::unique_ptr<sk_app::Window> fWindow;
    BlendLayer fLayer;
    double fNextTime = -DBL_MAX;

    BlendApplication(std::unique_ptr<sk_app::Window> win, const Config &config)
        : fWindow(std::move(win))
        , fLayer(config) {}

    bool init() {
        std::unique_ptr<skwindow::DisplayParams> displayParams;

        if (fLayer.getBackendType() == BackendType::kGraphiteVulkan_BackendType) {
            auto paramsBuilder = skwindow::GraphiteDisplayParamsBuilder();
            paramsBuilder.createProtectedNativeBackend(true);
            displayParams = paramsBuilder.build();
        } else {
            auto paramsBuilder = skwindow::DisplayParamsBuilder();
            paramsBuilder.createProtectedNativeBackend(true);
            displayParams = paramsBuilder.build();
        }

        fWindow->setRequestedDisplayParams(std::move(displayParams));

        fWindow->attach(fLayer.getBackendType());

        fLayer.init();
        fWindow->pushLayer(&fLayer);
        fWindow->setTitle("Protected Blend Test");
        fLayer.onResize(fWindow->width(), fWindow->height());

        fWindow->show();
        return true;
    }
    ~BlendApplication() override { fWindow->detach(); }

    void onIdle() override {
        double now = SkTime::GetNSecs();
        if (now >= fNextTime) {
            constexpr double kHalfPeriodNanoSeconds = 0.5 * 1e9;
            fNextTime = now + kHalfPeriodNanoSeconds;
            fWindow->inval();
        }
    }
};

sk_app::Application* sk_app::Application::Create(int argc, char** argv, void* dat) {
    std::unique_ptr<sk_app::Window> win(sk_app::Windows::CreateNativeWindow(dat));
    if (!win) {
        SK_ABORT("CreateNativeWindow failed.");
    }

    Config config;
    config.parseArgs(argc, argv);
    std::unique_ptr<BlendApplication> app(new BlendApplication(std::move(win), config));

    (void)app->init();

    return app.release();
}
