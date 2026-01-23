# Task: Debugging SkJpegCodec Scaling State Mismatch (Heap Buffer Overflow)

## The Problem
A reported vulnerability (refer to `@vuln.txt`) describes a heap buffer overflow in `SkJpegCodec`. The root cause is a "scaling state mismatch":
1. The user requests a scaled decode (e.g., 1/2 scale).
2. Skia calls `onDimensionsSupported`, which sets `scale_num` and `scale_denom` in the `libjpeg-turbo` decompress struct.
3. A "rewind" occurs (e.g., because of multiple `getPixels` calls or a transition between metadata querying and decoding).
4. `SkJpegCodec::onRewind` destroys the existing `JpegDecoderMgr` and creates a fresh one.
5. The fresh `libjpeg-turbo` decompress struct defaults to **1:1 scaling**.
6. If the scale is not re-applied, `libjpeg-turbo` writes full-sized scanlines into a buffer allocated for a scaled-down image, causing an OOB write.

## Important Files for Context
*   **The Buggy Code:** `@src/codec/SkJpegCodec.cpp` / `@src/codec/SkJpegCodec.h`
*   **The Base Logic:** `@src/codec/SkCodec.cpp` / `@include/codec/SkCodec.h`
*   **Wrapper Logic:** `@src/codec/SkAndroidCodec.cpp` / `@src/codec/SkSampledCodec.cpp`
*   **The Reproduction Test:** `@tests/CodecTest.cpp` (Search for `Codec_jpeg_rewind_scale`)
*   **The Vulnerability Report:** `vuln.txt` (Detailed analysis of the fuzzer crash)

## Current State & Observations
*   **The Logic:** The vulnerability exists because `onRewind` replaces the decoder manager without preserving the scaling state.
*   **The Difficulty:** Modern Skia wrappers (`SkAndroidCodec`, `SkSampledCodec`) are very "helpful." In the test cases tried so far (`getAndroidPixels`), these wrappers call `dimensionsSupported` *after* the rewind has already happened, effectively "fixing" the state mismatch before `onGetPixels` is reached.
*   **Logging Verified:**
    *   `onRewind` does indeed reset the scale to 1/1.
    *   In `SkAndroidCodec::getAndroidPixels` paths, `onDimensionsSupported` is usually called again immediately after the rewind.

## What Has Been Attempted
1.  **Logging:** Added `SkDebugf` to `onDimensionsSupported`, `onRewind`, `onGetPixels`, and `readRows` to track the address of `fDecoderMgr` and the current `scale_num/denom`.
2.  **Reproduction Test:** Created `DEF_TEST(Codec_jpeg_rewind_scale, r)` in `tests/CodecTest.cpp`. It uses `SkAndroidCodec` to perform two consecutive decodes at different scales. 
3.  **Proposed Fix:** Added `fScaleNum` and `fScaleDenom` members to `SkJpegCodec` to store the last successful scale and restore it in `onRewind`.

## Things Verified It Is NOT (in current test paths)
*   It is not a simple "first decode" failure. The first decode sets the state correctly.
*   The `SkAndroidCodec::getAndroidPixels` path seems to re-validate dimensions frequently, making it hard to catch the codec in an inconsistent state using only that public API.

## Leads Not Yet Checked (The "Smoking Gun")
1.  **Scanline Decoding:** The `vuln.txt` specifically highlights `SkJpegCodec::onStartScanlineDecode`. Does the scanline path skip the "helpful" re-validation found in `getAndroidPixels`? 
2.  **`SkSampledCodec::sampledDecode`:** Investigate if this path triggers a rewind but fails to call `dimensionsSupported` on the underlying `fCodec` before starting the scanline or incremental decode.
3.  **Incremental Decoding:** Similar to scanline decoding, check if the state is preserved between `startIncrementalDecode` and `incrementalDecode` if a rewind happens in between.
4.  **Raw Data/YUV Path:** Check `onGetYUVAPlanes`. It calls `rewindIfNeeded` and then `jpeg_start_decompress` with `raw_data_out = TRUE`.
5.  **Direct `onGetPixels` calls:** Is there a way to trigger `onRewind` such that the very next call is `onGetPixels` (or `onStartScanlineDecode`) without an intermediate `dimensionsSupported` call? 

## To compile and test
`ninja -C out/Default/ dm out/Default/dm --match Codec_jpeg_rewind_scale`

## Goal
Prove the vulnerability exists by triggering a `readRows` call where `fDecoderMgr->dinfo()->output_width` is larger than the width expected by the `SkImageInfo` passed to the codec. Once proven with a test/log, apply the state preservation fix.

