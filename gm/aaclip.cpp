/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/ToolUtils.h"

#include <span>

#include "src/gpu/graphite/DrawCommands.h"
#include "src/sksl/ir/SkSLType.h"

class Image {
public:
    static std::unique_ptr<Image> Make() { return std::make_unique<Image>(); }
};

class Sync {
public:
    Sync() {}

    void wait() {}
};

class Command {
public:
    enum class Type { kWait, kRect, kRRect, kConvexPath, kConcavePath, kImage };

    Type fType = Type::kRect;
    SkColor fColor = SK_ColorCYAN;
    Image* fImage = nullptr;
    Sync fSync {};
};

class CommandBuffer {
public:
    void add_cmd_to_buffer(Command command) {
        SkASSERT(fCur < kLimit);
        fCommands[fCur] = command;
        fCur++;
    }

    bool full() const { return fCur >= kLimit; }
    bool empty() const { return fCur == 0; }

    static std::unique_ptr<CommandBuffer> New() {
        return std::make_unique<CommandBuffer>();
    }

private:
    static constexpr int kLimit = 10;
    int fCur = 0;
    std::array<Command, kLimit> fCommands;
};

class GPU {
public:
    static Sync Execute(CommandBuffer* commands, Image* dst) {
        return Sync();
    }

    static void Display(Image* image) {
        fCurDisplayed = image;
    }

private:
    static Image* fCurDisplayed;
};

Image* GPU::fCurDisplayed = nullptr;

class CPU {
public:
    // Spawns a thread
    static Sync Execute(CommandBuffer* commands, Image* dst) {
        return Sync();
    }

};

void draw_loop(std::span<const Command> drawCalls) {
    std::unique_ptr<Image> finalImages[2];
    finalImages[0] = Image::Make();
    finalImages[1] = Image::Make();
    int curImage = 0;

    std::vector<std::unique_ptr<Image>> tmpImages;
    std::vector<std::unique_ptr<CommandBuffer>> tmpBuffers;

    for (int i = 0; i < 10; ++i) {
        std::vector<std::unique_ptr<CommandBuffer>> buffers;

        buffers.push_back(CommandBuffer::New());
        CommandBuffer* curBuffer = buffers.back().get();

        Sync lastSync;

        for (const Command& dc : drawCalls) {
            if (curBuffer->full()) {
                lastSync = GPU::Execute(curBuffer, finalImages[curImage].get());
                buffers.push_back(CommandBuffer::New());
                curBuffer = buffers.back().get();
            }

            if (dc.fType == Command::Type::kConcavePath) {
                tmpImages.push_back(Image::Make());
                Image* tmpImg = tmpImages.back().get();

                tmpBuffers.push_back(CommandBuffer::New());
                CommandBuffer* tmpBuffer = tmpBuffers.back().get();

                tmpBuffer->add_cmd_to_buffer(dc);
                Sync imgSync = CPU::Execute(tmpBuffer, tmpImg);

                curBuffer->add_cmd_to_buffer({Command::Type::kWait, SK_ColorRED, nullptr, imgSync});
                curBuffer->add_cmd_to_buffer({Command::Type::kImage, SK_ColorRED, tmpImg });
                continue;
            }

            curBuffer->add_cmd_to_buffer(dc);
        }

        if (!curBuffer->empty()) {
            lastSync = GPU::Execute(curBuffer, finalImages[curImage].get());
        }

        lastSync.wait();

        GPU::Display(finalImages[curImage].get());
        curImage = (curImage+1) % 2;
    }
}

void mainish() {
    std::unique_ptr<Image> tmp = Image::Make();

    const Command drawCalls[] = {
        { Command::Type::kRect,  SK_ColorRED },
        { Command::Type::kRRect, SK_ColorBLUE },
        { Command::Type::kConcavePath,  SK_ColorGREEN },
        { Command::Type::kConvexPath,  SK_ColorCYAN },
        { Command::Type::kImage, SK_ColorRED, tmp.get() },
    };

    draw_loop(drawCalls);
}

class ClipCubicGM : public skiagm::GM {
public:
    ClipCubicGM() {}

protected:
    SkString getName() const override { return SkString("clipcubic"); }

    SkISize getISize() override { return SkISize::Make(400, 410); }

    void onDraw(SkCanvas* canvas) override {
        mainish();
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM(return new ClipCubicGM;)
