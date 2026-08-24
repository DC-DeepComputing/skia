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
#include "tools/flags/CommandLineFlags.h"

#include <cstdio>

static DEFINE_string2(input, i, "", "Capture file(s) to dump.");
static DEFINE_bool2(assets, a, true, "Include assets in the dump.");

// Printer class for use with the Visitor Pattern with SkRecord (e.g. record.visit(j, dumper))
// This class overrides the operator() for each type of command, and SkRecord calls "visit" to have
// the RecordDumper visit the command at index "j" in the example above.
class RecordDumper {
public:
    RecordDumper(int total) : fDigits(0), fIndent(0), fIndex(0) {
        while (total > 0) {
            total /= 10;
            fDigits++;
        }
        if (fDigits < 2) fDigits = 2;
    }

    template <typename T> void operator()(const T& command) { this->printName(NameOf(command)); }

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
        snprintf(buf,
                 sizeof(buf),
                 "DrawRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawOval& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawOval [%g, %g, %g, %g]",
                 command.oval.left(),
                 command.oval.top(),
                 command.oval.right(),
                 command.oval.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawImage& command) {
        char buf[128];
        snprintf(buf, sizeof(buf), "DrawImage [%g, %g]", command.left, command.top);
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawImageRect& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "DrawImageRect dst=[%g, %g, %g, %g]",
                 command.dst.left(),
                 command.dst.top(),
                 command.dst.right(),
                 command.dst.bottom());
        this->printName(buf);
    }

    void operator()(const SkRecords::DrawPaint& command) {
        this->printName("DrawPaint (clear/paint)");
    }

    void operator()(const SkRecords::ClipRect& command) {
        char buf[128];
        snprintf(buf,
                 sizeof(buf),
                 "ClipRect [%g, %g, %g, %g]",
                 command.rect.left(),
                 command.rect.top(),
                 command.rect.right(),
                 command.rect.bottom());
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

    // Complex double macro function.
    // SK_RECORD_TYPES enumerates all SkRecord objects like "NOOP(CASE), DRAW(CASE), etc"
    // CASE(U) is a macro we pass into SK_RECORD_TYPES to enumarate each command
    // (i.e. case SkRecords::NOOP_Type: return NOOP;)
    template <typename T>
    static const char* NameOf(const T&) {
        // clang-format off
#define CASE(U) case SkRecords::U##_Type: return #U;
        switch (T::kType) {
            SK_RECORD_TYPES(CASE)
        }
#undef CASE
        return "Unknown";
        // clang-format on
    }

    int fDigits;
    int fIndent;
    int fIndex;
};

void dump_assets(SkCapture::Metadata meta, sk_sp<SkCapture> capture) {
    printf("--------------------------------------------------------------------------------\n");
    printf("ASSETS SECTION (%u assets):\n", meta.numAssets);
    printf("--------------------------------------------------------------------------------\n");

    for (uint32_t i = 0; i < meta.numAssets; ++i) {
        sk_sp<SkPicture> picture = capture->getAsset(i);
        if (!picture) {
            printf("  Picture [%u]: <null>\n", i);
            continue;
        }

        SkRect cull = picture->cullRect();
        printf("  Picture [%u]: Dimensions %dx%d (cull: [%g, %g, %g, %g])\n",
               i,
               SkScalarCeilToInt(cull.width()),
               SkScalarCeilToInt(cull.height()),
               cull.left(),
               cull.top(),
               cull.right(),
               cull.bottom());

        const int w = SkScalarCeilToInt(cull.width());
        const int h = SkScalarCeilToInt(cull.height());
        SkRecord record;
        SkRecordCanvas recCanvas(&record, w, h);
        picture->playback(&recCanvas);

        printf("    Record count: %d operations\n", record.count());
        if (record.count() == 0) {
            printf("    (Empty picture - snapped for idle surface)\n");
        } else {
            RecordDumper dumper(record.count());
            for (int j = 0; j < record.count(); ++j) {
                record.visit(j, dumper);
            }
        }
        printf("\n");
    }
}

void dump_timeline(SkCapture::Metadata meta, sk_sp<SkCapture> capture) {
    printf("--------------------------------------------------------------------------------\n");
    printf("TIMELINE SECTION (%u recordings):\n", meta.numRecordingCaptures);
    printf("--------------------------------------------------------------------------------\n");
    for (uint32_t i = 0; i < meta.numRecordingCaptures; ++i) {
        const SkCapture::RecordingCapture* rec = capture->getRecordingCapture(i);
        if (!rec) {
            printf("  Recording [%u]: <null>\n", i);
            continue;
        }
        printf("  Recording [%u]: %d draw task(s)\n", i, rec->fDrawTasks.size());
        for (int j = 0; j < rec->fDrawTasks.size(); ++j) {
            uint32_t assetIdx = rec->fDrawTasks[j].fAssetIndex;
            sk_sp<SkPicture> pic = capture->getAsset(assetIdx);
            if (pic) {
                SkRect cull = pic->cullRect();
                printf("    [Task %d] -> Asset [%u] (Dimensions: %dx%d, cull: [%g, %g, %g, %g], "
                       "approxOps: %d)\n",
                       j,
                       assetIdx,
                       SkScalarCeilToInt(cull.width()),
                       SkScalarCeilToInt(cull.height()),
                       cull.left(),
                       cull.top(),
                       cull.right(),
                       cull.bottom(),
                       pic->approximateOpCount());
            } else {
                printf("    [Task %d] -> Asset [%u] (<null>)\n", j, assetIdx);
            }
        }
        printf("\n");
    }
}

void dump_capture_file(const char* path, bool include_assets = true) {
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

    // Metadata Section
    SkCapture::Metadata meta = capture->getMetadata();
    printf("Capture Format Version: %u\n", meta.version);
    printf("Number of Recorded Assets: %u\n", meta.numAssets);
    printf("Number of Recording Captures (Timeline): %u\n", meta.numRecordingCaptures);

    // Assets Section
    if (include_assets) dump_assets(meta, capture);

    // Timeline Section
    dump_timeline(meta, capture);

    printf("================================================================================\n\n");
}

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage(
            R"(Dumps the contents and structure of SkCapture files (.capt). dump_capture
            Usage:
                dump_capture -i <file1.capt> [file2.capt ...]
                dump_capture -i <file.capt> --noassets
            )");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.isEmpty()) {
        CommandLineFlags::PrintUsage();
        return 1;
    }

    for (int i = 0; i < FLAGS_input.size(); ++i) {
        dump_capture_file(FLAGS_input[i], FLAGS_assets);
    }
    return 0;
}
