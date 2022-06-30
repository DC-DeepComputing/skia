/*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkString.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/utils/SkUTF.h"
#include <vector>
#include <functional>

class SkBidiIterator_client : public SkBidiIterator {

public:
    explicit SkBidiIterator_client();
    Position getLength() override;
    Level getLevelAt(Position pos) override;

    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t utf16[],
                                                            int utf16Units,
                                                            Direction dir);

    // ICU bidi iterator works with utf16 but clients (Flutter for instance) may work with utf8
    // This method allows the clients not to think about all these details
    static std::unique_ptr<SkBidiIterator> makeBidiIterator(const char utf8[],
                                                            int utf8Units,
                                                            Direction dir);
};

class SkBreakIterator_icu : public SkBreakIterator {
 public:
    explicit SkBreakIterator_icu();
    Position first() override;
    Position current() override;
    Position next() override;
    Position preceding(Position offset) override;
    Position following(Position offset) override;
    Status status() override;
    bool isDone() override;

    bool setText(const char utftext8[], int utf8Units) override;
    bool setText(const char16_t utftext16[], int utf16Units) override;
};

class SkScriptIterator_client : public SkScriptIterator {
 public:
   bool getScript(SkUnichar u, ScriptID* script) override;

   static std::unique_ptr<SkScriptIterator> makeScriptIterator();
};

class SkUnicode_client : public SkUnicode {
    static bool extractBidi(const char utf8[],
                            int utf8Units,
                            TextDirection dir,
                            std::vector<BidiRegion>* bidiRegions);

    static bool extractWords(uint16_t utf16[], int utf16Units, std::vector<Position>* words);

    static bool extractPositions
        (const char utf8[], int utf8Units, BreakType type, std::function<void(int, int)> setBreak);

public:
    ~SkUnicode_client() override { }
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override;
    std::unique_ptr<SkScriptIterator> makeScriptIterator() override;

    static bool isHardLineBreak(SkUnichar utf8);

    bool isControl(SkUnichar utf8) override;

    bool isWhitespace(SkUnichar utf8) override;

    bool isSpace(SkUnichar utf8) override;

    bool isHardBreak(SkUnichar utf8) override;

    SkString toUpper(const SkString& str) override;

    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override;

    bool getLineBreaks(const char utf8[],
                       int utf8Units,
                       std::vector<LineBreakBefore>* results) override;

    bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) override;

    bool getGraphemes(const char utf8[], int utf8Units, std::vector<Position>* results) override;

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override;
};

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    return std::make_unique<SkUnicode_client>();
}
