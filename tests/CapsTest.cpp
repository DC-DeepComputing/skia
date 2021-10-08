/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"

DEF_GRAPHITE_TEST_FOR_CONTEXTS(CapsTest, reporter) {
    auto caps = context->caps();

}
