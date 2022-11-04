/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkGetExecutablePath.h"
#include <windows.h>

std::string SkGetExecutablePath() {
    char buff[MAX_PATH];
    DWORD executablePathLen = GetModuleFileNameA(nullptr, buff, MAX_PATH);
    if executablePathLen <= 0 {
        return "";
    }
    buff[executablePathLen] = 0;
    return std::string(buff);
}
