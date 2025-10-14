/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDocument_DEFINED
#define SkXPSDocument_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include "include/core/SkDocument.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

struct IXpsOMObjectFactory;
class SkWStream;
class SkPixmap;

namespace SkXPS {

using EncodePngCallback = bool (*)(SkWStream* dst, const SkPixmap& src);

struct Options {
    float dpi = SK_ScalarDefaultRasterDPI;

    /** Clients can provide a way to encode png. If not supplied, this will
     *  temporarily be filled in with a libpng implementation until clients
     *  are migrated.
     *  Skia's libpng or rust png encoder can be used here.
    */
    SkXPS::EncodePngCallback pngEncoder = nullptr;

    // Skia's XPS support depends on having both a png encoder registered to handle
    // XPS image brushes. It will technically work on some XPS documents with a
    // nullptr png encoder. If clients will be creating XPS documents that don't
    // need this, they should set this to true to avoid an internal assert from firing.
    bool allowNoPngs = false;
};

SK_API sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      SkXPS::Options opts);

#if !defined(SK_DISABLE_LEGACY_XPS_FACTORIES)
inline sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory) {
    SkXPS::Options opts;
    return MakeDocument(stream, xpsFactory, opts);
}

inline sk_sp<SkDocument> MakeDocument(SkWStream* stream,
                                      IXpsOMObjectFactory* xpsFactory,
                                      float dpi) {
    SkXPS::Options opts;
    opts.dpi = dpi;
    return MakeDocument(stream, xpsFactory, opts);
}
#endif

}  // namespace SkXPS
#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDocument_DEFINED
