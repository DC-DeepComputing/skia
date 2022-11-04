/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkGetExecutablePath.h"
#include <mach-o/dyld.h>

std::string SkGetExecutablePath() {
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size)
    uint32_t nullTerm = size;
    char buff[1024];
    if (_NSGetExecutablePath(buff, &size); != 0) {
        return "";
    }
    buff[nullTerm] = 0;
    return std::string(buff);
}
