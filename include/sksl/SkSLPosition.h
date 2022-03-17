/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POSITION
#define SKSL_POSITION

#include "include/core/SkTypes.h"
#include <cinttypes>
#include <string>

#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

namespace SkSL {

class Position {
public:
    Position()
        : fFile(nullptr)
        , fStartOffsetOrLine(NO_VALUE)
        , fEndOffset(NO_VALUE) {}

    static Position Line(int line, const char* file = nullptr) {
        Position result;
        SkASSERT(line <= MAX_OFFSET);
        result.fFile = file;
        result.fStartOffsetOrLine = line;
        result.fEndOffset = NO_VALUE;
        return result;
    }

    static Position Range(int startOffset, int endOffset) {
        Position result;
        SkASSERT(startOffset <= MAX_OFFSET && endOffset <= MAX_OFFSET);
        result.fFile = nullptr;
        result.fStartOffsetOrLine = startOffset;
        result.fEndOffset = endOffset;
        return result;
    }

#if __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)
    static Position Capture(const char* file = __builtin_FILE(), int line = __builtin_LINE()) {
        return Position::Line(line, file);
    }
#else
    static Position Capture() { return Position(); }
#endif // __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)

    const char* file_name() const {
        return fFile;
    }

    bool valid() const {
        return fStartOffsetOrLine != NO_VALUE;
    }

    int line(std::string_view source = std::string_view()) const;

    int startOffset() const {
        SkASSERT(fEndOffset != NO_VALUE);
        return fStartOffsetOrLine;
    }

    int endOffset() const {
        SkASSERT(fEndOffset != NO_VALUE);
        return fEndOffset;
    }

    bool operator==(const Position& other) const {
        return fFile == other.fFile && fStartOffsetOrLine == other.fStartOffsetOrLine &&
                fEndOffset == other.fEndOffset;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    bool operator>(const Position& other) const {
        return fStartOffsetOrLine > other.fStartOffsetOrLine;
    }

    bool operator>=(const Position& other) const {
        return fStartOffsetOrLine >= other.fStartOffsetOrLine;
    }

    bool operator<(const Position& other) const {
        return fStartOffsetOrLine < other.fStartOffsetOrLine;
    }

    bool operator<=(const Position& other) const {
        return fStartOffsetOrLine <= other.fStartOffsetOrLine;
    }

private:
    // TODO(skia:13051): remove fFile
    const char* fFile = nullptr;
    // Since ever IRNode contains a Position, we use the smallest type we can get away with
    typedef uint16_t offset;
    // Reserve the max value as a sentinel
    static constexpr int NO_VALUE = std::numeric_limits<offset>::max();
    static constexpr int MAX_OFFSET = NO_VALUE - 1;
    // Contains either a start offset (if fEndOffset != NO_VALUE) or a line number
    // (if fEndOffset == NO_VALUE)
    offset fStartOffsetOrLine;
    offset fEndOffset;
};

} // namespace SkSL

#endif
