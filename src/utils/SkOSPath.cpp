/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkOSPath.h"

#include "include/core/SkTypes.h"

#include <string.h>

SkString SkOSPath::Join(const char *rootPath, const char *relativePath) {
    SkString result(rootPath);
    if (!result.endsWith(SEPARATOR) && !result.isEmpty()) {
        result.appendUnichar(SEPARATOR);
    }
    result.append(relativePath);
    return result;
}

SkString SkOSPath::Basename(const char* fullPath) {
    if (!fullPath) {
        return SkString();
    }
    const char* filename = strrchr(fullPath, SEPARATOR);
    if (nullptr == filename) {
        filename = fullPath;
    } else {
        ++filename;
    }
    return SkString(filename);
}

SkString SkOSPath::Dirname(const char* fullPath) {
    if (!fullPath) {
        return SkString();
    }
    const char* end = strrchr(fullPath, SEPARATOR);
    if (nullptr == end) {
        return SkString();
    }
    if (end == fullPath) {
        SkASSERT(fullPath[0] == SEPARATOR);
        ++end;
    }
    return SkString(fullPath, end - fullPath);
}

#if defined(SK_BUILD_FOR_WIN)

#include <windows.h>

SkString SkOSPath::GetExecutablePath() {
    char executableFileBuf[MAX_PATH];
    DWORD executablePathLen = GetModuleFileNameA(nullptr, executableFileBuf,
                                                 static_cast<DWORD>(executableFileBuf.size()));
    return (executablePathLen > 0) ? SkString(executableFileBuf) : SkString();
}

#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include <mach-o/dyld.h>

SkString SkOSPath::GetExecutablePath() {
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    SkString result(/*text=*/nullptr, size);
    if (_NSGetExecutablePath(result.writable_str(), &size) != 0) {
        result.reset();
    }
    return result;
}

#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)

// Note that /proc/self/exe is Linux-specific; this won't work on other UNIX systems.

#include <unistd.h>

SkString SkOSPath::GetExecutablePath() {
    SkString result(/*text=*/nullptr, PATH_MAX);
    ssize_t len = readlink("/proc/self/exe", result.writable_str(), result.size() - 1);
    if (len < 0 || static_cast<size_t>(len) >= PATH_MAX - 1) {
        result.reset();
    } else {
        result.resize(len);
    }
    return result;
}

#endif
