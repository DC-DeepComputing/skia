/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/PathAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGPath.h"

namespace skottie::internal {

PathAdapter::PathAdapter(const skjson::Value& jpath, const AnimationBuilder& abuilder)
    : INHERITED(sksg::Path::Make()) {
    this->bind(abuilder, jpath, fShape);
}


void PathAdapter::onSync() {
    const auto& path_node = this->node();

    SkPath path = fShape;

    // FillType is tracked in the SG node, not in keyframes -- make sure we preserve it.
    path.setFillType(path_node->getFillType());
    path.setIsVolatile(!this->isStatic());

    path_node->setPath(path);
}

}  // namespace skottie::internal
