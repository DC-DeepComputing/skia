/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TRANSFORM
#define SKSL_TRANSFORM

#include "include/private/SkSLProgramKind.h"

#include <vector>

namespace SkSL {

class Context;
class ProgramElement;

class Transform {
public:
    static void FindAndDeclareBuiltinVariables(const Context& context, ProgramKind programKind,
            std::vector<const ProgramElement*>& sharedElements);
};

} // namespace SkSL

#endif
