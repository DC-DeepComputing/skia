/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/gl/Util.h"
#include <stdio.h>

namespace skgpu::gl {

GLVersion GLGetVersionFromString(const char* versionString) {
    if (!versionString) {
        SkDebugf("nullptr GL version string.");
        return GR_GL_INVALID_VER;
    }

    int major, minor;

    // check for mesa
    int mesaMajor, mesaMinor;
    int n = sscanf(versionString, "%d.%d Mesa %d.%d", &major, &minor, &mesaMajor, &mesaMinor);
    if (4 == n) {
        return GR_GL_VER(major, minor);
    }

    n = sscanf(versionString, "%d.%d", &major, &minor);
    if (2 == n) {
        return GR_GL_VER(major, minor);
    }

    // WebGL might look like "OpenGL ES 2.0 (WebGL 1.0 (OpenGL ES 2.0 Chromium))"
    int esMajor, esMinor;
    n = sscanf(versionString, "OpenGL ES %d.%d (WebGL %d.%d", &esMajor, &esMinor, &major, &minor);
    if (4 == n) {
        return GR_GL_VER(major, minor);
    }

    char profile[2];
    n = sscanf(versionString, "OpenGL ES-%c%c %d.%d", profile, profile + 1, &major, &minor);
    if (4 == n) {
        return GR_GL_VER(major, minor);
    }

    n = sscanf(versionString, "OpenGL ES %d.%d", &major, &minor);
    if (2 == n) {
        return GR_GL_VER(major, minor);
    }

    return GR_GL_INVALID_VER;
}

}  // namespace skgpu::gl
