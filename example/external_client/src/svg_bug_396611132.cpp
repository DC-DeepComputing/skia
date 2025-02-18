/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDocument.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/svg/SkSVGCanvas.h"

#include <iostream>
#include <string>
#include <memory>

std::string generateSVG() {
    SkRect boundsRect = SkRect::MakeXYWH(0, 0, 100, 100);
    SkRect clipRect = SkRect::MakeXYWH(10, 10, 80, 80);
    SkRect drawRect = SkRect::MakeXYWH(20, 20, 60, 60);

    SkDynamicMemoryWStream stream;
    auto svgCanvas = SkSVGCanvas::Make(boundsRect, &stream);

    svgCanvas->save();
    svgCanvas->clipRect(clipRect);
    svgCanvas->drawRect(drawRect, SkPaint());
    svgCanvas->restore();

    auto data = stream.detachAsData();
    return std::string(reinterpret_cast<const char*>(data->data()), data->size());
}

int main() {
    std::string svg1 = generateSVG();
    std::string svg2 = generateSVG();

    std::cout << "SVG 1: " << svg1 << std::endl;
    std::cout << "SVG 2: " << svg2 << std::endl;

    std::cout << "Are the same? " << (svg1 == svg2 ? "yes" : "no") << std::endl;
    return 0;
}