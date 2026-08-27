/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "src/capture/SkCapture.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordCanvas.h"
#include "src/core/SkRecordDraw.h"
#include "tools/flags/CommandLineFlags.h"

#include <cstdio>

// RecordDumper is a functor for inspecting canvas commands via SkRecord::visit()
// (see src/core/SkRecord.h).
//
// Specific overloads (e.g. operator()(const SkRecords::DrawRect&)) customize formatting
// or manage indentation state (Save/Restore). The generic template fallback handles all
// other SkRecords::* types by default via NameOf().
class RecordDumper {
public:
    // totalCommands is the number of operations in the record, used to calculate
    // the number of digits needed to format the command index column.
    RecordDumper(SkCanvas* canvas, int totalCommands, FILE* output = stdout)
            : fDigits(0)
            , fIndent(0)
            , fIndex(0)
            , fOutput(output)
            , fDraw(canvas, nullptr, nullptr, 0, nullptr) {
        while (totalCommands > 0) {
            totalCommands /= 10;
            fDigits++;
        }
        if (fDigits < 2) fDigits = 2;
    }

    template <typename T> void operator()(const T& command) {
        auto start = SkTime::GetNSecs();
        fDraw(command);
        print(command, SkTime::GetNSecs() - start);
    }

    void operator()(const SkRecords::NoOp&) {}

    template <typename T> void print(const T& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);
    }

    void print(const SkRecords::Restore& command, double ns) {
        --fIndent;
        this->visitingPrinter(NameOf(command), ns);
    }

    void print(const SkRecords::Save& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);
        ++fIndent;
    }

    void print(const SkRecords::SaveLayer& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);
        ++fIndent;
    }

    void print(const SkRecords::DrawPicture& command, double ns) {
        this->visitingPrinter(NameOf(command), ns);

        if (auto bp = SkPicturePriv::AsSkBigPicture(command.picture)) {
            ++fIndent;

            const SkRecord& record = *bp->record();
            for (int i = 0; i < record.count(); i++) {
                record.visit(i, *this);
            }

            --fIndent;
        }
    }

    void print(const SkRecords::DrawAnnotation& command, double ns) {
        int us = (int)(ns * 1e-3);
        if (!FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            printf("    ");
        }
        if (FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("DrawAnnotation [%g %g %g %g] %s\n",
               command.rect.left(),
               command.rect.top(),
               command.rect.right(),
               command.rect.bottom(),
               command.key.c_str());
    }

    void print(const SkRecords::DrawRect& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawOval& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawOval [%g, %g, %g, %g]",
                 command.oval.left(),
                 command.oval.top(),
                 command.oval.right(),
                 command.oval.bottom());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawImage& command, double ns) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawImage [%g, %g]", command.left, command.top);
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawImageRect& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawImageRect dst=[%g, %g, %g, %g]",
                 command.dst.left(),
                 command.dst.top(),
                 command.dst.right(),
                 command.dst.bottom());
        visitingPrinter(buf, ns);
    }

    void print(const SkRecords::DrawPaint& command, double ns) {
        visitingPrinter("DrawPaint (clear/paint)", ns);
    }

    void print(const SkRecords::ClipRect& command, double ns) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "ClipRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        visitingPrinter(buf, ns);
    }

protected:
    virtual void visitingPrinter(const char* name, double ns) = 0;

    template <typename T> void printNameAndTime(const T& command, double ns) {
        int us = (int)(ns * 1e-3);
        if (!FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        printf("%*d ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; i++) {
            printf("    ");
        }
        if (FLAGS_timeWithCommand) {
            printf("%6dus  ", us);
        }
        puts(NameOf(command));
    }

    // NameOf returns the string name of an SkRecord command type.
    // It uses the X-macro pattern via SK_RECORD_TYPES (defined in src/core/SkRecord.h):
    // SK_RECORD_TYPES(M) expands macro M over all SkRecord command types.
    // CASE(U) generates a case label for SkRecords::U##_Type returning its stringified name "#U".
    template <typename T> static const char* NameOf(const T&) {
#define CASE(U)               \
    case SkRecords::U##_Type: \
        return #U;
        switch (T::kType) { SK_RECORD_TYPES(CASE) }
#undef CASE
        SkDEBUGFAIL("Unknown T");
        return "Unknown T";
    }

    static const char* NameOf(const SkRecords::SaveLayer&) {
        return "\x1b[31;1mSaveLayer\x1b[0m";  // Bold red.
    }

    int fDigits;    // Number of digits for formatting the command index column.
    int fIndent;    // Current indentation level to represent Save/SaveLayer nesting depth.
    int fIndex;     // Zero-based index of the current command being dumped.
    FILE* fOutput;  // The file to write the output to. If null, print to std-out
    SkRecords::Draw fDraw;
};
