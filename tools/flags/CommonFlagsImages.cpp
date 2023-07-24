// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommonFlags.h"

using namespace skia_private;

namespace CommonFlags {
bool CollectImages(CommandLineFlags::StringArray images, TArray<SkString>* output) {
    SkASSERT(output);

    static const char* const exts[] = {
        "bmp",
        "gif",
        "jpg",
        "jpeg",
        "png",
        "webp",
        "ktx",
        "astc",
        "wbmp",
        "ico",
#if !defined(SK_BUILD_FOR_WIN)
        "BMP",
        "GIF",
        "JPG",
        "JPEG",
        "PNG",
        "WEBP",
        "KTX",
        "ASTC",
        "WBMP",
        "ICO",
#endif
#ifdef SK_HAS_HEIF_LIBRARY
        "heic",
#if !defined(SK_BUILD_FOR_WIN)
        "HEIC",
#endif
#endif
#ifdef SK_CODEC_DECODES_RAW
        "arw",
        "cr2",
        "dng",
        "nef",
        "nrw",
        "orf",
        "raf",
        "rw2",
        "pef",
        "srw",
#if !defined(SK_BUILD_FOR_WIN)
        "ARW",
        "CR2",
        "DNG",
        "NEF",
        "NRW",
        "ORF",
        "RAF",
        "RW2",
        "PEF",
        "SRW",
#endif
#endif
    };

    // DO NOT SUBMIT
    SkDebugf("*** COLLECTING IMAGES... %d IMAGES GIVEN ***\n", images.size());

    for (int i = 0; i < images.size(); ++i) {
        const char* flag = images[i];
        SkDebugf("*** COLLECTING IMAGE: %s ***\n", flag); // DO NOT SUBMIT
        if (!sk_exists(flag)) {
            SkDebugf("%s does not exist!\n", flag);
            return false;
        }

        if (sk_isdir(flag)) {
            SkDebugf("*** IMAGE IS A DIRECTORY: %s ***\n", flag); // DO NOT SUBMIT
            // If the value passed in is a directory, add all the images
            bool foundAnImage = false;
            for (const char* ext : exts) {
                SkOSFile::Iter it(flag, ext);
                SkString file;
                while (it.next(&file)) {
                    foundAnImage = true;
                    SkDebugf("*** FOUND IMAGE INSIDE DIRECTORY %s: %s ***\n", flag, file.c_str()); // DO NOT SUBMIT
                    output->push_back() = SkOSPath::Join(flag, file.c_str());
                }
            }
            if (!foundAnImage) {
                SkDebugf("No supported images found in %s!\n", flag);
                return false;
            }
        } else {
            SkDebugf("*** COLLECTED SINGLE IMAGE: %s ***\n", flag); // DO NOT SUBMIT
            // Also add the value if it is a single image
            output->push_back() = flag;
        }
    }

    // DO NOT SUBMIT
    SkDebugf("*** FINISHED COLLECTING IMAGES ***\n");
    return true;
}

}  // namespace CommonFlags
