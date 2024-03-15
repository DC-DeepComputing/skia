/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "modules/skottie/utils/TextPreshape.h"
#include "src/utils/SkJSON.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(input , i, nullptr, "Input .json file.");
static DEFINE_string2(output, o, nullptr, "Output .json file.");

 int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    SkGraphics::Init();

    if (FLAGS_input.isEmpty() || FLAGS_output.isEmpty()) {
        SkDebugf("Missing required 'input' and 'output' args.\n");
        return 1;
    }

    const auto data = SkData::MakeFromFileName(FLAGS_input[0]);
    if (!data) {
        SkDebugf("Could not read file: %s\n", FLAGS_input[0]);
        return 1;
    }

    SkFILEWStream out(FLAGS_output[0]);
    if (!out.isValid()) {
        SkDebugf("Could not write file: %s\n", FLAGS_output[0]);
        return 1;
    }

    skjson::DOM dom(static_cast<const char*>(data->data()), data->size());
    if (!dom.root().is<skjson::ObjectValue>()) {
        SkDebugf("Could parse JSON file: %s\n", FLAGS_input[0]);
        return 1;
    }

    skottie_utils::Preshape(dom, &out);

    return 0;
 }
