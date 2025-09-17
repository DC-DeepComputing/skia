/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "tools/DecodeUtils.h"
#include "tools/ToolUtils.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/codec/SkPngRustDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"

static void draw_small_circle(SkCanvas* canvas, const SkPaint& p, const float radius) {
    SkRect oval = SkRect::MakeLTRB(radius * -1, radius * -1, radius, radius);
    constexpr bool kUseCenter = false;
    constexpr float kDegrees = 360;
    canvas->drawArc(oval, 0, kDegrees, kUseCenter, p);
}

DEF_SIMPLE_GM(smallcircles, canvas, 425, 425) {
    auto surface = canvas->makeSurface(SkImageInfo::MakeN32Premul(100, 100));
    if (!surface) {
      surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100));
    }
    SkCanvas* canv = surface->getCanvas();

    canv->translate(5, 5);
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kFill_Style);

    // Draw circles
    for (int i = 0; i < 11; ++i) {
      canv->save();
      for (int j = 0; j < 11; ++j) {
        draw_small_circle(canv, p, 0.8f);
        canv->translate(5.1f, 0.f);
      }
      canv->restore();
      canv->translate(0.f, 5.1f);
    }

    // Scale up image of small circles
    canvas->scale(7, 7);
    auto img = surface->makeImageSnapshot();
    canvas->drawImage(img, 0, 0);
}

DEF_SIMPLE_GM(imageshader2_tinyscale, canvas, 1000, 1000) {
  // 128x128px image with red/black/green/blue quadrants.
  // auto img = ToolUtils::GetResourceAsImage("images/gainmap_gcontainer_only.jpg");
  const int SOURCE_WIDTH = 500;
  const int SOURCE_HEIGHT = 40000;
  SkImageInfo source_info = SkImageInfo::MakeN32Premul(SOURCE_WIDTH, SOURCE_HEIGHT);

  auto surf =  SkSurfaces::Raster(source_info);
  SkCanvas* canv = surf->getCanvas();

  SkPoint pts[] = {SkPoint::Make(0, 0), SkPoint::Make(0, SOURCE_HEIGHT)};
  SkColor4f colors[] = {SkColor4f::FromColor(SK_ColorCYAN), SkColor4f::FromColor(SK_ColorMAGENTA)};
  sk_sp<SkShader> gradient_shader = SkGradientShader::MakeLinear(
      pts,
      colors,
      nullptr,
      nullptr,
      2,
      SkTileMode::kClamp,
      0
  );

  SkPaint paint;
  paint.setShader(gradient_shader);

  canv->drawRect(SkRect::MakeXYWH(0, 0, SOURCE_WIDTH, SOURCE_HEIGHT), paint);

  // Create an immutable source image.
  sk_sp<SkImage> img = surf->makeImageSnapshot();


  // A small scale amplifies the sampling coords in image space.
  static constexpr float kScale = 0.01f;
  const auto m = /*SkMatrix::Translate(500, 500) **/
                 SkMatrix::Scale(1, 0.0125);

  // In clamp mode we should see no repeating patterns, just the viewport filled
  // with four-colored quadrants.
  SkPaint p;
  p.setShader(img->makeShader(SkTileMode::kClamp,
                              SkTileMode::kClamp,
                              SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
                              &m));

  // auto surface = canvas->makeSurface(SkImageInfo::Make(1000, 1000,  kN32_SkColorType, kUnpremul_SkAlphaType));
  // if (!surface) {
  //     return;
  // }
  // SkCanvas* canv = surface->getCanvas();
  // canv->drawPaint(p);
  // auto img2 = surface->makeImageSnapshot();

  // canvas->drawImage(img, 0, 0);
    canvas->drawPaint(p);
}

DEF_SIMPLE_GM(pngcodectesting, canvas, 1000, 1000) {
    // canvas->clear(SK_ColorBLACK);
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/interlaced-multiframe-with-blending.png"));
    if (!stream) {
      SkDebugf("no stream\n");
        return;
    }

    std::unique_ptr<SkCodec> codec;
    size_t length = stream->getLength();
    codec = SkCodec::MakeFromStream(std::move(stream));
    if (!codec) {
        SkDebugf("Unable to decode\n");
        return;
    }

    const SkImageInfo info = codec->getInfo();

    SkBitmap bm;
    bm.allocPixels(info);

    for (int i = 0; i < 4; ++i) {

      SkCodec::Options options;
      options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
      options.fSubset = nullptr;
      options.fFrameIndex = i;  // We want to decode the second frame.
      options.fPriorFrame = i-1;  // `pixmap` contains the first frame before `getPixels` call.

      codec->startIncrementalDecode(info, bm.getPixels(),
      bm.rowBytes() , &options);

       codec->incrementalDecode();

    }

    canvas->drawImage(bm.asImage(), 0, 0);

//     std::unique_ptr<SkStream> stream2(GetResourceAsStream("images/plane_interlaced.png"));
//     if (!stream2) {
//       SkDebugf("no stream2\n");
//         return;
//     }

//     std::unique_ptr<SkCodec> codec2;
//     codec2 = SkCodec::MakeFromStream(std::move(stream2));
//     if (!codec2) {
//         SkDebugf("Unable to decode\n");
//         return;
//     }

//     const SkImageInfo info2 = codec2->getInfo().makeColorType(kN32_SkColorType);

//     SkIRect subset2 = SkIRect::MakeLTRB(info2.width()/2, 0, info2.width(), info2.height());
// SkCodec::Options options2;
// options2.fSubset = &subset2;


//     SkBitmap bm2;
//     bm2.allocPixels(info2);

//      codec2->startIncrementalDecode(info2, bm2.getPixels(),
//     bm2.rowBytes() , &options2);

//      codec2->incrementalDecode();


//     canvas->drawImage(bm2.asImage(), info.width(), 0);

//     // SkImageInfo info565 = info.makeColorType(kRGB_565_SkColorType);
//     // if (info.alphaType() == kOpaque_SkAlphaType) {
//     //     // Decoding to 565 should succeed.
//     //     SkBitmap bm565;
//     //     bm565.allocPixels(info565);

//     //     // This will allow comparison even if the image is incomplete.
//     //     bm565.eraseColor(SK_ColorBLACK);

//     //     auto actualResult = codec->getPixels(info565, bm565.getPixels(), bm565.rowBytes());

//     // }


//     // SkBitmap bm2;
//     // bm2.allocPixels(info);

//     // codec->getPixels(info, bm2.getPixels(), bm2.rowBytes());
//     // canvas->drawImage(bm2.asImage(), bm.width(), 0);

//     // // TEST IN STRIPES //////////////////////////////////////////////////////

//     SkBitmap tiledBM;
//     tiledBM.allocPixels(info);

//     const int height = info.height();
//     const int width = info.width();
//     // Note that if numStripes does not evenly divide height there will be an extra
//     // stripe.
//     const int numStripes = 4;
//     const int numVerticalStripes = 2;

//     if (numStripes > height || numVerticalStripes > width) {
//         // Image is too small.
//         return;
//     }

//     const int stripeHeight = height / numStripes;
//     const int stripeWidth = width / numVerticalStripes;

//     // Iterate through the image twice. Once to decode odd stripes, and once for even.
//     for (int oddEven = 1; oddEven >= 0; oddEven--) {
//         for (int y = oddEven * stripeHeight; y < height; y += 2 * stripeHeight) {
//             for (int xStripe = 0; xStripe < numVerticalStripes; xStripe++) {
//             // int xStripe = 1;
//                 // Calculate all four bounds for the grid section
//                 const int top = y;
//                 const int bottom = std::min(y + stripeHeight, height);
//                 const int left = xStripe * stripeWidth;
//                 const int right = std::min((xStripe + 1) * stripeWidth, width);

//                 SkIRect subset = SkIRect::MakeLTRB(left, top, right, bottom);
//                 SkDebugf("%d %d %d %d\n", left, top, right, bottom);
//                 SkCodec::Options options7s;
//                 options7s.fSubset = &subset;
//                 codec->startIncrementalDecode(
//                   info, tiledBM.getAddr(left, top), tiledBM.rowBytes(), &options7s);
//                 // REPORTER_ASSERT(r, SkCodec::kSuccess == codec->startIncrementalDecode(
//                     // info, tiledBM.getAddr(left, top), tiledBM.rowBytes(), &options));
//                 // REPORTER_ASSERT(r, SkCodec::kSuccess != codec->incrementalDecode());
//                 codec->incrementalDecode();
//             }
//         }
//     }
//     // SkDebugf("drawing third thingy\n");
//     canvas->drawImage(tiledBM.asImage(), bm.width()*2, 0);


//     // canvas->drawImage(bm3.asImage(), 0, 0);

//     // SkBitmap bm4;
//     // bm4.allocPixels(info.makeAlphaType(kPremul_SkAlphaType));

//     // codec->getPixels(info.makeAlphaType(kPremul_SkAlphaType), bm4.getPixels(), bm4.rowBytes());
//     // canvas->drawImage(bm4.asImage(), bm.width()*3, 0);


}

std::unique_ptr<SkCodec> SkPngRustDecoderDecode(const char* path) {
  sk_sp<SkData> data = GetResourceAsData(path);
  if (!data) {
      return nullptr;
  }

  SkCodec::Result result;
  std::unique_ptr<SkCodec> codec =
          SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);

  return codec;
}

sk_sp<SkImage> DecodeLastFrame( SkCodec* codec) {
  int frameCount = codec->getFrameCount();
  sk_sp<SkImage> image;
  SkCodec::Result result = SkCodec::kSuccess;
  for (int i = 0; i < frameCount; i++) {
      SkCodec::FrameInfo info;

      // This test method only supports `kKeep` disposal method.
      SkASSERT(info.fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);

      if (!image) {
          std::tie(image, result) = codec->getImage();
          if (result != SkCodec::kSuccess) {
              return nullptr;
          }
      } else {
          SkPixmap pixmap;

          SkCodec::Options options;
          options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
          options.fSubset = nullptr;
          options.fFrameIndex = i;
          options.fPriorFrame = i - 1;

          result = codec->getPixels(pixmap, &options);
          if (result != SkCodec::kSuccess) {
              return nullptr;
          }
      }
  }

  return image;
}

DEF_SIMPLE_GM(apngtesting, canvas, 1000, 1000) {
  std::unique_ptr<SkCodec> codec =
     SkPngRustDecoderDecode("images/apng-test-suite--regions--dispose-op-none.png");
  if (!codec) {
  return;
  }
  // auto [image, result] = codec->getImage();
  // canvas->drawImage(image, 0, 0);

  // SkBitmap bm;
  // bm.allocPixels(codec->getInfo());
  // SkCodec::Options options;
  //   options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
  //   options.fSubset = nullptr;
  //   options.fFrameIndex = 1;  // We want to decode the second frame.
  //   options.fPriorFrame = 0;  // `pixmap` contains the first frame before `getPixels` call.
  //   codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes(), &options);
  //   canvas->drawImage(bm.asImage(), codec->getInfo().width(), 0);

  //   options.fFrameIndex = 2;  // We want to decode the second frame.
  //   options.fPriorFrame = 1;  // `pixmap` contains the second frame before `getPixels` call.
  //   codec->getPixels(codec->getInfo(), bm.getPixels(), bm.rowBytes(), &options);
  //   canvas->drawImage(bm.asImage(), codec->getInfo().width()*2, 0);
  // sk_sp<SkImage> img = DecodeLastFrame(codec.get());

  // canvas->drawImage(img, 0, 0);


}
