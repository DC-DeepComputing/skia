/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieValue.h"

namespace sksg {
class Path;
}  // namespace sksg

namespace skottie::internal {

class PathAdapter final : public DiscardableAdapterBase<PathAdapter, sksg::Path> {
public:
    PathAdapter(const skjson::Value&, const AnimationBuilder&);

private:
    void onSync() override;

    ShapeValue fShape;

    using INHERITED = DiscardableAdapterBase<PathAdapter, sksg::Path>;
};

}  // namespace internal::skottie
