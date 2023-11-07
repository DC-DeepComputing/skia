// /*
//  * Copyright 2023 Google LLC
//  *
//  * Use of this source code is governed by a BSD-style license that can be
//  * found in the LICENSE file.
//  */

// #include "tools/gpu/vk/AHardwareBufferTestUtils.h"

// #if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26 && defined(SK_VULKAN)

// // namespace {
// SkPMColor GetSrcColor(int x, int y) {
//     SkASSERT(x >= 0 && x < DEV_W);
//     SkASSERT(y >= 0 && y < DEV_H);

//     U8CPU r = x;
//     U8CPU g = y;
//     U8CPU b = 0xc;

//     U8CPU a = 0xff;
//     switch ((x+y) % 5) {
//         case 0:
//             a = 0xff;
//             break;
//         case 1:
//             a = 0x80;
//             break;
//         case 2:
//             a = 0xCC;
//             break;
//         case 4:
//             a = 0x01;
//             break;
//         case 3:
//             a = 0x00;
//             break;
//     }
//     a = 0xff;
//     return SkPremultiplyARGBInline(a, r, g, b);
// }

// CleanupResources(AHardwareBuffer* buffer) {
//     if (buffer) {
//         AHardwareBuffer_release(buffer);
//     }
// }

// SkBitmap MakeSrcBitmap() {
//     static SkBitmap bmp;
//     if (bmp.isNull()) {
//         bmp.allocN32Pixels(DEV_W, DEV_H);
//         intptr_t pixels = reinterpret_cast<intptr_t>(bmp.getPixels());
//         for (int y = 0; y < DEV_H; ++y) {
//             for (int x = 0; x < DEV_W; ++x) {
//                 SkPMColor* pixel = reinterpret_cast<SkPMColor*>(
//                         pixels + y * bmp.rowBytes() + x * bmp.bytesPerPixel());
//                 *pixel = get_src_color(x, y);
//             }
//         }
//     }
//     return bmp;
// }

// bool CheckRead(const SkBitmap& expectedBitmap, const SkBitmap& actualBitmap,
//                skiatest::Reporter* reporter) {
//     bool result = true;
//     for (int y = 0; y < DEV_H && result; ++y) {
//         for (int x = 0; x < DEV_W && result; ++x) {
//             const uint32_t srcPixel = *expectedBitmap.getAddr32(x, y);
//             const uint32_t dstPixel = *actualBitmap.getAddr32(x, y);
//             if (srcPixel != dstPixel) {
//                 ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
//                        x, y,  srcPixel, dstPixel);
//                 result = false;
//             }/* else {
//                 SkDebugf("Got good pixel (%d, %d) value 0x%08x, got 0x%08x.\n",
//                        x, y,  srcPixel, dstPixel);
//             }*/
//         }
//     }
//     return result;
// }

// void SetupAHardwareBuffer(AHardwareBuffer* buffer, AHardwareBuffer_Desc* hwbDesc,
//                           skiatest::Reporter* reporter) {
//     hwbDesc->width = DEV_W;
//     hwbDesc->height = DEV_H;
//     hwbDesc->layers = 1;
//     hwbDesc->usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
//                      AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
//                      AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
//                      AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;

//     hwbDesc->format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
//     // The following three are not used in the allocate
//     hwbDesc->stride = 0;
//     hwbDesc->rfu0= 0;
//     hwbDesc->rfu1= 0;

//     if (int error = AHardwareBuffer_allocate(hwbDesc, &buffer)) {
//         ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
//         cleanup_resources(buffer);
//         return;
//     }

//     // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
//     AHardwareBuffer_describe(buffer, hwbDesc);
// }

// void PopulateBitmap(const SkBitmap& srcBitmap, AHardwareBuffer* buffer, uint32_t stride,
//                     skgpu::Origin origin, skiatest::Reporter* reporter) {
//     uint32_t* bufferAddr;
//     if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
//                              reinterpret_cast<void**>(&bufferAddr))) {
//         ERRORF(reporter, "Failed to lock hardware buffer");
//         cleanup_resources(buffer);
//         return;
//     }

//     int bbp = srcBitmap.bytesPerPixel();
//     uint32_t* src = (uint32_t*)srcBitmap.getPixels();
//     int nextLineStep = DEV_W;
//     if (origin == skgpu::Origin::kBottomLeft) {
//         nextLineStep = -nextLineStep;
//         src += (DEV_H - 1) * DEV_W;
//     }
//     uint32_t* dst = bufferAddr;
//     for (int y = 0; y < DEV_H; ++y) {
//         memcpy(dst, src, DEV_W * bbp);
//         src += nextLineStep;
//         dst += stride;
//     }
//     AHardwareBuffer_unlock(buffer, nullptr);
// }

// void DrawImageAndReadBack(const SkBitmap& srcBitmap, SkSurface& surface,
//                           skiatest::Reporter* reporter) {
//     surface.getCanvas()->drawImage(srcBitmap.asImage(), 0, 0);

//     SkBitmap readbackBitmap;
//     readbackBitmap.allocN32Pixels(DEV_W, DEV_H);

//     REPORTER_ASSERT(reporter, surface.readPixels(readbackBitmap, 0, 0));
//     REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, readbackBitmap));
// }

// //} // namespace testj
// #endif // defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26 && defined(SK_VULKAN)

