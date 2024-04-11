/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFUnion.h"
#include "src/pdf/SkPDFUtils.h"

#include <optional>

SkPDFIndirectReference SkPDFMakeFormXObject(SkPDFDocument* doc,
                                            std::unique_ptr<SkStreamAsset> content,
                                            std::unique_ptr<SkPDFArray> mediaBox,
                                            std::unique_ptr<SkPDFDict> resourceDict,
                                            const SkMatrix& inverseTransform,
                                            std::optional<SkPDFUnion>&& colorSpace) {
    std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict();
    dict->insertName("Type", "XObject");
    dict->insertName("Subtype", "Form");
    if (!inverseTransform.isIdentity()) {
        dict->insertObject("Matrix", SkPDFUtils::MatrixToArray(inverseTransform));
    }
    dict->insertObject("Resources", std::move(resourceDict));
    dict->insertObject("BBox", std::move(mediaBox));

    // Right now FormXObject is only used for saveLayer, which implies
    // isolated blending.  Do this conditionally if that changes.
    // TODO(halcanary): Is this comment obsolete, since we use it for
    // alpha masks?
    auto group = SkPDFMakeDict("Group");
    group->insertName("S", "Transparency");
    if (colorSpace.has_value()) {
        group->insertUnion("CS", std::move(colorSpace).value());
    }
    group->insertBool("I", true);  // Isolated.
    dict->insertObject("Group", std::move(group));
    return SkPDFStreamOut(std::move(dict), std::move(content), doc);
}
