/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_gl_Util_DEFINED
#define skgpu_gl_Util_DEFINED

#include "experimental/graphite/include/Types.h"
#include "experimental/graphite/src/gl/Defines.h"

////////////////////////////////////////////////////////////////////////////////

namespace skgpu::gl {

using GLVersion = uint32_t;
using GLSLVersion = uint32_t;
using GLDriverVersion = uint64_t;

#define GR_GL_VER(major, minor) \
    ((static_cast<uint32_t>(major) << 16) | static_cast<uint32_t>(minor))
#define GR_GLSL_VER(major, minor) \
    ((static_cast<uint32_t>(major) << 16) | static_cast<uint32_t>(minor))
#define GR_GL_DRIVER_VER(major, minor, point)                                      \
    ((static_cast<uint64_t>(major) << 32) | (static_cast<uint64_t>(minor) << 16) | \
     static_cast<uint64_t>(point))

#define GR_GL_MAJOR_VER(version) (static_cast<uint32_t>(version) >> 16)
#define GR_GL_MINOR_VER(version) (static_cast<uint32_t>(version) & 0xFFFF)

#define GR_GL_INVALID_VER GR_GL_VER(0, 0)
#define GR_GLSL_INVALID_VER GR_GLSL_VER(0, 0)
#define GR_GL_DRIVER_UNKNOWN_VER GR_GL_DRIVER_VER(0, 0, 0)

GLVersion GLGetVersionFromString(const char*);

}  // namespace skgpu::gl

#endif
