/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/PathExtractor.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"

#include <cstring>

class SkPaint;
class SkPath;

#if defined(SK_ENABLE_SVG)
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "src/xml/SkDOM.h"
#endif

namespace ToolUtils {

void ExtractPaths(const char filepath[], std::function<PathSniffCallback> callback) {
    SkFILEStream stream(filepath);
    if (!stream.isValid()) {
        SkDebugf("sniff_paths: invalid input file at \"%s\"\n", filepath);
        return;
    }

    class PathSniffer : public SkCanvas {
    public:
        PathSniffer(std::function<PathSniffCallback> callback)
                : SkCanvas(4096, 4096, nullptr)
                , fPathSniffCallback(callback) {}
    private:
        void onDrawPath(const SkPath& path, const SkPaint& paint) override {
            fPathSniffCallback(this->getTotalMatrix(), path, paint);
        }
        std::function<PathSniffCallback> fPathSniffCallback;
    };

    PathSniffer pathSniffer(callback);
    if (const char* ext = strrchr(filepath, '.'); ext && !strcmp(ext, ".svg")) {
#if defined(SK_ENABLE_SVG)
        sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromStream(stream);
        if (!svg) {
            SkDebugf("sniff_paths: couldn't load svg at \"%s\"\n", filepath);
            return;
        }
        svg->setContainerSize(SkSize::Make(pathSniffer.getBaseLayerSize()));
        svg->render(&pathSniffer);
#endif
    } else {
        sk_sp<SkPicture> skp = SkPicture::MakeFromStream(&stream);
        if (!skp) {
            SkDebugf("sniff_paths: couldn't load skp at \"%s\"\n", filepath);
            return;
        }
        skp->playback(&pathSniffer);
    }
}

}  // namespace ToolUtils
