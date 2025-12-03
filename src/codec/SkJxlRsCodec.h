/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJxlRsCodec_DEFINED
#define SkJxlRsCodec_DEFINED

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "rust/jxl/FFI.rs.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkScalingCodec.h"
#include "third_party/rust/cxx/v1/cxx.h"

class SkCodec;
class SkStream;
struct SkEncodedInfo;
struct SkImageInfo;

/**
 * This class implements the decoding for JPEG XL images using the jxl-rs
 * Rust decoder.
 */
class SkJxlRsCodec : public SkScalingCodec {
public:
    /**
     * Checks if the data begins with a valid JXL signature.
     */
    static bool IsJxl(const void* data, size_t length);

    /**
     * Assumes IsJxl was called and returned true.
     * Takes ownership of the stream.
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:
    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kJPEGXL;
    }

    Result onGetPixels(const SkImageInfo& dstInfo,
                       void* dst,
                       size_t rowBytes,
                       const Options& options,
                       int* rowsDecodedPtr) override;

    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                    void* dst,
                                    size_t rowBytes,
                                    const Options& options) override;

    Result onIncrementalDecode(int* rowsDecoded) override;

    bool onRewind() override;

    bool conversionSupported(const SkImageInfo&, bool, bool) override;

    int onGetFrameCount() override;

    bool onGetFrameInfo(int index, FrameInfo* info) const override;

    int onGetRepetitionCount() override;

    IsAnimated onIsAnimated() override;

private:
    const SkFrameHolder* getFrameHolder() const override;

    /**
     * Frame information for animation support.
     */
    struct JxlFrameInfo {
        rust_jxl::FrameHeader header;
        bool received = false;
    };

    class FrameHolder final : public SkFrameHolder {
    public:
        FrameHolder(int width, int height);
        ~FrameHolder() override;

        FrameHolder(const FrameHolder&) = delete;
        FrameHolder(FrameHolder&&) = delete;
        FrameHolder& operator=(const FrameHolder&) = delete;
        FrameHolder& operator=(FrameHolder&&) = delete;

        void appendFrame(const JxlFrameInfo& info, bool hasAlpha);
        void markFrameAsFullyReceived(size_t index);
        size_t size() const;
        bool getFrameInfo(int index, FrameInfo* info) const;

    private:
        class JxlFrame;

        const SkFrame* onGetFrame(int index) const override;

        std::vector<JxlFrame> fFrames;
    };

    SkJxlRsCodec(SkEncodedInfo&& info,
                 std::unique_ptr<SkStream> stream,
                 sk_sp<const SkData> data,
                 rust_jxl::BasicInfo basicInfo);

    // Scan through the file to discover all frames.
    bool scanFrames();

    // Decode a specific frame.
    Result decodeFrame(int frameIndex, void* dst, size_t rowBytes, const SkImageInfo& dstInfo);

    // The Rust decoder.
    rust::Box<rust_jxl::Decoder> fDecoder;

    // Full image data (needed for rewinding).
    sk_sp<const SkData> fData;

    // Cached basic info from decoder.
    rust_jxl::BasicInfo fBasicInfo;

    // Frame information for animation.
    FrameHolder fFrameHolder;

    // Number of frames we've decoded so far.
    int fNumDecodedFrames = 0;

    // Whether we've scanned the full file for all frames.
    bool fAllFramesScanned = false;

    // Whether this is a high bit depth image.
    bool fIsHighBitDepth = false;

    // State for incremental decoding.
    void* fIncrementalDst = nullptr;
    size_t fIncrementalRowBytes = 0;
    int fIncrementalFrameIndex = 0;
    bool fIncrementalDecodeStarted = false;
    SkImageInfo fIncrementalDstInfo;

    using INHERITED = SkScalingCodec;
};

#endif  // SkJxlRsCodec_DEFINED
