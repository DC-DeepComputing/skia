/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "src/utils/SkUTF.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#if !defined(SKUNICODE_IMPLEMENTATION)
    #define SKUNICODE_IMPLEMENTATION 0
#endif

#if !defined(SKUNICODE_API)
    #if defined(SKUNICODE_DLL)
        #if defined(_MSC_VER)
            #if SKUNICODE_IMPLEMENTATION
                #define SKUNICODE_API __declspec(dllexport)
            #else
                #define SKUNICODE_API __declspec(dllimport)
            #endif
        #else
            #define SKUNICODE_API __attribute__((visibility("default")))
        #endif
    #else
        #define SKUNICODE_API
    #endif
#endif

class SKUNICODE_API SkBidiIterator {
public:
    typedef int32_t Position;
    typedef uint8_t Level;
    struct Region {
        Region(Position start, Position end, Level level)
            : start(start), end(end), level(level) { }
        Position start;
        Position end;
        Level level;
    };
    enum Direction {
        kLTR,
        kRTL,
    };
    virtual ~SkBidiIterator() = default;
    virtual Position getLength() = 0;
    virtual Level getLevelAt(Position) = 0;
};
class SKUNICODE_API SkBreakIterator {
public:
    typedef int32_t Position;
    typedef int32_t Status;
    virtual ~SkBreakIterator() = default;
    virtual Position first() = 0;
    virtual Position current() = 0;
    virtual Position next() = 0;
    virtual bool isDone() = 0;
    virtual bool setText(const char utftext8[], int utf8Units) = 0;
    virtual bool setText(const char16_t utftext16[], int utf16Units) = 0;
};
class SKUNICODE_API SkUnicode {
    public:
        enum CodeUnitFlags {
            kNoCodeUnitFlag = 0x00,
            kPartOfWhiteSpaceBreak = 0x01,
            kGraphemeStart = 0x02,
            kSoftLineBreakBefore = 0x04,
            kHardLineBreakBefore = 0x08,
            kPartOfIntraWordBreak = 0x10,
            kControl = 0x20,
            kTabulation = 0x40,
        };
        enum class TextDirection {
            kLTR,
            kRTL,
        };
        typedef size_t Position;
        typedef uint8_t BidiLevel;
        struct BidiRegion {
            BidiRegion(Position start, Position end, BidiLevel level)
              : start(start), end(end), level(level) { }
            Position start;
            Position end;
            BidiLevel level;
        };
        enum class LineBreakType {
            kSoftLineBreak = 0,
            kHardLineBreak = 100,
        };

        enum class BreakType {
            kWords,
            kGraphemes,
            kLines
        };
        struct LineBreakBefore {
            LineBreakBefore(Position pos, LineBreakType breakType)
              : pos(pos), breakType(breakType) { }
            Position pos;
            LineBreakType breakType;
        };

        virtual ~SkUnicode() = default;

        virtual SkString toUpper(const SkString&) = 0;

        // Methods used in SkShaper and SkText
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const char text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator
            (const char locale[], BreakType breakType) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType type) = 0;

        // Methods used in SkParagraph
        virtual bool getBidiRegions(const char utf8[],
                                    int utf8Units,
                                    TextDirection dir,
                                    std::vector<BidiRegion>* results) = 0;
        virtual bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getAllCodepoints(char utf8[], int utf8Units, bool replaceTabs,
                                      SkTArray<SkUnicode::CodeUnitFlags, true>* results) = 0;
        virtual bool getAllCodepoints(char16_t utf16[], int utf16Units, bool replaceTabs,
                                      SkTArray<SkUnicode::CodeUnitFlags, true>* results) = 0;
        static SkString convertUtf16ToUtf8(const char16_t * utf16, int utf16Units);
        static SkString convertUtf16ToUtf8(const std::u16string& utf16);
        static std::u16string convertUtf8ToUtf16(const char* utf8, int utf8Units);
        static std::u16string convertUtf8ToUtf16(const SkString& utf8);

        static bool isTabulation(SkUnicode::CodeUnitFlags flags);
        static bool isHardLineBreak(SkUnicode::CodeUnitFlags flags);
        static bool isControl(SkUnicode::CodeUnitFlags flags);
        static bool isPartOfWhiteSpaceBreak(SkUnicode::CodeUnitFlags flags);

        template <typename Callback>
        void forEachCodepoint(const char* utf8, int32_t utf8Units, Callback&& callback) {
            const char* current = utf8;
            const char* end = utf8 + utf8Units;
            while (current < end) {
                auto before = current - utf8;
                SkUnichar unichar = SkUTF::NextUTF8(&current, end);
                if (unichar < 0) unichar = 0xFFFD;
                auto after = current - utf8;
                uint16_t buffer[2];
                size_t count = SkUTF::ToUTF16(unichar, buffer);
                callback(unichar, before, after, count);
            }
        }

        template <typename Callback>
        void forEachCodepoint(const char16_t* utf16, int32_t utf16Units, Callback&& callback) {
            const char16_t* current = utf16;
            const char16_t* end = utf16 + utf16Units;
            while (current < end) {
                auto before = current - utf16;
                SkUnichar unichar = SkUTF::NextUTF16((const uint16_t**)&current, (const uint16_t*)end);
                auto after = current - utf16;
                callback(unichar, before, after);
            }
        }

        virtual void reorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) = 0;

        static std::unique_ptr<SkUnicode> Make();

        static std::unique_ptr<SkUnicode> Make(SkSpan<const char> text,
                                               std::vector<SkUnicode::BidiRegion> bidiRegions,
                                               std::vector<SkUnicode::Position> graphemeBreaks,
                                               std::vector<SkUnicode::LineBreakBefore> lineBreaks);
};

namespace sknonstd {
    template <> struct is_bitmask_enum<SkUnicode::CodeUnitFlags> : std::true_type {};
}  // namespace sknonstd
#endif // SkUnicode_DEFINED