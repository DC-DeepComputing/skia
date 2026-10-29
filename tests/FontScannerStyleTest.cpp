/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkEndian.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstdint>
#include <cstring>
#include <vector>

#if defined(SK_TYPEFACE_FACTORY_FREETYPE)
#include "include/ports/SkFontScanner_FreeType.h"
#endif
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
#include "include/ports/SkFontScanner_Fontations.h"
#endif

namespace {

void put_be16(uint8_t* dst, uint16_t v) {
    v = SkEndian_SwapBE16(v);
    memcpy(dst, &v, sizeof(v));
}

void put_be32(uint8_t* dst, uint32_t v) {
    v = SkEndian_SwapBE32(v);
    memcpy(dst, &v, sizeof(v));
}

uint16_t get_be16(const uint8_t* src) {
    uint16_t v;
    memcpy(&v, src, sizeof(v));
    return SkEndian_SwapBE16(v);
}

uint32_t get_be32(const uint8_t* src) {
    uint32_t v;
    memcpy(&v, src, sizeof(v));
    return SkEndian_SwapBE32(v);
}

std::vector<uint8_t> load_resource_bytes(skiatest::Reporter* reporter, const char* resource) {
    sk_sp<SkData> data = GetResourceAsData(resource);
    REPORTER_ASSERT(reporter, data && data->size() > 0, "%s", resource);
    if (!data) {
        return {};
    }
    return std::vector<uint8_t>(data->bytes(), data->bytes() + data->size());
}

size_t find_sfnt_dir_entry(const std::vector<uint8_t>& sfnt, SkFourByteTag tag) {
    if (sfnt.size() < 12) {
        return 0;
    }
    const uint16_t numTables = get_be16(sfnt.data() + 4);
    for (uint16_t i = 0; i < numTables; ++i) {
        const size_t entry = 12 + size_t(i) * 16;
        if (entry + 16 <= sfnt.size() && get_be32(sfnt.data() + entry) == tag) {
            return entry;
        }
    }
    return 0;
}

void assert_scan_style_agrees(skiatest::Reporter* reporter,
                              const SkFontScanner& scanner,
                              const uint8_t* bytes,
                              size_t size,
                              int face,
                              const char* label,
                              bool requireScanSuccess = false) {
    SkMemoryStream stream(bytes, size, /*copyData=*/false);
    const SkFontArguments args = SkFontArguments().setCollectionIndex(face);
    SkFontStyle style;
    const bool scanned = scanner.scanStyle(&stream, args, &style);
    if (requireScanSuccess) {
        REPORTER_ASSERT(reporter, scanned, "%s", label);
    }
    if (!scanned) {
        return;
    }
    sk_sp<SkTypeface> tf = scanner.MakeFromStream(
            std::make_unique<SkMemoryStream>(bytes, size, /*copyData=*/true), args);
    REPORTER_ASSERT(reporter, tf && tf->fontStyle() == style, "%s", label);
}

void check_scan_style_edge_cases(skiatest::Reporter* reporter, const SkFontScanner& scanner) {
    const std::vector<uint8_t> ttc = load_resource_bytes(reporter, "fonts/test.ttc");
    const std::vector<uint8_t> em = load_resource_bytes(reporter, "fonts/Em.ttf");
    const std::vector<uint8_t> var = load_resource_bytes(reporter, "fonts/Distortable.ttf");
    if (ttc.empty() || em.empty() || var.empty()) {
        return;
    }

    // Odd-aligned memory base and odd SFNT table offset exercise unaligned multi-byte reads
    std::vector<uint8_t> oddTtc(ttc.size() + 1, 0);
    memcpy(oddTtc.data() + 1, ttc.data(), ttc.size());
    assert_scan_style_agrees(
            reporter, scanner, oddTtc.data() + 1, ttc.size(), 0, "odd-ttc-0", true);
    assert_scan_style_agrees(
            reporter, scanner, oddTtc.data() + 1, ttc.size(), 1, "odd-ttc-1", true);

    const size_t emOs2Entry = find_sfnt_dir_entry(em, SkSetFourByteTag('O', 'S', '/', '2'));
    REPORTER_ASSERT(reporter, emOs2Entry != 0);
    const uint32_t emOs2Off = get_be32(em.data() + emOs2Entry + 8);
    const uint32_t emOs2Len = get_be32(em.data() + emOs2Entry + 12);

    {
        std::vector<uint8_t> buf = em;
        buf.resize(buf.size() + ((5 - (buf.size() % 4)) % 4), 0);
        const uint32_t oddOff = static_cast<uint32_t>(buf.size());
        buf.insert(buf.end(), em.begin() + emOs2Off, em.begin() + emOs2Off + emOs2Len);
        put_be32(buf.data() + emOs2Entry + 8, oddOff);
        std::vector<uint8_t> shifted(buf.size() + 1, 0);
        memcpy(shifted.data() + 1, buf.data(), buf.size());
        assert_scan_style_agrees(
                reporter, scanner, shifted.data() + 1, buf.size(), 0, "odd-os2-offset", true);
    }

    // FreeType treats OS/2 version 0xFFFF or a versioned table hitting EOF before its spec
    // minimum as missing OS/2, ignoring usWeightClass/fsSelection
    {
        std::vector<uint8_t> buf = em;
        put_be16(buf.data() + emOs2Off + 0, 0xFFFF);
        put_be16(buf.data() + emOs2Off + 4, 900);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "os2-v-ffff");
    }
    {
        std::vector<uint8_t> buf = em;
        const uint32_t tailOff = static_cast<uint32_t>(buf.size());
        buf.insert(buf.end(), em.begin() + emOs2Off, em.begin() + emOs2Off + 78);
        put_be16(buf.data() + tailOff + 0, 4);
        put_be16(buf.data() + tailOff + 4, 900);
        put_be32(buf.data() + emOs2Entry + 8, tailOff);
        put_be32(buf.data() + emOs2Entry + 12, 78);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "os2-v4-len78");
    }
    {
        std::vector<uint8_t> buf = em;
        const uint32_t tailOff = static_cast<uint32_t>(buf.size());
        buf.insert(buf.end(), em.begin() + emOs2Off, em.begin() + emOs2Off + 96);
        put_be16(buf.data() + tailOff + 0, 5);
        put_be16(buf.data() + tailOff + 4, 900);
        put_be32(buf.data() + emOs2Entry + 8, tailOff);
        put_be32(buf.data() + emOs2Entry + 12, 96);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "os2-v5-len96");
    }

    // FreeType clears has_outline when CBLC/CBDT is present (taking italic from head.macStyle)
    // and rejects or strips outlines when hhea is missing
    {
        std::vector<uint8_t> buf = em;
        const size_t fftmEntry = find_sfnt_dir_entry(buf, SkSetFourByteTag('F', 'F', 'T', 'M'));
        REPORTER_ASSERT(reporter, fftmEntry != 0);
        put_be32(buf.data() + fftmEntry, SkSetFourByteTag('C', 'B', 'L', 'C'));
        put_be16(buf.data() + emOs2Off + 62, 0x0001);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "hybrid-cblc");
    }
    {
        std::vector<uint8_t> buf = em;
        const size_t hheaEntry = find_sfnt_dir_entry(buf, SkSetFourByteTag('h', 'h', 'e', 'a'));
        REPORTER_ASSERT(reporter, hheaEntry != 0);
        put_be32(buf.data() + hheaEntry, SkSetFourByteTag('h', 'h', 'e', 'b'));
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "missing-hhea");
    }

    // FreeType disables variations when fvar's minorVersion or instanceSize is invalid
    const size_t fvarEntry = find_sfnt_dir_entry(var, SkSetFourByteTag('f', 'v', 'a', 'r'));
    REPORTER_ASSERT(reporter, fvarEntry != 0);
    const uint32_t fvarOff = get_be32(var.data() + fvarEntry + 8);
    const uint16_t axesOff = get_be16(var.data() + fvarOff + 4);
    {
        std::vector<uint8_t> buf = var;
        put_be16(buf.data() + fvarOff + 2, 1);
        put_be32(buf.data() + fvarOff + axesOff + 4, 100 << 16);
        put_be32(buf.data() + fvarOff + axesOff + 8, 700 << 16);
        put_be32(buf.data() + fvarOff + axesOff + 12, 900 << 16);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "fvar-minor-1");
    }
    {
        std::vector<uint8_t> buf = var;
        put_be16(buf.data() + fvarOff + 14, 6);
        put_be32(buf.data() + fvarOff + axesOff + 4, 100 << 16);
        put_be32(buf.data() + fvarOff + axesOff + 8, 700 << 16);
        put_be32(buf.data() + fvarOff + axesOff + 12, 900 << 16);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "fvar-inst-size");
    }

    // Negative 16.16 fixed-point fvar bounds (such as variable Roboto's 'slnt' min=-10, def=0,
    // max=0) must be decoded as signed SkFixed rather than unsigned uint32_t
    {
        std::vector<uint8_t> buf = var;
        put_be32(buf.data() + fvarOff + axesOff + 0, SkSetFourByteTag('s', 'l', 'n', 't'));
        put_be32(buf.data() + fvarOff + axesOff + 4, static_cast<uint32_t>(-10 * 65536));
        put_be32(buf.data() + fvarOff + axesOff + 8, 0);
        put_be32(buf.data() + fvarOff + axesOff + 12, 0);
        assert_scan_style_agrees(
                reporter, scanner, buf.data(), buf.size(), 0, "fvar-negative-slnt", true);
    }

    // FreeType rejects unrecognized SFNT version tags and TrueType outlines missing 'loca'
    {
        std::vector<uint8_t> buf = em;
        put_be32(buf.data(), 0x12345678);
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "bad-sfnt-tag");
    }
    {
        std::vector<uint8_t> buf = em;
        const size_t locaEntry = find_sfnt_dir_entry(buf, SkSetFourByteTag('l', 'o', 'c', 'a'));
        REPORTER_ASSERT(reporter, locaEntry != 0);
        put_be32(buf.data() + locaEntry, SkSetFourByteTag('l', 'o', 'c', 'b'));
        assert_scan_style_agrees(reporter, scanner, buf.data(), buf.size(), 0, "missing-loca");
    }

    // Non-memory streams at a non-zero cursor must not crash or misread in scanStyle
    {
        SkFILEStream fileStream(GetResourcePath("fonts/Em.ttf").c_str());
        REPORTER_ASSERT(reporter, fileStream.isValid());
        REPORTER_ASSERT(reporter, fileStream.seek(4));
        SkFontStyle style;
        if (scanner.scanStyle(&fileStream, SkFontArguments(), &style)) {
            sk_sp<SkTypeface> tf =
                    scanner.MakeFromStream(fileStream.duplicate(), SkFontArguments());
            REPORTER_ASSERT(reporter, tf && tf->fontStyle() == style, "filestream-nonzero-pos");
        }
    }
}

}  // namespace

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
    std::unique_ptr<SkFontScanner> scanner = SkFontScanner_Make_FreeType();
    check_scan_style(reporter, *scanner, GetResourcePath("fonts").c_str());
    check_scan_style_edge_cases(reporter, *scanner);
}
#endif

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
DEF_TEST(FontScanner_Fontations_ScanStyleMatchesMakeFromStream, reporter) {
    std::unique_ptr<SkFontScanner> scanner = SkFontScanner_Make_Fontations();
    check_scan_style(reporter, *scanner, GetResourcePath("fonts").c_str());
    check_scan_style_edge_cases(reporter, *scanner);
}
#endif
