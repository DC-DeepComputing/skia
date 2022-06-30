/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h"
#include "src/utils/SkUTF.h"
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
    static void ReorderVisual(const Level runLevels[], int levelsCount, int32_t logicalFromVisual[]);
};

class SKUNICODE_API SkBreakIterator {
public:
    typedef int32_t Position;
    typedef int32_t Status;
    virtual ~SkBreakIterator() = default;
    virtual Position first() = 0;
    virtual Position current() = 0;
    virtual Position next() = 0;
    //virtual Position preceding(Position offset) = 0;
    //virtual Position following(Position offset) = 0;
    //virtual Status status() = 0;
    virtual bool isDone() = 0;
    virtual bool setText(const char utftext8[], int utf8Units) = 0;
    virtual bool setText(const char16_t utftext16[], int utf16Units) = 0;
};

class SKUNICODE_API SkScriptIterator {
 public:
    typedef uint32_t ScriptID;
    virtual ~SkScriptIterator() = default;
    virtual bool getScript(SkUnichar u, ScriptID* script) = 0;
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
            kTabulation = 0x20,
            kControl = 0x40,
        };

        typedef uint32_t CombiningClass;
        typedef uint32_t GeneralCategory;
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

        virtual bool fill(SkSpan<const char> text,
                          std::vector<BidiRegion> bidiRegions,
                          std::vector<Position> grapemes,
                          std::vector<LineBreakBefore> lineBreaks) = 0;

        virtual SkString toUpper(const SkString&) = 0;

        // Methods used in SkShaper and SkText
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const char text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator
            (const char locale[], BreakType breakType) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType type) = 0;
        virtual std::unique_ptr<SkScriptIterator> makeScriptIterator() = 0;

        // High level methods (that we actually use somewhere=SkParagraph)
        virtual bool getBidiRegions(const char utf8[],
                                    int utf8Units,
                                    TextDirection dir,
                                    std::vector<BidiRegion>* results) = 0;
        virtual bool getWords
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getAllCodepoints(const char utf8[],
                                      int utf8Units,
                                      bool replaceTabs,
                                      SkTArray<SkUnicode::CodeUnitFlags, true>* results) = 0;
        virtual bool getAllCodepoints(const char16_t utf16[],
                                      int utf16Units,
                                      bool replaceTabs,
                                      SkTArray<SkUnicode::CodeUnitFlags, true>* results) = 0;

        static SkString convertUtf16ToUtf8(const char16_t * utf16, int utf16Units) {

            int utf8Units = SkUTF::UTF16ToUTF8(nullptr, 0, (uint16_t*)utf16, utf16Units);
            if (utf8Units < 0) {
                SkDEBUGF("Convert error: Invalid utf16 input");
                return SkString();
            }
            SkAutoTArray<char> utf8(utf8Units);
            SkDEBUGCODE(int dstLen =) SkUTF::UTF16ToUTF8(utf8.data(), utf8Units, (uint16_t*)utf16, utf16Units);
            SkASSERT(dstLen == utf8Units);

            return SkString(utf8.data(), utf8Units);
        }

        static SkString convertUtf16ToUtf8(const std::u16string& utf16) {
            return convertUtf16ToUtf8(utf16.c_str(), utf16.size());
        }

        static std::u16string convertUtf8ToUtf16(const char* utf8, int utf8Units) {

            int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
            if (utf16Units < 0) {
                SkDEBUGF("Convert error: Invalid utf8 input");
                return std::u16string();
            }

            SkAutoTArray<uint16_t> utf16(utf16Units);
            SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16.data(), utf16Units, utf8, utf8Units);
            SkASSERT(dstLen == utf16Units);

            return std::u16string((char16_t *)utf16.data(), utf16Units);
        }

        static std::u16string convertUtf8ToUtf16(const SkString& utf8) {
            return convertUtf8ToUtf16(utf8.c_str(), utf8.size());
        }

        template <typename Callback8, typename Callback16>
        bool extractMapping(SkSpan<const char> utf8, Callback8&& callback8, Callback16&& callback16) {
            // Fill out code points 16
            size_t size8 = 0;
            size_t size16 = 0;
            auto ptr = utf8.begin();
            auto end = utf8.end();
            while (ptr < end) {

                size_t index = ptr - utf8.begin();
                SkUnichar u = SkUTF::NextUTF8(&ptr, end);

                // All utf8 units refer to the same codepoint
                size_t next = ptr - utf8.begin();
                for (auto i = index; i < next; ++i) {
                    //fUTF16IndexForUTF8Index.emplace_back(fUTF8IndexForUTF16Index.size());
                    callback16(size8);
                    ++size16;
                }
                //SkASSERT(fUTF16IndexForUTF8Index.size() == next);
                SkASSERT(size16 == next);

                // One or two codepoints refer to the same text index
                uint16_t buffer[2];
                size_t count = SkUTF::ToUTF16(u, buffer);
                //fUTF8IndexForUTF16Index.emplace_back(index);
                callback8(index);
                ++size8;
                if (count > 1) {
                    //fUTF8IndexForUTF16Index.emplace_back(index);
                    callback8(index);
                    ++size8;
                }
            }
            //fUTF16IndexForUTF8Index.emplace_back(fUTF8IndexForUTF16Index.size());
            callback16(size8);
            ++size16;
            //fUTF8IndexForUTF16Index.emplace_back(fText.size());
            callback8(utf8.size());
            ++size8;

            return true;
        }

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
};

namespace sknonstd {
template <> struct is_bitmask_enum<SkUnicode::CodeUnitFlags> : std::true_type {};
}  // namespace sknonstd

#endif // SkUnicode_DEFINED
