/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkGetExecutablePath.h"
#include <cstddef>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>

// Note that /proc/self/exe is Linux-specific; this won't work on other UNIX systems.

std::string SkGetExecutablePath() {
    char buff[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buff, PATH_MAX - 1);
    if (len < 0 || static_cast<size_t>(len) >= PATH_MAX - 1) {
        return "";
    }
    buff[len] = '\0';
    return std::string(buff);
}
