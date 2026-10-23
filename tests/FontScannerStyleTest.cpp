/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
#include "include/ports/SkFontScanner_FreeType.h"
#endif
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
#include "include/ports/SkFontScanner_Fontations.h"
#endif

// scanStyle must agree with MakeFromStream whenever it claims success
static void check_scan_style(skiatest::Reporter* reporter,
                             const SkFontScanner& scanner,
                             const char* dirPath) {
    static constexpr SkFontArguments::VariationPosition::Coordinate kNarrowBold[] = {
            {SkSetFourByteTag('w', 'd', 't', 'h'), 75},
            {SkSetFourByteTag('w', 'g', 'h', 't'), 700},
    };
    const SkFontArguments::VariationPosition positions[] = {{nullptr, 0}, {kNarrowBold, 2}};
    int scanned = 0;
    SkOSFile::Iter iter(dirPath);
    for (SkString name; iter.next(&name);) {
        const SkString path = SkOSPath::Join(dirPath, name.c_str());
        std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path.c_str());
        int numFaces = 0;
        if (!stream || !scanner.scanFile(stream.get(), &numFaces)) {
            continue;
        }
        for (int face = 0; face < numFaces; ++face) {
            for (const auto& position : positions) {
                const SkFontArguments args =
                        SkFontArguments().setCollectionIndex(face).setVariationDesignPosition(
                                position);
                SkFontStyle style;
                if (!scanner.scanStyle(stream.get(), args, &style)) {
                    continue;
                }
                ++scanned;
                sk_sp<SkTypeface> typeface = scanner.MakeFromStream(stream->duplicate(), args);
                REPORTER_ASSERT(reporter,
                                typeface && typeface->fontStyle() == style,
                                "%s face %d",
                                path.c_str(),
                                face);
            }
        }
    }
    REPORTER_ASSERT(reporter, scanned > 0);
}

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
DEF_TEST(FontScanner_FreeType_ScanStyleMatchesMakeFromStream, reporter) {
    check_scan_style(reporter, *SkFontScanner_Make_FreeType(), GetResourcePath("fonts").c_str());
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
DEF_TEST(FontScanner_Fontations_ScanStyleMatchesMakeFromStream, reporter) {
    check_scan_style(reporter, *SkFontScanner_Make_Fontations(), GetResourcePath("fonts").c_str());
}
#endif
