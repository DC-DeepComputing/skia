/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegSourceMgr_codec_DEFINED
#define SkJpegSourceMgr_codec_DEFINED

#include <cstddef>
#include <cstdint>
#include <memory>

class SkStream;
struct SkJpegSegment;

/*
 * Interface to adapt an SkStream to the jpeg_source_mgr interface. This interface has different
 * implementations for SkStreams with different capabilities.
 */
class SkJpegSourceMgr {
public:
    static std::unique_ptr<SkJpegSourceMgr> Make(SkStream* stream);
    virtual ~SkJpegSourceMgr() {}

    // Interface called by libjpeg via its jpeg_source_mgr interface.
    virtual void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) = 0;
    virtual bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) = 0;
    virtual bool skipInputBytes(size_t bytes,
                                const uint8_t*& nextInputByte,
                                size_t& bytesInBuffer) = 0;

    // Parse this stream all the way through its EndOfImage marker and return the list of segments.
    // Return false if there is an error or if no EndOfImage marker is found.
    virtual const std::vector<SkJpegSegment>& getAllSegments() = 0;

    // Return a stream for the subset of this source's stream with the specified offset and size.
    virtual std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size) = 0;
};

#endif
