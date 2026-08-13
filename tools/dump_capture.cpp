/*
 * Copyright 2025 Google LLC
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

#include <cstdio>

class RecordDumper {
public:
    RecordDumper(int total) : fDigits(0), fIndent(0), fIndex(0) {
        while (total > 0) {
            total /= 10;
            fDigits++;
        }
        if (fDigits < 2) fDigits = 2;
    }

    template <typename T>
    void operator()(const T& command) {
        this->printName(NameOf(command));
    }

    void operator()(const SkRecords::NoOp&) {}

    void operator()(const SkRecords::Restore& command) {
        if (fIndent > 0) --fIndent;
        this->printName(NameOf(command));
    }

    void operator()(const SkRecords::Save& command) {
        this->printName(NameOf(command));
        ++fIndent;
    }

    void operator()(const SkRecords::SaveLayer& command) {
        this->printName(NameOf(command));
        ++fIndent;
    }

    void operator()(const SkRecords::DrawRect& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawRect [%g, %g, %g, %g]",
                 command.rect.left(), command.rect.top(), command.rect.right(), command.rect.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawOval& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawOval [%g, %g, %g, %g]",
                 command.oval.left(), command.oval.top(), command.oval.right(), command.oval.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawImage& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawImage [%g, %g]", command.left, command.top);
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawImageRect& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawImageRect dst=[%g, %g, %g, %g]",
                 command.dst.left(), command.dst.top(), command.dst.right(), command.dst.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawPaint& command) {
        this->printName("DrawPaint (clear/paint)");
    }

    void operator()(const SkRecords::ClipRect& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "ClipRect [%g, %g, %g, %g]",
                 command.rect.left(), command.rect.top(), command.rect.right(), command.rect.bottom());
        this->printName(buf);
    }

private:
    void printName(const char* name) {
        printf("      [%*d] ", fDigits, fIndex++);
        for (int i = 0; i < fIndent; ++i) {
            printf("  ");
        }
        printf("%s\n", name);
    }

    template <typename T>
    static const char* NameOf(const T&) {
    #define CASE(U) case SkRecords::U##_Type: return #U;
        switch (T::kType) { SK_RECORD_TYPES(CASE) }
    #undef CASE
        return "Unknown";
    }

    int fDigits;
    int fIndent;
    int fIndex;
};

void dump_capture_file(const char* path) {
    printf("================================================================================\n");
    printf("CAPTURE FILE: %s\n", path);
    printf("================================================================================\n");

    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    if (!data) {
        printf("Error: Could not read file '%s'\n\n", path);
        return;
    }

    printf("File Size: %zu bytes\n", data->size());

    sk_sp<SkCapture> capture = SkCapture::MakeFromData(data);
    if (!capture) {
        printf("Error: Failed to deserialize SkCapture from '%s'\n\n", path);
        return;
    }

    SkCapture::Metadata meta = capture->getMetadata();
    printf("Capture Format Version: %u\n", meta.version);
    printf("Number of Recorded Pictures: %u\n", meta.numPictures);
    printf("--------------------------------------------------------------------------------\n");

    for (uint32_t i = 0; i < meta.numPictures; ++i) {
        sk_sp<SkPicture> picture = capture->getPicture(i);
        if (!picture) {
            printf("  Picture [%u]: <null>\n", i);
            continue;
        }

        SkRect cull = picture->cullRect();
        printf("  Picture [%u]: Dimensions %dx%d (cull: [%g, %g, %g, %g])\n",
               i,
               SkScalarCeilToInt(cull.width()),
               SkScalarCeilToInt(cull.height()),
               cull.left(), cull.top(), cull.right(), cull.bottom());

        const int w = SkScalarCeilToInt(cull.width());
        const int h = SkScalarCeilToInt(cull.height());
        SkRecord record;
        SkRecordCanvas recCanvas(&record, w, h);
        picture->playback(&recCanvas);

        printf("    Record count: %d operations\n", record.count());
        if (record.count() == 0) {
            printf("    (Empty picture - snapped at endCapture for idle surface)\n");
        } else {
            RecordDumper dumper(record.count());
            for (int j = 0; j < record.count(); ++j) {
                record.visit(j, dumper);
            }
        }
        printf("\n");
    }
    printf("================================================================================\n\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <file1.capt> [file2.capt ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        dump_capture_file(argv[i]);
    }
    return 0;
}
