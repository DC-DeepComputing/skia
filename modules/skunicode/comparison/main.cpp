// Copyright 2023 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// Proof of principle of a text editor written with Skia & SkShaper.
// https://bugs.skia.org/9020

#include "include/core/SkStream.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkReadBuffer.h"
#include "include/private/base/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"

const size_t limit = 4096*4096;
template<typename F>
static bool readlines(SkFILEStream& input, F callback) {

    auto length = input.getLength();
    input.rewind();

    if (length >= limit) {
        SkDebugf("Too big file, skipped\n");
        return false;
    }
    skia_private::TArray<char> buffer(limit);
    buffer.resize(length);
    if (input.read(buffer.data(), length) != length) {
        SkDebugf("Could not read file, skipped\n");
        return false;
    }

    callback(buffer.data(), length);
    return true;
}

static bool cleanInputs(const char* input) {
    SkString inputs(input);
    inputs.append("/input");
    SkOSFile::Iter iter(inputs.c_str());
    SkString outputs(input);
    outputs.append("/preprocessed");
    SkString path;
    while (iter.next(&path)) {
        SkString text;
        text.printf("%s/%s", inputs.c_str(), path.c_str());
        SkFILEStream input(text.c_str());
        if (!input.isValid()) {
            SkDebugf("File %s is not valid\n", text.c_str());
            continue;
        }
        if (!readlines(input, [&](const char* line, size_t size) {
            SkString str(line, size);
            while (true) {
                auto pos = str.find("<br />");
                if (pos < 0) {
                    break;
                }
                str.remove(pos, 6);
                str.insert(pos, "\n");
            }
            SkString output;
            output.printf("%s/%s", outputs.c_str(), path.c_str());
            SkFILEWStream outputFile(output.c_str());
            outputFile.write(str.c_str(), str.size());
        })) {
            SkDebugf("Could not read ile %s is not valid, skipped\n", text.c_str());
        }
    }
    return true;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        SkDebugf("Missing parameter - a folder with datasets\n.");
        return -1;
    }

    if (argc == 3 && strcmp(argv[2], "clean") == 0) {
        cleanInputs(argv[1]);
        return 0;
    }

    if (argc < 3) {
        SkDebugf("Missing parameter - a result name\n.");
        return -1;
    }
    auto unicode = SkUnicode::Make();
    SkString inputs(argv[1]);
    inputs.append("/input");
    SkOSFile::Iter iter(inputs.c_str());
    SkString outputs(argv[1]);
    outputs.append("/output");
    SkString path;
    while (iter.next(&path)) {
        SkString text;
        text.printf("%s/%s", inputs.c_str(), path.c_str());
        SkFILEStream input(text.c_str());
        if (!input.isValid()) {
            SkDebugf("File %s is not valid\n", text.c_str());
            continue;
        }

        skia_private::TArray<SkString> lines;
        if (!readlines(input, [&](const char* line, size_t size) {
            lines.emplace_back(line, size);
        })) {
            SkDebugf("Could not read ile %s is not valid, skipped\n", text.c_str());
        }

        // Soft line breaks; hard line breaks; grapheme breaks; word breaks; bidi breaks
        SkString resultSoftLineBreaks;
        resultSoftLineBreaks.printf("%s/%s/%s.soft", outputs.c_str(), argv[2], path.c_str());
        SkFILEWStream outputSoftLineBreaks(resultSoftLineBreaks.c_str());
        if (!outputSoftLineBreaks.isValid()) {
            SkDebugf("File %s is not valid\n", resultSoftLineBreaks.c_str());
            continue;
        }
        SkString resultGraphemeBreaks;
        resultGraphemeBreaks.printf("%s/%s/%s.grapheme", outputs.c_str(), argv[2], path.c_str());
        SkFILEWStream outputGraphemeBreaks(resultGraphemeBreaks.c_str());
        if (!outputGraphemeBreaks.isValid()) {
            SkDebugf("File %s is not valid\n", resultGraphemeBreaks.c_str());
            continue;
        }
        SkString softLineBreaks;
        SkString graphemeBreaks;
        for (auto& line : lines) {
            skia_private::TArray<SkUnicode::CodeUnitFlags, true> flags;
            if (!unicode->computeCodeUnitFlags(line.data(), line.size(), false, &flags)) {
                SkDebugf("Could not read line #%lu, skipped\n", &line - &lines[0]);
            }
            for (auto i = 0ul; i < flags.size(); ++i) {
                auto flag = flags[i];
                if (SkUnicode::isSoftLineBreak(flag)) {
                    softLineBreaks += std::to_string(i).c_str();
                    softLineBreaks += " ";
                }
                if (SkUnicode::isGraphemeStart(flag)) {
                    graphemeBreaks += std::to_string(i).c_str();
                    graphemeBreaks += " ";
                }
            }
            graphemeBreaks += "\n";
        }
        outputSoftLineBreaks.write(softLineBreaks.c_str(), softLineBreaks.size());
        outputGraphemeBreaks.write(graphemeBreaks.c_str(), graphemeBreaks.size());
    }
}