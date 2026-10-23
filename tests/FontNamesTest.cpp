/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkDebug.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkEndian.h"
#include "src/sfnt/SkOTTable_head.h"
#include "src/sfnt/SkOTTable_name.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_GDI_AVAILABLE)
#include "include/ports/SkTypeface_win.h"
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace skia_private;

namespace {

template <size_t R, size_t D> struct Format0NameTable {
    SkOTTableName header;
    SkOTTableName::Record nameRecord[R];
    char data[D];
};

template <size_t R, size_t L, size_t D> struct Format1NameTable {
    SkOTTableName header;
    SkOTTableName::Record nameRecord[R];
    struct {
        SkOTTableName::Format1Ext header;
        SkOTTableName::Format1Ext::LangTagRecord langTagRecord[L];
    } format1ext;
    char data[D];
};

typedef Format0NameTable<1, 9> SimpleFormat0NameTable;
constexpr SimpleFormat0NameTable simpleFormat0NameTable = {
    /*header*/ {
        /*format*/ SkOTTableName::format_0,
        /*count*/ SkTEndianSwap16<1>::value,
        /*stringOffset*/ SkTEndianSwap16<offsetof(SimpleFormat0NameTable, data)>::value,
    },
    /*nameRecord[]*/ {
        /*Record*/ {
            /*platformID*/ { SkOTTableName::Record::PlatformID::Windows },
            /*encodingID*/ { SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2 },
            /*languageID*/ { SkOTTableName::Record::LanguageID::Windows::English_UnitedStates },
            /*nameID*/ { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
            /*length*/ SkTEndianSwap16<8>::value,
            /*offset*/ SkTEndianSwap16<0>::value,
        }
    },
    /*data*/ "\x0" "T" "\x0" "e" "\x0" "s" "\x0" "t",
};

typedef Format1NameTable<1, 1, 19> SimpleFormat1NameTable;
constexpr SimpleFormat1NameTable simpleFormat1NameTable = {
    /*header*/ {
        /*format*/ SkOTTableName::format_1,
        /*count*/ SkTEndianSwap16<1>::value,
        /*stringOffset*/ SkTEndianSwap16<offsetof(SimpleFormat1NameTable, data)>::value,
    },
    /*nameRecord[]*/ {
        /*Record*/ {
            /*platformID*/ { SkOTTableName::Record::PlatformID::Windows },
            /*encodingID*/ { SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2 },
            /*languageID*/ { SkTEndianSwap16<0x8000 + 0>::value },
            /*nameID*/ { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
            /*length*/ SkTEndianSwap16<8>::value,
            /*offset*/ SkTEndianSwap16<0>::value,
        }
    },
    /*format1ext*/ {
        /*header*/ {
            /*langTagCount*/ SkTEndianSwap16<1>::value,
        },
        /*langTagRecord[]*/ {
            /*LangTagRecord*/ {
                /*length*/ SkTEndianSwap16<10>::value,
                /*offset*/ SkTEndianSwap16<8>::value,
            },
        },
    },
    /*data*/ "\x0" "T" "\x0" "e" "\x0" "s" "\x0" "t"
             "\x0" "e" "\x0" "n" "\x0" "-" "\x0" "U" "\x0" "S",
};

struct FontNamesTest {
    const uint8_t* data;
    size_t size;
    SkOTTableName::Record::NameID nameID;
    size_t nameCount;
    struct {
        const char* name;
        const char* language;
    } names[10];

} tests[] = {
    {
        reinterpret_cast<const uint8_t*>(&simpleFormat0NameTable),
        sizeof(simpleFormat0NameTable),
        { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
        1,
        {
            { "Test", "en-US" },
        },
    },
    {
        reinterpret_cast<const uint8_t*>(&simpleFormat1NameTable),
        sizeof(simpleFormat1NameTable),
        { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
        1,
        {
            { "Test", "en-US" },
        },
    },
};

static void test_synthetic(skiatest::Reporter* reporter, bool verbose) {
    for (const auto& test : tests) {
        SkOTTableName::Iterator iter(test.data, test.size, test.nameID.predefined.value);
        SkOTTableName::Iterator::Record record;
        size_t nameIndex = 0;
        while (nameIndex < test.nameCount && iter.next(record)) {
            REPORTER_ASSERT(reporter,
                            strcmp(test.names[nameIndex].name, record.name.c_str()) == 0,
                            "Name did not match.");

            REPORTER_ASSERT(reporter,
                            strcmp(test.names[nameIndex].language, record.language.c_str()) == 0,
                            "Language did not match.");

            if (verbose) {
                SkDebugf("%s <%s>\n", record.name.c_str(), record.language.c_str());
            }

            ++nameIndex;
        }

        REPORTER_ASSERT(reporter, nameIndex == test.nameCount, "Fewer names than expected.");

        REPORTER_ASSERT(reporter, !iter.next(record), "More names than expected.");
    }
}

#define MAX_FAMILIES 1000
static void test_systemfonts(skiatest::Reporter* reporter, bool verbose) {
    static const SkFontTableTag nameTag = SkSetFourByteTag('n','a','m','e');

    sk_sp<SkFontMgr> fm(ToolUtils::TestFontMgr());
    SkASSERT_RELEASE(fm);
    int count = std::min(fm->countFamilies(), MAX_FAMILIES);
    for (int i = 0; i < count; ++i) {
        SkString fname;
        fm->getFamilyName(i, &fname);

        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        for (int j = 0; j < set->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            set->getStyle(j, &fs, &sname);

            sk_sp<SkTypeface> typeface(set->createTypeface(j));
            if (!typeface) {
                REPORTER_ASSERT(reporter, typeface.get(),
                                "Could not create %s %s.", fname.c_str(), sname.c_str());
                continue;
            }

            SkString familyName;
            typeface->getFamilyName(&familyName);
            if (verbose) {
                SkDebugf("[%s]\n", familyName.c_str());
            }

            sk_sp<SkTypeface::LocalizedStrings> familyNamesIter(
                typeface->createFamilyNameIterator());
            SkTypeface::LocalizedString familyNameLocalized;
            while (familyNamesIter->next(&familyNameLocalized)) {
                if (verbose) {
                    SkDebugf("(%s) <%s>\n", familyNameLocalized.fString.c_str(),
                                            familyNameLocalized.fLanguage.c_str());
                }
            }

            size_t nameTableSize = typeface->getTableSize(nameTag);
            if (0 == nameTableSize) {
                continue;
            }
            AutoTMalloc<uint8_t> nameTableData(nameTableSize);
            size_t copied = typeface->getTableData(nameTag, 0, nameTableSize, nameTableData.get());
            if (copied != nameTableSize) {
                continue;
            }

            SkOTTableName::Iterator::Record record;
            SkOTTableName::Iterator familyNameIter(nameTableData.get(), nameTableSize,
                SkOTTableName::Record::NameID::Predefined::FontFamilyName);
            while (familyNameIter.next(record)) {
                REPORTER_ASSERT(
                        reporter,
                        SkOTTableName::Record::NameID::Predefined::FontFamilyName == record.type,
                        "Requested family name, got something else.");
                if (verbose) {
                    SkDebugf("{%s} <%s>\n", record.name.c_str(), record.language.c_str());
                }
            }

            SkOTTableName::Iterator styleNameIter(nameTableData.get(), nameTableSize,
                SkOTTableName::Record::NameID::Predefined::FontSubfamilyName);
            while (styleNameIter.next(record)) {
                REPORTER_ASSERT(
                        reporter,
                        SkOTTableName::Record::NameID::Predefined::FontSubfamilyName == record.type,
                        "Requested subfamily name, got something else.");
                if (verbose) {
                    SkDebugf("{{%s}} <%s>\n", record.name.c_str(), record.language.c_str());
                }
            }

            if (verbose) {
                SkDebugf("\n");
            }
        }
    }
}

static void test_rename_font(skiatest::Reporter* reporter) {
    struct EntrySpec {
        SkFourByteTag tag;
        uint32_t offset;
        uint32_t logicalLength;
    };
    auto makeSfnt = [](uint16_t headerNumTables,
                       std::initializer_list<EntrySpec> entries,
                       size_t totalSize) {
        std::vector<uint8_t> bytes(totalSize, 0);
        if (totalSize >= sizeof(SkSFNTHeader)) {
            SkSFNTHeader header = {};
            header.fontType = SkSFNTHeader::fontType_WindowsTrueType::TAG;
            header.numTables = SkEndian_SwapBE16(headerNumTables);
            memcpy(bytes.data(), &header, sizeof(header));
        }
        size_t pos = sizeof(SkSFNTHeader);
        for (const auto& spec : entries) {
            if (pos + sizeof(SkSFNTHeader::TableDirectoryEntry) > totalSize) {
                break;
            }
            SkSFNTHeader::TableDirectoryEntry entry = {};
            entry.tag = spec.tag;
            entry.offset = SkEndian_SwapBE32(spec.offset);
            entry.logicalLength = SkEndian_SwapBE32(spec.logicalLength);
            memcpy(bytes.data() + pos, &entry, sizeof(entry));
            pos += sizeof(entry);
        }
        return bytes;
    };

    const uint32_t dirEnd =
            sizeof(SkSFNTHeader) + 2 * sizeof(SkSFNTHeader::TableDirectoryEntry);
    const uint32_t oldNameLen = sizeof(simpleFormat0NameTable);
    const uint32_t oldNamePadded = (oldNameLen + 3u) & ~3u;
    const uint32_t headOffset = dirEnd + oldNamePadded;
    const size_t validFontSize = headOffset + sizeof(SkOTTableHead);
    std::vector<uint8_t> validFont = makeSfnt(
            2,
            {{SkOTTableName::TAG, dirEnd, oldNameLen},
             {SkOTTableHead::TAG, headOffset, sizeof(SkOTTableHead)}},
            validFontSize);
    memcpy(validFont.data() + dirEnd, &simpleFormat0NameTable, sizeof(simpleFormat0NameTable));
    {
        SkMemoryStream stream(validFont.data(), validFont.size(), false);
        sk_sp<SkData> renamed(SkOTUtils::RenameFont(&stream, "Renamed", 7));
        REPORTER_ASSERT(reporter, renamed != nullptr);
        if (renamed) {
            const uint8_t* out = renamed->bytes();
            SkSFNTHeader::TableDirectoryEntry nameEntry;
            SkSFNTHeader::TableDirectoryEntry headEntry;
            memcpy(&nameEntry, out + sizeof(SkSFNTHeader), sizeof(nameEntry));
            memcpy(&headEntry,
                   out + sizeof(SkSFNTHeader) + sizeof(nameEntry),
                   sizeof(headEntry));
            uint32_t newNameOffset = SkEndian_SwapBE32(nameEntry.offset);
            uint32_t newNameLen = SkEndian_SwapBE32(nameEntry.logicalLength);
            uint32_t newHeadOffset = SkEndian_SwapBE32(headEntry.offset);
            REPORTER_ASSERT(reporter, (newNameOffset & 3u) == 0);
            REPORTER_ASSERT(reporter, newNameOffset + newNameLen <= renamed->size());
            REPORTER_ASSERT(reporter, newHeadOffset == dirEnd);

            SkOTTableName::Iterator iter(
                    out + newNameOffset,
                    newNameLen,
                    SkOTTableName::Record::NameID::Predefined::FontFamilyName);
            SkOTTableName::Iterator::Record record;
            REPORTER_ASSERT(reporter, iter.next(record));
            REPORTER_ASSERT(reporter, record.name.equals("Renamed"));

            // Verify that the rewritten head table checksum matches the required font checksum
            uint32_t validChecksum = SkOTUtils::CalcTableChecksum(
                    reinterpret_cast<SK_OT_ULONG*>(const_cast<void*>(renamed->data())),
                    renamed->size());
            REPORTER_ASSERT(reporter, validChecksum == SkOTTableHead::fontChecksum);
        }
    }

    // Offset + physical length exceeds stream length and offset exceeds newDataSize (b/557269199)
    {
        std::vector<uint8_t> oobFont = makeSfnt(1, {{SkOTTableName::TAG, 800, 900}}, 1024);
        SkMemoryStream stream(oobFont.data(), oobFont.size(), false);
        sk_sp<SkData> renamed(SkOTUtils::RenameFont(&stream, "A", 1));
        REPORTER_ASSERT(reporter, renamed == nullptr);
    }

    // 32-bit wrap-around when rounding logicalLength up to a multiple of 4
    {
        std::vector<uint8_t> wrapFont =
                makeSfnt(1, {{SkOTTableName::TAG, 28, 0xFFFFFFFFu}}, 128);
        SkMemoryStream stream(wrapFont.data(), wrapFont.size(), false);
        sk_sp<SkData> renamed(SkOTUtils::RenameFont(&stream, "A", 1));
        REPORTER_ASSERT(reporter, renamed == nullptr);
    }

    // Declared numTables extends past oldNameTableOffset and allocated buffer
    {
        std::vector<uint8_t> hugeDirFont = makeSfnt(100, {{SkOTTableName::TAG, 28, 32}}, 128);
        SkMemoryStream stream(hugeDirFont.data(), hugeDirFont.size(), false);
        sk_sp<SkData> renamed(SkOTUtils::RenameFont(&stream, "A", 1));
        REPORTER_ASSERT(reporter, renamed == nullptr);
    }

    // Sibling table offset points inside the old 'name' table region
    {
        std::vector<uint8_t> overlapFont = makeSfnt(
                2,
                {{SkOTTableName::TAG, 44, 40}, {SkOTTableHead::TAG, 60, sizeof(SkOTTableHead)}},
                128);
        SkMemoryStream stream(overlapFont.data(), overlapFont.size(), false);
        sk_sp<SkData> renamed(SkOTUtils::RenameFont(&stream, "A", 1));
        REPORTER_ASSERT(reporter, renamed == nullptr);
    }

    // Round-trip real SFNT fonts through RenameFont and SkFontMgr::makeFromData
    static constexpr const char* kResourceFonts[] = {
            "fonts/Distortable.ttf",
            "fonts/Roboto-Regular.ttf",
            "fonts/HangingS.ttf",
            "fonts/7630.otf",
            "fonts/Stroking.otf",
            "fonts/Variable.ttf",
            "fonts/colr.ttf",
    };
    static constexpr char kRenamedFamily[] = "SkiaRenamedFont";
    constexpr int kRenamedFamilyLen = sizeof(kRenamedFamily) - 1;
    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();
    for (const char* res : kResourceFonts) {
        std::unique_ptr<SkStreamAsset> resStream = GetResourceAsStream(res);
        if (!resStream) {
            continue;
        }
        sk_sp<SkData> renamed(
                SkOTUtils::RenameFont(resStream.get(), kRenamedFamily, kRenamedFamilyLen));
        REPORTER_ASSERT(reporter, renamed != nullptr, "Failed to rename %s", res);
        if (!renamed) {
            continue;
        }
        REPORTER_ASSERT(reporter, (renamed->size() & 3u) == 0);
        uint32_t checksum = SkOTUtils::CalcTableChecksum(
                reinterpret_cast<SK_OT_ULONG*>(const_cast<void*>(renamed->data())),
                renamed->size());
        REPORTER_ASSERT(
                reporter, checksum == SkOTTableHead::fontChecksum, "Checksum mismatch for %s", res);

        if (sk_sp<SkTypeface> origFace = fm->makeFromStream(resStream->duplicate())) {
            sk_sp<SkTypeface> renamedFace = fm->makeFromData(renamed);
            REPORTER_ASSERT(reporter, renamedFace != nullptr, "Failed to reload %s", res);
            if (renamedFace) {
                SkString familyName;
                renamedFace->getFamilyName(&familyName);
                REPORTER_ASSERT(reporter, familyName.equals(kRenamedFamily));
                REPORTER_ASSERT(reporter, renamedFace->countGlyphs() == origFace->countGlyphs());
            }
        }
    }

    int familyCount = std::min(fm->countFamilies(), 16);
    for (int i = 0; i < familyCount; ++i) {
        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        if (!set || set->count() == 0) {
            continue;
        }
        sk_sp<SkTypeface> face(set->createTypeface(0));
        if (!face || face->getTableSize(SkOTTableName::TAG) == 0) {
            continue;
        }
        int ttcIndex = 0;
        std::unique_ptr<SkStreamAsset> faceStream = face->openStream(&ttcIndex);
        if (!faceStream || ttcIndex != 0) {
            continue;
        }
        sk_sp<SkData> renamed(
                SkOTUtils::RenameFont(faceStream.get(), kRenamedFamily, kRenamedFamilyLen));
        if (!renamed) {
            continue;
        }
        REPORTER_ASSERT(reporter, (renamed->size() & 3u) == 0);
        if (face->getTableSize(SkOTTableHead::TAG) >= sizeof(SkOTTableHead)) {
            uint32_t checksum = SkOTUtils::CalcTableChecksum(
                    reinterpret_cast<SK_OT_ULONG*>(const_cast<void*>(renamed->data())),
                    renamed->size());
            REPORTER_ASSERT(reporter, checksum == SkOTTableHead::fontChecksum);
        }
        if (sk_sp<SkTypeface> origReloaded = fm->makeFromStream(faceStream->duplicate())) {
            sk_sp<SkTypeface> renamedFace = fm->makeFromData(renamed);
            REPORTER_ASSERT(reporter, renamedFace != nullptr);
            if (renamedFace) {
                SkString familyName;
                renamedFace->getFamilyName(&familyName);
                REPORTER_ASSERT(reporter, familyName.equals(kRenamedFamily));
                REPORTER_ASSERT(reporter,
                                renamedFace->countGlyphs() == origReloaded->countGlyphs());
            }
        }
    }

#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTMGR_GDI_AVAILABLE)
    if (sk_sp<SkFontMgr> gdiFm = SkFontMgr_New_GDI()) {
        if (std::unique_ptr<SkStreamAsset> ttfStream =
                    GetResourceAsStream("fonts/Roboto-Regular.ttf")) {
            sk_sp<SkTypeface> gdiFace = gdiFm->makeFromStream(std::move(ttfStream));
            REPORTER_ASSERT(reporter, gdiFace != nullptr);
            if (gdiFace) {
                REPORTER_ASSERT(reporter, gdiFace->countGlyphs() > 0);
            }
        }
        std::vector<uint8_t> oobFont = makeSfnt(1, {{SkOTTableName::TAG, 800, 900}}, 1024);
        sk_sp<SkData> oobData = SkData::MakeWithoutCopy(oobFont.data(), oobFont.size());
        REPORTER_ASSERT(reporter, gdiFm->makeFromData(oobData) == nullptr);
    }
#endif
}

} // namespace

static DEFINE_bool(verboseFontNames, false, "verbose FontNames test.");

DEF_TEST(FontNames, reporter) {
    test_synthetic(reporter, FLAGS_verboseFontNames);
    test_rename_font(reporter);
    test_systemfonts(reporter, FLAGS_verboseFontNames);
}
