/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Despite the name and location, this is portable code.

#include "src/ports/SkFontMgr_android_parser.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/SkFixed.h"
#include "include/private/SkOnlyStackAllocatable.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTSearch.h"
#include "src/xml/SkXMLParser.h"

#include <stdlib.h>
#include <string.h>
#include <iterator>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

using namespace skia_private;

#define LMP_SYSTEM_FONTS_FILE "/system/etc/fonts.xml"
#define OLD_SYSTEM_FONTS_FILE "/system/etc/system_fonts.xml"
#define FALLBACK_FONTS_FILE "/system/etc/fallback_fonts.xml"
#define VENDOR_FONTS_FILE "/vendor/etc/fallback_fonts.xml"
#define PRODUCT_CUSTOMIZATION_FILE "/product/etc/fonts_customization.xml"

#define LOCALE_FALLBACK_FONTS_SYSTEM_DIR "/system/etc"
#define LOCALE_FALLBACK_FONTS_VENDOR_DIR "/vendor/etc"
#define LOCALE_FALLBACK_FONTS_PREFIX "fallback_fonts-"
#define LOCALE_FALLBACK_FONTS_SUFFIX ".xml"

/**
 * This file contains TWO 'familyset' handlers:
 * One for JB and earlier which works with
 *   /system/etc/system_fonts.xml
 *   /system/etc/fallback_fonts.xml
 *   /vendor/etc/fallback_fonts.xml
 *   /system/etc/fallback_fonts-XX.xml
 *   /vendor/etc/fallback_fonts-XX.xml
 * and the other for LMP and later which works with
 *   /system/etc/fonts.xml
 *
 * If the 'familyset' 'version' attribute is 21 or higher the LMP parser is used, otherwise the JB.
 *
 * API 15 4.0.4_r2.1 system_fonts.xml, vendor_fonts.xml, fallback_fonts.xml (system+vendor) no lang.
 * API 16 4.1.1_r1 fallback_fonts-xx-XX.xml are added. Use the xx list in order.
 * API 17 4.2.2_r1.1 fallback_fonts-xx.xml are removed and 'lang' is added.
 * API 21 5.0.0_r1.0.1 fonts.xml replaces the other files.
 * API 29 10.0.0_r1 /product/etc/fonts_customization.xml with base /product/fonts is added.
 * The NDK interface is added and reading the files directly is discouraged.
 */

struct FamilyData;

struct TagHandler {
    /** Called at the start tag.
     *  Called immediately after the parent tag returns this handler from a call to 'tag'.
     *  Allows setting up for handling the tag content and processing attributes.
     *  If nullptr, will not be called.
     */
    void (*start)(FamilyData* data, SkSpan<const SkXMLParser::Attribute> attributes);

    /** Called at the end tag.
     *  Allows post-processing of any accumulated information.
     *  This will be the last call made in relation to the current tag.
     *  If nullptr, will not be called.
     */
    void (*end)(FamilyData* data);

    /** Called when a nested tag is encountered.
     *  This is responsible for determining how to handle the tag.
     *  If the tag is not recognized, return nullptr to skip the tag.
     *  If nullptr, all nested tags will be skipped.
     */
    const TagHandler* (*tag)(FamilyData* data,
                             std::string_view tag,
                             SkSpan<const SkXMLParser::Attribute> attributes);

    /** The character handler for this tag.
     *  This is only active for character data contained directly in this tag (not sub-tags).
     *  If nullptr, any character data in this tag will be ignored.
     */
    void (*chars)(FamilyData* data, std::string_view text);
};

/** Represents the current parsing state. */
struct FamilyData : public SkXMLParser {
    SK_ONLY_STACK_ALLOCATABLE;

    FamilyData(std::vector<std::unique_ptr<FontFamily>>& families,
               const SkString& basePath,
               bool isFallback,
               const char* filename,
               const TagHandler* topLevelHandler)
        : fFamilies(families)
        , fBasePath(basePath)
        , fIsFallback(isFallback)
        , fFilename(filename)
        , fHandler(&topLevelHandler, 1)
    { }

protected:
    void onStartElement(std::string_view elem, SkSpan<const Attribute> attributes) override;
    void onEndElement(std::string_view elem) override;
    void onText(std::string_view text) override {
        if (!fSkip) {
            const TagHandler* handler = fHandler.back();
            if (handler->chars) {
                handler->chars(this, text);
            }
        }
    }

public:
    std::vector<std::unique_ptr<FontFamily>>& fFamilies; // Collection of families, owned by caller
    std::unique_ptr<FontFamily> fCurrentFamily; // The family being created, owned by this
    FontFileInfo* fCurrentFontInfo = nullptr;   // The info being created, owned by fCurrentFamily
    int fVersion = 0;                           // The version of the file parsed.
    const SkString& fBasePath;                  // The current base path.
    const bool fIsFallback;                     // The file being parsed is a fallback file
    const char* fFilename;                      // The name of the file currently being parsed.

    SkString fCurrentFamilyList;                // Name from family-list tag

    int fDepth = 1;                             // The current element depth of the parse.
    int fSkip = 0;                              // The depth to stop skipping, 0 if not skipping.
    SkTDArray<const TagHandler*> fHandler;      // The stack of current tag handlers.
};

#define SK_FONTMGR_ANDROID_PARSER_PREFIX "[SkFontMgr Android Parser] "

#define SK_FONTCONFIGPARSER_WARNING(message, ...)                                 \
    SkDebugf(SK_FONTMGR_ANDROID_PARSER_PREFIX "%s:%d:%d: warning: " message "\n", \
             self->fFilename,                                                     \
             self->getCurrentLineNumber(),                                        \
             self->getCurrentColumnNumber(),                                      \
             ##__VA_ARGS__)

static bool is_whitespace(char c) {
    return c == ' ' || c == '\n'|| c == '\r' || c == '\t';
}

static void trim_string(SkString* s) {
    char* str = s->data();
    const char* start = str;  // start is inclusive
    const char* end = start + s->size();  // end is exclusive
    while (is_whitespace(*start)) { ++start; }
    if (start != end) {
        --end;  // make end inclusive
        while (is_whitespace(*end)) { --end; }
        ++end;  // make end exclusive
    }
    size_t len = end - start;
    memmove(str, start, len);
    s->resize(len);
}

static void parse_space_separated_languages(std::string_view value,
                                            TArray<SkLanguage, true>& languages) {
    while (true) {
        while (!value.empty() && is_whitespace(value.front())) {
            value.remove_prefix(1);
        }
        if (value.empty()) {
            break;
        }
        size_t len = 1;
        while (len < value.size() && !is_whitespace(value[len])) {
            ++len;
        }
        languages.emplace_back(value.data(), len);
        value.remove_prefix(len);
    }
}

namespace lmpParser {

static const TagHandler axisHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        FontFileInfo& file = *self->fCurrentFontInfo;
        SkFourByteTag axisTag = SkSetFourByteTag('\0','\0','\0','\0');
        SkFixed axisStyleValue = 0;
        bool axisTagIsValid = false;
        bool axisStyleValueIsValid = false;
        for (const auto& [name, value] : attributes) {
            if (name == "tag") {
                if (value.size() == 4) {
                    axisTag = SkSetFourByteTag(value[0], value[1], value[2], value[3]);
                    axisTagIsValid = true;
                    for (int j = 0; j < file.fVariationDesignPosition.size() - 1; ++j) {
                        if (file.fVariationDesignPosition[j].axis == axisTag) {
                            axisTagIsValid = false;
                            SK_FONTCONFIGPARSER_WARNING("'%c%c%c%c' axis specified more than once",
                                                        (char)((axisTag >> 24) & 0xFF),
                                                        (char)((axisTag >> 16) & 0xFF),
                                                        (char)((axisTag >>  8) & 0xFF),
                                                        (char)((axisTag      ) & 0xFF));
                        }
                    }
                } else {
                    SK_FONTCONFIGPARSER_WARNING("'%.*s' is an invalid axis tag",
                                                SkToInt(value.size()),
                                                value.data());
                }
            } else if (name == "stylevalue") {
                if (parse_fixed<16>(value, &axisStyleValue)) {
                    axisStyleValueIsValid = true;
                } else {
                    SK_FONTCONFIGPARSER_WARNING("'%.*s' is an invalid axis stylevalue",
                                                SkToInt(value.size()),
                                                value.data());
                }
            }
        }
        if (axisTagIsValid && axisStyleValueIsValid) {
            auto& coordinate = file.fVariationDesignPosition.push_back();
            coordinate.axis = axisTag;
            coordinate.value = SkFixedToScalar(axisStyleValue);
        }
    },
    /*end*/nullptr,
    /*tag*/nullptr,
    /*chars*/nullptr,
};

static const TagHandler fontHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        // 'weight' (non-negative integer) [default 0]
        // 'style' ("normal", "italic") [default "auto"]
        // 'index' (non-negative integer) [default 0]
        // The character data should be a filename.
        FontFileInfo& file = self->fCurrentFamily->fFonts.push_back();
        self->fCurrentFontInfo = &file;
        SkString fallbackFor;
        for (const auto& [name, value] : attributes) {
            if (name == "weight") {
                if (!parse_non_negative_integer(value, &file.fWeight)) {
                    SK_FONTCONFIGPARSER_WARNING("'%.*s' is an invalid weight",
                                                SkToInt(value.size()),
                                                value.data());
                }
            } else if (name == "style") {
                if (value == "normal") {
                    file.fStyle = FontFileInfo::Style::kNormal;
                } else if (value == "italic") {
                    file.fStyle = FontFileInfo::Style::kItalic;
                }
            } else if (name == "index") {
                if (!parse_non_negative_integer(value, &file.fIndex)) {
                    SK_FONTCONFIGPARSER_WARNING("'%.*s' is an invalid index",
                                                SkToInt(value.size()),
                                                value.data());
                }
            } else if (name == "fallbackFor") {
                /** fallbackFor specifies a family fallback and should have been on family. */
                fallbackFor.set(value);
            }
        }
        if (!fallbackFor.isEmpty()) {
            std::unique_ptr<FontFamily>* fallbackFamily =
                    self->fCurrentFamily->fallbackFamilies.find(fallbackFor);
            if (!fallbackFamily) {
                auto newFallbackFamily =
                        std::make_unique<FontFamily>(self->fCurrentFamily->fBasePath, true);
                fallbackFamily = self->fCurrentFamily->fallbackFamilies.set(
                        fallbackFor, std::move(newFallbackFamily));
                (*fallbackFamily)->fLanguages = self->fCurrentFamily->fLanguages;
                (*fallbackFamily)->fVariant = self->fCurrentFamily->fVariant;
                (*fallbackFamily)->fOrder = self->fCurrentFamily->fOrder;
                (*fallbackFamily)->fFallbackFor = fallbackFor;
            }
            self->fCurrentFontInfo = &(*fallbackFamily)->fFonts.emplace_back(file);
            self->fCurrentFamily->fFonts.pop_back();
        }
    },
    /*end*/[](FamilyData* self) {
        trim_string(&self->fCurrentFontInfo->fFileName);
    },
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "axis") {
            return &axisHandler;
        }
        return nullptr;
    },
    /*chars*/[](FamilyData* self, std::string_view s) {
        self->fCurrentFontInfo->fFileName.append(s);
    }
};

static const TagHandler familyHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        // 'name' (string) [optional]
        // 'lang' (space separated string) [default ""]
        // 'variant' ("elegant", "compact") [default "default"]
        // If there is no name, this is a fallback only font.
        FontFamily* family = new FontFamily(self->fBasePath, true);
        self->fCurrentFamily.reset(family);

        if (!self->fCurrentFamilyList.isEmpty()) {
            family->fNames.push_back().set(self->fCurrentFamilyList);
            family->fIsFallbackFont = false;
        }

        for (const auto& [name, value] : attributes) {
            if (name == "name") {
                SkAutoAsciiToLC tolc(value.data(), value.size());
                family->fNames.push_back().set(tolc.lc(), tolc.length());
                family->fIsFallbackFont = false;
            } else if (name == "lang") {
                parse_space_separated_languages(value, family->fLanguages);
            } else if (name == "variant") {
                if (value == "elegant") {
                    family->fVariant = kElegant_FontVariant;
                } else if (value == "compact") {
                    family->fVariant = kCompact_FontVariant;
                }
            }
        }
    },
    /*end*/[](FamilyData* self) {
        self->fFamilies.push_back(std::move(self->fCurrentFamily));
    },
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "font") {
            return &fontHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

static const TagHandler familyListHandler = {
        /*start*/ [](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
            for (const auto& [name, value] : attributes) {
                if (name == "name") {
                    SkAutoAsciiToLC tolc(value.data(), value.size());
                    self->fCurrentFamilyList.set(tolc.lc(), tolc.length());
                }
            }
        },
        /*end*/ [](FamilyData* self) { self->fCurrentFamilyList.reset(); },
        /*tag*/
        [](FamilyData* self,
           std::string_view tag,
           SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
            if (tag == "family") {
                return &familyHandler;
            }
            return nullptr;
        },
        /*chars*/ nullptr,
};

static FontFamily* find_family(FamilyData* self, const SkString& familyName) {
    for (std::unique_ptr<FontFamily>& candidate : self->fFamilies) {
        for (const SkString& candidateName : candidate->fNames) {
            if (candidateName == familyName) {
                return &*candidate;
            }
        }
    }
    return nullptr;
}

static const TagHandler aliasHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        // 'name' (string) introduces a new family name.
        // 'to' (string) specifies which (previous) family to alias
        // 'weight' (non-negative integer) [optional]
        // If it *does not* have a weight, 'name' is an alias for the entire 'to' family.
        // If it *does* have a weight, 'name' is a new family consisting of
        // the font(s) with 'weight' from the 'to' family.

        SkString aliasName;
        SkString to;
        int weight = 0;
        for (const auto& [name, value] : attributes) {
            if (name == "name") {
                SkAutoAsciiToLC tolc(value.data(), value.size());
                aliasName.set(tolc.lc(), tolc.length());
            } else if (name == "to") {
                SkAutoAsciiToLC tolc(value.data(), value.size());
                to.set(tolc.lc(), tolc.length());
            } else if (name == "weight") {
                if (!parse_non_negative_integer(value, &weight)) {
                    SK_FONTCONFIGPARSER_WARNING("'%.*s' is an invalid weight",
                                                SkToInt(value.size()),
                                                value.data());
                }
            }
        }

        // Assumes that the named family is already declared
        FontFamily* targetFamily = find_family(self, to);
        if (!targetFamily) {
            SK_FONTCONFIGPARSER_WARNING("'%s' alias target not found", to.c_str());
            return;
        }

        if (weight) {
            auto family =
                    std::make_unique<FontFamily>(targetFamily->fBasePath, self->fIsFallback);
            family->fNames.push_back().set(aliasName);

            for (int i = 0; i < targetFamily->fFonts.size(); i++) {
                if (targetFamily->fFonts[i].fWeight == weight) {
                    family->fFonts.push_back(targetFamily->fFonts[i]);
                }
            }
            self->fFamilies.push_back(std::move(family));
        } else {
            targetFamily->fNames.push_back().set(aliasName);
        }
    },
    /*end*/nullptr,
    /*tag*/nullptr,
    /*chars*/nullptr,
};

static const TagHandler familySetHandler = {
    /*start*/nullptr,
    /*end*/nullptr,
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "family") {
            return &familyHandler;
        } else if (tag == "family-list") {
            return &familyListHandler;
        } else if (tag == "alias") {
            return &aliasHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

}  // namespace lmpParser

namespace jbParser {

static const TagHandler fileHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        // 'variant' ("elegant", "compact") [default "default"]
        // 'lang' (string) [default ""]
        // 'index' (non-negative integer) [default 0]
        // The character data should be a filename.
        FontFamily& currentFamily = *self->fCurrentFamily;
        FontFileInfo& newFileInfo = currentFamily.fFonts.push_back();
        for (const auto& [name, value] : attributes) {
            if (name == "variant") {
                const FontVariant prevVariant = currentFamily.fVariant;
                if (value == "elegant") {
                    currentFamily.fVariant = kElegant_FontVariant;
                } else if (value == "compact") {
                    currentFamily.fVariant = kCompact_FontVariant;
                }
                if (currentFamily.fFonts.size() > 1 && currentFamily.fVariant != prevVariant) {
                    SK_FONTCONFIGPARSER_WARNING(
                            "'%.*s' unexpected variant found\n"
                            "Note: Every font file within a family must have identical variants.",
                            SkToInt(value.size()),
                            value.data());
                }

            } else if (name == "lang") { // JB MR1 (API level 17)
                SkLanguage currentLanguage = SkLanguage(value.data(), value.size());
                bool showWarning = false;
                if (currentFamily.fLanguages.empty()) {
                    showWarning = (currentFamily.fFonts.size() > 1);
                    currentFamily.fLanguages.push_back(std::move(currentLanguage));
                } else if (currentFamily.fLanguages[0] != currentLanguage) {
                    showWarning = true;
                    currentFamily.fLanguages[0] = std::move(currentLanguage);
                }
                if (showWarning) {
                    SK_FONTCONFIGPARSER_WARNING(
                            "'%.*s' unexpected language found\n"
                            "Note: Every font file within a family must have identical languages.",
                            SkToInt(value.size()),
                            value.data());
                }

            } else if (name == "index") {
                if (!parse_non_negative_integer(value, &newFileInfo.fIndex)) {
                    SK_FONTCONFIGPARSER_WARNING("'%.*s' is an invalid index",
                                                SkToInt(value.size()),
                                                value.data());
                }
            }
        }
        self->fCurrentFontInfo = &newFileInfo;
    },
    /*end*/nullptr,
    /*tag*/nullptr,
    /*chars*/[](FamilyData* self, std::string_view s) {
        self->fCurrentFontInfo->fFileName.append(s);
    }
};

static const TagHandler fileSetHandler = {
    /*start*/nullptr,
    /*end*/nullptr,
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "file") {
            return &fileHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

static const TagHandler nameHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        // The character data should be a name for the font.
        self->fCurrentFamily->fNames.push_back();
    },
    /*end*/nullptr,
    /*tag*/nullptr,
    /*chars*/[](FamilyData* self, std::string_view s) {
        SkAutoAsciiToLC tolc(s.data(), s.size());
        self->fCurrentFamily->fNames.back().append(tolc.lc(), tolc.length());
    }
};

static const TagHandler nameSetHandler = {
    /*start*/nullptr,
    /*end*/nullptr,
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "name") {
            return &nameHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

static const TagHandler familyHandler = {
    /*start*/[](FamilyData* self, SkSpan<const SkXMLParser::Attribute> attributes) {
        self->fCurrentFamily = std::make_unique<FontFamily>(self->fBasePath, self->fIsFallback);
        // 'order' (non-negative integer) [default -1]
        for (const auto& attr : attributes) {
            parse_non_negative_integer(attr.fValue, &self->fCurrentFamily->fOrder);
        }
    },
    /*end*/[](FamilyData* self) {
        self->fFamilies.push_back(std::move(self->fCurrentFamily));
    },
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "nameset") {
            return &nameSetHandler;
        } else if (tag == "fileset") {
            return &fileSetHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

static const TagHandler familySetHandler = {
    /*start*/nullptr,
    /*end*/nullptr,
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "family") {
            return &familyHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

} // namespace jbParser

static const TagHandler topLevelHandler = {
    /*start*/nullptr,
    /*end*/nullptr,
    /*tag*/[](FamilyData* self,
              std::string_view tag,
              SkSpan<const SkXMLParser::Attribute> attributes) -> const TagHandler* {
        if (tag == "familyset") {
            // 'version' (non-negative integer) [default 0]
            for (const auto& [name, value] : attributes) {
                if (name == "version") {
                    if (parse_non_negative_integer(value, &self->fVersion)) {
                        if (self->fVersion >= 21) {
                            return &lmpParser::familySetHandler;
                        }
                    }
                }
            }
            return &jbParser::familySetHandler;
        } else if (tag == "fonts-modification") {
            // Modern OEM customization files (e.g. /product/etc/fonts_customization.xml)
            // use the <fonts-modification> root tag instead of <familyset>.
            //
            // Skia natively supports parsing the standard children of this file:
            // - `<family name="...">`: Instantiated perfectly as new global named families.
            // - `<font>` and `<axis>`: Parsed and bound correctly to the new named families.
            // - `<alias>`: Mapped properly to existing named families.
            //
            // What is NOT supported:
            // 1. `customizationType="append-to-existing"`
            //    If an OEM writes `<family customizationType="append-to-existing" name="sans-serif">`
            //    Minikin would find the existing `sans-serif` family from `fonts.xml` and merge
            //    these new fonts directly into it. Skia's parser ignores `customizationType`
            //    entirely. By purely looking at `name="sans-serif"`, Skia will simply instantiate
            //    a second, completely independent `FontFamily` named "sans-serif". This can lead
            //    to shadowing instead of merging. (OEMs primarily use `new-named-family` which
            //    Skia maps perfectly).
            //
            // 2. `supportedAxes="wght,ital"`
            //    If an OEM writes `<font supportedAxes="wght">`, Minikin restricts the exposed
            //    variable font axes of the TTF to only weight. Skia completely ignores the
            //    `supportedAxes` attribute, meaning all valid axes contained within the font file
            //    remain fully exposed and animatable.

            // Force the modern parser.
            self->fVersion = 21;
            return &lmpParser::familySetHandler;
        }
        return nullptr;
    },
    /*chars*/nullptr,
};

void FamilyData::onStartElement(std::string_view elem, SkSpan<const Attribute> attributes) {
    FamilyData* self = this;

    if (!fSkip) {
        const TagHandler* parent = fHandler.back();
        const TagHandler* child = parent->tag ? parent->tag(this, elem, attributes) : nullptr;
        if (child) {
            if (child->start) {
                child->start(this, attributes);
            }
            fHandler.push_back(child);
        } else {
            SK_FONTCONFIGPARSER_WARNING(
                    "'%.*s' tag not recognized, skipping", SkToInt(elem.size()), elem.data());
            fSkip = fDepth;
        }
    }

    ++fDepth;
}

void FamilyData::onEndElement(std::string_view elem) {
    --fDepth;

    if (!fSkip) {
        const TagHandler* child = fHandler.back();
        if (child->end) {
            child->end(this);
        }
        fHandler.pop_back();
    }

    if (fSkip == fDepth) {
        fSkip = 0;
    }
}

/**
 * This function parses the given filename and stores the results in the given
 * families array. Returns the version of the file, negative if the file does not exist.
 */
static int parse_config_file(const char* filename,
                             std::vector<std::unique_ptr<FontFamily>>& families,
                             const SkString& basePath, bool isFallback)
{
    SkFILEStream file(filename);

    // Some of the files we attempt to parse (in particular, /vendor/etc/fallback_fonts.xml)
    // are optional - failure here is okay because one of these optional files may not exist.
    if (!file.isValid()) {
        SkDebugf(SK_FONTMGR_ANDROID_PARSER_PREFIX "'%s' could not be opened\n", filename);
        return -1;
    }

    FamilyData self(families, basePath, isFallback, filename, &topLevelHandler);
    if (!self.parse(file)) {
        return -1;
    }
    return self.fVersion;
}

/** Returns the version of the system font file actually found, negative if none. */
static int append_system_font_families(std::vector<std::unique_ptr<FontFamily>>& fontFamilies,
                                       const SkString& basePath)
{
    size_t initialCount = fontFamilies.size();
    int version = parse_config_file(LMP_SYSTEM_FONTS_FILE, fontFamilies, basePath, false);
    if (version < 0 || fontFamilies.size() == initialCount) {
        version = parse_config_file(OLD_SYSTEM_FONTS_FILE, fontFamilies, basePath, false);
    }
    return version;
}

/**
 * In some versions of Android prior to Android 4.2 (JellyBean MR1 at API
 * Level 17) the fallback fonts for certain locales were encoded in their own
 * XML files with a suffix that identified the locale.  We search the provided
 * directory for those files,add all of their entries to the fallback chain, and
 * include the locale as part of each entry.
 */
static void append_fallback_font_families_for_locale(
        std::vector<std::unique_ptr<FontFamily>>& fallbackFonts,
        const char* dir,
        const SkString& basePath)
{
    SkOSFile::Iter iter(dir, nullptr);
    SkString fileName;
    while (iter.next(&fileName, false)) {
        // The size of the prefix and suffix.
        static const size_t fixedLen = sizeof(LOCALE_FALLBACK_FONTS_PREFIX) - 1
                                     + sizeof(LOCALE_FALLBACK_FONTS_SUFFIX) - 1;

        // The size of the prefix, suffix, and a minimum valid language code
        static const size_t minSize = fixedLen + 2;

        if (fileName.size() < minSize ||
            !fileName.startsWith(LOCALE_FALLBACK_FONTS_PREFIX) ||
            !fileName.endsWith(LOCALE_FALLBACK_FONTS_SUFFIX))
        {
            continue;
        }

        SkString locale(fileName.c_str() + sizeof(LOCALE_FALLBACK_FONTS_PREFIX) - 1,
                        fileName.size() - fixedLen);

        SkString absoluteFilename;
        absoluteFilename.printf("%s/%s", dir, fileName.c_str());

        std::vector<std::unique_ptr<FontFamily>> langSpecificFonts;
        parse_config_file(absoluteFilename.c_str(), langSpecificFonts, basePath, true);

        for (std::unique_ptr<FontFamily>& family : langSpecificFonts) {
            family->fLanguages.emplace_back(locale);
            fallbackFonts.push_back(std::move(family));
        }
    }
}

static void append_system_fallback_font_families(
    std::vector<std::unique_ptr<FontFamily>>& fallbackFonts,
    const SkString& basePath)
{
    parse_config_file(FALLBACK_FONTS_FILE, fallbackFonts, basePath, true);
    append_fallback_font_families_for_locale(fallbackFonts,
                                             LOCALE_FALLBACK_FONTS_SYSTEM_DIR,
                                             basePath);
}

static void mixin_vendor_fallback_font_families(
        std::vector<std::unique_ptr<FontFamily>>& fallbackFonts,
        const SkString& basePath)
{
    std::vector<std::unique_ptr<FontFamily>> vendorFonts;
    parse_config_file(VENDOR_FONTS_FILE, vendorFonts, basePath, true);
    append_fallback_font_families_for_locale(vendorFonts,
                                             LOCALE_FALLBACK_FONTS_VENDOR_DIR,
                                             basePath);

    // This loop inserts the vendor fallback fonts in the correct order in the
    // overall fallbacks list.
    int currentOrder = -1;
    for (std::unique_ptr<FontFamily>& family : vendorFonts) {
        int order = family->fOrder;
        if (order < 0) {
            if (currentOrder < 0) {
                // Default case - just add it to the end of the fallback list
                fallbackFonts.push_back(std::move(family));
            } else {
                // no order specified on this font, but we're incrementing the order
                // based on an earlier order insertion request
                fallbackFonts.insert(fallbackFonts.begin() + currentOrder, std::move(family));
                ++currentOrder;
            }
        } else {
            // Add the font into the fallback list in the specified order. Set
            // currentOrder for correct placement of other fonts in the vendor list.
            fallbackFonts.insert(fallbackFonts.begin() + order, std::move(family));
            currentOrder = order + 1;
        }
    }
}

void SkFontMgr_Android_Parser::GetSystemFontFamilies(
        std::vector<std::unique_ptr<FontFamily>>& fontFamilies) {
    static constexpr char kFontFilePrefix[] = "/fonts/";

    // Version 21 of the system font configuration does not need any fallback configuration files.
    SkString basePath(getenv("ANDROID_ROOT"));
    basePath.append(kFontFilePrefix);

    if (append_system_font_families(fontFamilies, basePath) >= 21) {
        // Product fonts (fonts_customization.xml) leverage <fonts-modification version="21">
        // Therefore, we only attempt to parse them on modern devices (Android Lollipop+).
        // Pre-21 systems fall through to the legacy fallback fonts handling below.
        SkFontMgr_Android_Parser::GetCustomFontFamilies(
                fontFamilies, basePath, PRODUCT_CUSTOMIZATION_FILE, nullptr, nullptr);
        return;
    }

    // Append all the fallback fonts to system fonts
    std::vector<std::unique_ptr<FontFamily>> fallbackFonts;
    append_system_fallback_font_families(fallbackFonts, basePath);
    mixin_vendor_fallback_font_families(fallbackFonts, basePath);
    fontFamilies.insert(fontFamilies.end(),
                        std::make_move_iterator(fallbackFonts.begin()),
                        std::make_move_iterator(fallbackFonts.end()));
}

void SkFontMgr_Android_Parser::GetCustomFontFamilies(
        std::vector<std::unique_ptr<FontFamily>>& fontFamilies,
        const SkString& basePath,
        const char* fontsXml,
        const char* fallbackFontsXml,
        const char* langFallbackFontsDir)
{
    if (fontsXml) {
        parse_config_file(fontsXml, fontFamilies, basePath, false);
    }
    if (fallbackFontsXml) {
        parse_config_file(fallbackFontsXml, fontFamilies, basePath, true);
    }
    if (langFallbackFontsDir) {
        append_fallback_font_families_for_locale(fontFamilies,
                                                 langFallbackFontsDir,
                                                 basePath);
    }
}

SkLanguage SkLanguage::getParent() const {
    SkASSERT(!fTag.isEmpty());
    const char* tag = fTag.c_str();

    // strip off the rightmost "-.*"
    const char* parentTagEnd = strrchr(tag, '-');
    if (parentTagEnd == nullptr) {
        return SkLanguage();
    }
    size_t parentTagLen = parentTagEnd - tag;
    return SkLanguage(tag, parentTagLen);
}
