/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegDecoderMgr_DEFINED
#define SkJpegDecoderMgr_DEFINED

#include "include/codec/SkCodec.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSourceMgr.h"

extern "C" {
    #include "jpeglib.h"  // NO_G3_REWRITE
}

#include <memory>

class SkStream;

class JpegDecoderMgr : SkNoncopyable {
public:

    /*
     * Print a useful error message and return false
     */
    bool returnFalse(const char caller[]);

    /*
     * Print a useful error message and return a decode failure
     */
    SkCodec::Result returnFailure(const char caller[], SkCodec::Result result);

    /*
     * Create the decode manager
     * Does not take ownership of stream
     */
    explicit JpegDecoderMgr(SkStream* stream);

    /*
     * Initialize decompress struct
     * Initialize the source manager
     */
    void  init();

    /*
     * Returns true if it successfully sets outColor to the encoded color,
     * and false otherwise.
     */
    bool getEncodedColor(SkEncodedInfo::Color* outColor);

    /*
     * Free memory used by the decode manager
     */
    ~JpegDecoderMgr();

    /*
     * Get the skjpeg_error_mgr in order to set an error return jmp_buf
     */
    skjpeg_error_mgr* errorMgr() { return &fErrorMgr; }

    /*
     * Get function for the decompress info struct
     */
    jpeg_decompress_struct* dinfo() { return &fDInfo; }

    // Get the source manager.
    SkJpegSourceMgr* getSourceMgr();

private:
    // Wrapper that calls into the full SkJpegSourceMgr interface.
    struct SourceMgr : jpeg_source_mgr {
        static void InitSource(j_decompress_ptr dinfo);
        static void SkipInputData(j_decompress_ptr dinfo, long num_bytes_long);
        static boolean FillInputBuffer(j_decompress_ptr dinfo);
        static void TermSource(j_decompress_ptr dinfo);

        explicit SourceMgr(std::unique_ptr<SkJpegSourceMgr> mgr);
        std::unique_ptr<SkJpegSourceMgr> fSourceMgr;
    };

    struct MemoryMgr : jpeg_memory_mgr {
        MemoryMgr();

        static void* AllocSmall(j_common_ptr, int, size_t);
        static void* AllocLarge(j_common_ptr, int, size_t) ;
        static JSAMPARRAY AllocSarray(j_common_ptr, int, JDIMENSION, JDIMENSION);
        static JBLOCKARRAY AllocBarray(j_common_ptr , int ,
                                                   JDIMENSION blocksperrow, JDIMENSION numrows);
        static jvirt_sarray_ptr RequestVirtSarray(j_common_ptr , int ,
                                                              boolean ,
                                                              JDIMENSION ,
                                                              JDIMENSION ,
                                                              JDIMENSION );
        static jvirt_barray_ptr RequestVirtBarray(j_common_ptr , int ,
                                                              boolean ,
                                                              JDIMENSION ,
                                                              JDIMENSION ,
                                                              JDIMENSION );
        static void RealizeVirtArrays(j_common_ptr );
        static JSAMPARRAY AccessVirtSarray(j_common_ptr , jvirt_sarray_ptr ,
                                                       JDIMENSION , JDIMENSION ,
                                                       boolean );
        static JBLOCKARRAY AccessVirtBarray(j_common_ptr , jvirt_barray_ptr ,
                                                        JDIMENSION , JDIMENSION ,
                                                        boolean );
        static void FreePool(j_common_ptr , int );
        static void SelfDestruct(j_common_ptr );

        jpeg_memory_mgr* fOrig;
    };

    jpeg_decompress_struct fDInfo;
    SourceMgr              fSrcMgr;
    MemoryMgr              fMemMgr;
    skjpeg_error_mgr       fErrorMgr;
    jpeg_progress_mgr      fProgressMgr;
    bool                   fInit;
};

#endif
