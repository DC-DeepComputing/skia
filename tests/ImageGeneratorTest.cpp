/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "src/base/SkAutoMalloc.h"
#include "src/image/SkImageGeneratorPriv.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"
#include "tools/EncodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    #include "include/ports/SkImageGeneratorCG.h"
#elif defined(SK_BUILD_FOR_WIN)
    #include "include/ports/SkImageGeneratorWIC.h"
#endif

static constexpr int kImgW = 100;
static constexpr int kImgH =  80;

static bool gMyFactoryWasCalled;

// NOLINTNEXTLINE(performance-unnecessary-value-param)
static std::unique_ptr<SkImageGenerator> my_factory(sk_sp<SkData>) {
    gMyFactoryWasCalled = true;
    return nullptr;
}

static void test_imagegenerator_factory(skiatest::Reporter* reporter) {
    // just need a non-empty data to test things
    sk_sp<SkData> data(SkData::MakeWithCString("test_imagegenerator_factory"));

    gMyFactoryWasCalled = false;

    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    std::unique_ptr<SkImageGenerator> gen = SkImageGenerators::MakeFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, !gMyFactoryWasCalled);

    // Test is racy, in that it hopes no other thread is changing this global...
    auto prev = SkGraphics::SetImageGeneratorFromEncodedDataFactory(my_factory);
    gen = SkImageGenerators::MakeFromEncoded(data);
    REPORTER_ASSERT(reporter, nullptr == gen);
    REPORTER_ASSERT(reporter, gMyFactoryWasCalled);

    // This just verifies that the signatures match.
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(SkImageGeneratorCG::MakeFromEncodedCG);
#elif defined(SK_BUILD_FOR_WIN)
    SkGraphics::SetImageGeneratorFromEncodedDataFactory(SkImageGeneratorWIC::MakeFromEncodedWIC);
#endif

    SkGraphics::SetImageGeneratorFromEncodedDataFactory(prev);
}

class MyImageGenerator : public SkImageGenerator {
public:
    MyImageGenerator() : SkImageGenerator(SkImageInfo::MakeN32Premul(0, 0)) {}
};

DEF_TEST(ImageGenerator, reporter) {
    MyImageGenerator ig;
    SkYUVAPixmapInfo yuvaPixmapInfo;

    // Check that the YUV decoding API does not cause any crashes
    ig.queryYUVAInfo(SkYUVAPixmapInfo::SupportedDataTypes::All(), &yuvaPixmapInfo);
    SkYUVAInfo yuvaInfo({250, 250},
                        SkYUVAInfo::PlaneConfig::kY_UV,
                        SkYUVAInfo::Subsampling::k420,
                        kJPEG_Full_SkYUVColorSpace);
    yuvaPixmapInfo = SkYUVAPixmapInfo(yuvaInfo,
                                      SkYUVAPixmapInfo::DataType::kUnorm8,
                                      /*rowBytes[]*/ nullptr);
    SkYUVAPixmaps yuvaPixmaps = SkYUVAPixmaps::Allocate(yuvaPixmapInfo);
    ig.getYUVAPlanes(yuvaPixmaps);

    // Suppressed due to https://code.google.com/p/skia/issues/detail?id=4339
    if ((false)) {
        test_imagegenerator_factory(reporter);
    }
}

static sk_sp<SkPicture> make_picture() {
    SkPictureRecorder recorder;
    recorder.beginRecording(100, 100)->drawColor(SK_ColorRED);
    return recorder.finishRecordingAsPicture();
}

DEF_TEST(PictureImageGenerator, reporter) {
    const struct {
        SkColorType fColorType;
        SkAlphaType fAlphaType;
    } recs[] = {
        { kRGBA_8888_SkColorType, kPremul_SkAlphaType },
        { kBGRA_8888_SkColorType, kPremul_SkAlphaType },
        { kRGBA_F16_SkColorType,  kPremul_SkAlphaType },
        { kRGBA_F32_SkColorType,  kPremul_SkAlphaType },
        { kRGBA_1010102_SkColorType, kPremul_SkAlphaType },

        { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType },
        { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType },
        { kRGBA_F16_SkColorType,  kUnpremul_SkAlphaType },
        { kRGBA_F32_SkColorType,  kUnpremul_SkAlphaType },
        { kRGBA_1010102_SkColorType, kUnpremul_SkAlphaType },
    };

    auto colorspace = SkColorSpace::MakeSRGB();
    auto picture = make_picture();
    auto gen = SkImageGenerators::MakeFromPicture(
            {100, 100}, picture, nullptr, nullptr, SkImages::BitDepth::kU8, colorspace);

    // worst case for all requests
    SkAutoMalloc storage(100 * 100 * SkColorTypeBytesPerPixel(kRGBA_F32_SkColorType));

    for (const auto& rec : recs) {
        SkImageInfo info = SkImageInfo::Make(100, 100, rec.fColorType, rec.fAlphaType, colorspace);
        REPORTER_ASSERT(reporter, gen->getPixels(info, storage.get(), info.minRowBytes()));
    }
}

DEF_TEST(TestingOrientation, reporter) {
  sk_sp<SkImage> image;

 // Get the image as data
  sk_sp<SkData> data(GetResourceAsData("images/exif-orientation-2-ur.jpg"));
  REPORTER_ASSERT(reporter, nullptr != data);

  #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
      image = SkImages::DeferredFromGenerator(SkImageGeneratorCG::MakeFromEncodedCG(data));
  #elif defined(SK_BUILD_FOR_WIN)
      image = SkImages::DeferredFromGenerator(SkImageGeneratorWIC::MakeFromEncodedWIC(data));
  #endif
  REPORTER_ASSERT(reporter, nullptr != image);

  SkBitmap fromGenerator;
  SkImageInfo imageInfo = image->imageInfo();
    if (!fromGenerator.tryAllocPixels(imageInfo, 0)) {
        ERRORF(reporter, "Failed to allocate pixels for fromGenerator bitmap");
        return;
  }

//   SkPixmap pm;
//   SkISize size{imageInfo.width(), imageInfo.height()};
//   auto surf = SkSurfaces::Raster(
//                 SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType));

//   auto* canv = surf->getCanvas();
//   canv->drawImage(image, 0, 0);
//   surf->makeImageSnapshot()->peekPixels(&pm);
//   ToolUtils::EncodeImageToPngFile("generator.png", pm);

  // Get image from SkCodec
  std::unique_ptr<SkStream> stream(GetResourceAsStream("images/exif-orientation-2-ur.jpg"));
  REPORTER_ASSERT(reporter, nullptr != stream);
  if (!stream) {
      return;
  }
  std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
  REPORTER_ASSERT(reporter, nullptr != codec);

  auto info = codec->getInfo();
  SkBitmap fromCodec;
  if (!fromCodec.tryAllocPixels(info, 0)) {
    ERRORF(reporter, "Failed to allocate pixels for fromGenerator bitmap");
    return;
  }

   for (int i = 0; i < info.width();  ++i) {
        for (int j = 0; j < info.height(); ++j) {
            SkColor c1 = *fromGenerator   .getAddr32(i, j);
            SkColor c2 = *fromCodec.getAddr32(i, j);
            if (c1 != c2) {
                ERRORF(reporter, "Bitmaps do not match starting at position %i, %i\n"
                          "\tfromGenerator: %x\tfromCodec: %x", i, j,
                          c1, c2);
                return;
            }
        }
    }
}
