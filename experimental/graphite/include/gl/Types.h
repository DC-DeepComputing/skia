/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_gl_Types_DEFINED
#define skgpu_gl_Types_DEFINED

#include "experimental/graphite/include/gl/Config.h"

namespace skgpu::gl {

/**
 * Classifies GL contexts by which standard they implement (currently as OpenGL vs. OpenGL ES).
 */
enum GLStandard {
    kNone_GLStandard,
    kGL_GLStandard,
    kGLES_GLStandard,
    kWebGL_GLStandard,
};
static const int kGLStandardCnt = 4;

// The following allow certain interfaces to be turned off at compile time
// (for example, to lower code size).
#if SK_ASSUME_GL_ES
#define GR_IS_GR_GL(standard) false
#define GR_IS_GR_GL_ES(standard) true
#define GR_IS_GR_WEBGL(standard) false
#define SK_DISABLE_GL_INTERFACE 1
#define SK_DISABLE_WEBGL_INTERFACE 1
#elif SK_ASSUME_GL
#define GR_IS_GR_GL(standard) true
#define GR_IS_GR_GL_ES(standard) false
#define GR_IS_GR_WEBGL(standard) false
#define SK_DISABLE_GL_ES_INTERFACE 1
#define SK_DISABLE_WEBGL_INTERFACE 1
#elif SK_ASSUME_WEBGL
#define GR_IS_GR_GL(standard) false
#define GR_IS_GR_GL_ES(standard) false
#define GR_IS_GR_WEBGL(standard) true
#define SK_DISABLE_GL_ES_INTERFACE 1
#define SK_DISABLE_GL_INTERFACE 1
#else
#define GR_IS_GR_GL(standard) (kGL_GLStandard == standard)
#define GR_IS_GR_GL_ES(standard) (kGLES_GLStandard == standard)
#define GR_IS_GR_WEBGL(standard) (kWebGL_GLStandard == standard)
#endif

///////////////////////////////////////////////////////////////////////////////

/**
 * The supported GL formats represented as an enum. Actual support by GrContext depends on GL
 * context version and extensions.
 */
enum class Format {
    kUnknown,

    kRGBA8,
    kR8,
    kALPHA8,
    kLUMINANCE8,
    kLUMINANCE8_ALPHA8,
    kBGRA8,
    kRGB565,
    kRGBA16F,
    kR16F,
    kRGB8,
    kRG8,
    kRGB10_A2,
    kRGBA4,
    kSRGB8_ALPHA8,
    kCOMPRESSED_ETC1_RGB8,
    kCOMPRESSED_RGB8_ETC2,
    kCOMPRESSED_RGB8_BC1,
    kCOMPRESSED_RGBA8_BC1,
    kR16,
    kRG16,
    kRGBA16,
    kRG16F,
    kLUMINANCE16F,

    kLastColorFormat = kLUMINANCE16F,

    // Depth/Stencil formats
    kSTENCIL_INDEX8,
    kSTENCIL_INDEX16,
    kDEPTH24_STENCIL8,

    kLast = kDEPTH24_STENCIL8
};

///////////////////////////////////////////////////////////////////////////////
/**
 * Declares typedefs for all the GL functions used in Interface
 */

using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLbyte = signed char;
using GLchar = char;
using GLshort = short;
using GLint = int;
using GLsizei = int;
using GLint64 = int64_t;
using GLubyte = unsigned char;
using GLushort = unsigned short;
using GLuint = unsigned int;
using GLuint64 = uint64_t;
using GLhalf = unsigned short int;
using GLfloat = float;
using GLclampf = float;
using GLdouble = double;
using GLclampd = double;
using GLvoid = void;
#ifdef _WIN64
using GLintptr = signed long long int;
using GLsizeiptr = signed long long int;
#else
using GLintptr = signed long int;
using GLsizeiptr = signed long int;
#endif
using GLeglImage = void*;
using GLsync = struct __GLsync*;

struct GLDrawArraysIndirectCommand {
    GLuint fCount;
    GLuint fInstanceCount;
    GLuint fFirst;
    GLuint fBaseInstance;  // Requires EXT_base_instance on ES.
};

// static_asserts must have messages in this file because its included in C++14 client code.
static_assert(16 == sizeof(GLDrawArraysIndirectCommand), "");

struct GLDrawElementsIndirectCommand {
    GLuint fCount;
    GLuint fInstanceCount;
    GLuint fFirstIndex;
    GLuint fBaseVertex;
    GLuint fBaseInstance;  // Requires EXT_base_instance on ES.
};

static_assert(20 == sizeof(GLDrawElementsIndirectCommand), "");

/**
 * KHR_debug
 */
typedef void(GR_GL_FUNCTION_TYPE* GRGLDEBUGPROC)(GLenum source,
                                                 GLenum type,
                                                 GLuint id,
                                                 GLenum severity,
                                                 GLsizei length,
                                                 const GLchar* message,
                                                 const void* userParam);

/**
 * EGL types.
 */
using EGLImage = void*;
using EGLDisplay = void*;
using EGLContext = void*;
using EGLClientBuffer = void*;
using EGLenum = unsigned int;
using EGLint = int32_t;
using EGLBoolean = unsigned int;

}  // namespace skgpu::gl

#endif  // skgpu_gl_Types_DEFINED
