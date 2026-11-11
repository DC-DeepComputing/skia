/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkDeflate.h"

#include "include/core/SkSpan.h"
#include "include/private/SkAssert.h"
#include "include/private/SkAttributes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTo.h"
#include "src/core/SkTraceEvent.h"
#include "src/partition_alloc/raw_ptr.h"

#include <zlib.h>
#include <algorithm>
#include <array>
#include <cstdint>

namespace {

// Different zlib implementations use different T.
// We've seen size_t and unsigned.
template <typename T> void* skia_alloc_func(void*, T items, T size) {
    if (!SkTFitsIn<size_t>(size)) {
        return nullptr;
    }
    const size_t maxItems = SIZE_MAX / size;
    if (maxItems < items) {
        return nullptr;
    }
    return sk_calloc_throw(SkToSizeT(items) * SkToSizeT(size));
}

void skia_free_func(void*, void* address) { sk_free(address); }

}  // namespace

#define SKDEFLATEWSTREAM_INPUT_BUFFER_SIZE 4096
#define SKDEFLATEWSTREAM_OUTPUT_BUFFER_SIZE 4224  // 4096 + 128, usually big
                                                  // enough to always do a
                                                  // single loop.

// called by both write() and finalize()
static void do_deflate(int flush,
                       z_stream* zStream,
                       SkWStream* out,
                       SkSpan<unsigned char> in) {
    zStream->next_in = in.data();
    zStream->avail_in = SkToInt(in.size());
    std::array<unsigned char, SKDEFLATEWSTREAM_OUTPUT_BUFFER_SIZE> outBuffer;
    SkDEBUGCODE(int returnValue;)
    do {
        zStream->next_out = outBuffer.data();
        zStream->avail_out = outBuffer.size();
        SkDEBUGCODE(returnValue =) deflate(zStream, flush);
        SkASSERT(!zStream->msg);

        out->write(outBuffer.data(), outBuffer.size() - zStream->avail_out);
    } while (zStream->avail_in || !zStream->avail_out);
    SkASSERT(flush == Z_FINISH
                 ? returnValue == Z_STREAM_END
                 : returnValue == Z_OK);
}

// Hide all zlib impl details.
struct SkDeflateWStream::Impl {
    raw_ptr<SkWStream> fOut;
    std::array<unsigned char, SKDEFLATEWSTREAM_INPUT_BUFFER_SIZE> fInBuffer;
    size_t fInBufferIndex;
    z_stream fZStream;
};

SkDeflateWStream::SkDeflateWStream(SkWStream* out,
                                   int compressionLevel,
                                   bool gzip)
    : fImpl(std::make_unique<SkDeflateWStream::Impl>()) {

    // There has existed at some point at least one zlib implementation which thought it was being
    // clever by randomizing the compression level. This is actually not entirely incorrect, except
    // for the no-compression level which should always be deterministically pass-through.
    // Users should instead consider the zero compression level broken and handle it themselves.
    SkASSERT(compressionLevel != 0);

    fImpl->fOut = out;
    fImpl->fInBufferIndex = 0;
    if (!fImpl->fOut) {
        return;
    }
    fImpl->fZStream.next_in = nullptr;
    fImpl->fZStream.zalloc = &skia_alloc_func;
    fImpl->fZStream.zfree = &skia_free_func;
    fImpl->fZStream.opaque = nullptr;
    SkASSERT(compressionLevel <= 9 && compressionLevel >= -1);
    SkDEBUGCODE(int r =) deflateInit2(&fImpl->fZStream, compressionLevel,
                                      Z_DEFLATED, gzip ? 0x1F : 0x0F,
                                      8, Z_DEFAULT_STRATEGY);
    SkASSERT(Z_OK == r);
}

SkDeflateWStream::~SkDeflateWStream() { this->finalize(); }

void SkDeflateWStream::finalize() {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (!fImpl->fOut) {
        return;
    }
    do_deflate(Z_FINISH,
               &fImpl->fZStream,
               fImpl->fOut,
               SkSpan(fImpl->fInBuffer).first(fImpl->fInBufferIndex));
    (void)deflateEnd(&fImpl->fZStream);
    fImpl->fOut = nullptr;
}

bool SkDeflateWStream::write(const void* void_buffer, size_t len) {
    // TODO(crbug.com/582862856): SkWStream::write is a (ptr, len) virtual; the caller guarantees
    // void_buffer holds len bytes.
    return this->write(SK_UNSAFE_TODO(SkSpan(static_cast<const uint8_t*>(void_buffer), len)));
}

bool SkDeflateWStream::write(SkSpan<const uint8_t> input) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (!fImpl->fOut) {
        return false;
    }
    while (!input.empty()) {
        size_t tocopy = std::min(input.size(), fImpl->fInBuffer.size() - fImpl->fInBufferIndex);
        SkSpan<unsigned char> dst = SkSpan(fImpl->fInBuffer).subspan(fImpl->fInBufferIndex, tocopy);
        SkSpan<const uint8_t> src = input.first(tocopy);
        SkASSERT(src.size() == dst.size());
        std::copy(src.begin(), src.end(), dst.begin());
        input = input.subspan(tocopy);
        fImpl->fInBufferIndex += tocopy;
        SkASSERT(fImpl->fInBufferIndex <= fImpl->fInBuffer.size());

        // if the buffer isn't filled, don't call into zlib yet.
        if (fImpl->fInBuffer.size() == fImpl->fInBufferIndex) {
            do_deflate(Z_NO_FLUSH,
                       &fImpl->fZStream,
                       fImpl->fOut,
                       SkSpan(fImpl->fInBuffer).first(fImpl->fInBufferIndex));
            fImpl->fInBufferIndex = 0;
        }
    }
    return true;
}

size_t SkDeflateWStream::bytesWritten() const {
    return fImpl->fZStream.total_in + fImpl->fInBufferIndex;
}
