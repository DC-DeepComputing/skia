/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_gl_Functions_DEFINED
#define skgpu_gl_Functions_DEFINED

#include <cstring>
#include "experimental/graphite/include/gl/Types.h"
#include "include/private/SkTLogic.h"

namespace skgpu::gl {

extern "C" {

///////////////////////////////////////////////////////////////////////////////

using GLActiveTextureFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum texture);
using GLAttachShaderFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program, GLuint shader);
using GLBeginQueryFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLuint id);
using GLBindAttribLocationFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program,
                                                          GLuint index,
                                                          const char* name);
using GLBindBufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLuint buffer);
using GLBindFramebufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLuint framebuffer);
using GLBindRenderbufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLuint renderbuffer);
using GLBindTextureFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLuint texture);
using GLBindFragDataLocationFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program,
                                                            GLuint colorNumber,
                                                            const GLchar* name);
using GLBindFragDataLocationIndexedFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program,
                                                                   GLuint colorNumber,
                                                                   GLuint index,
                                                                   const GLchar* name);
using GLBindSamplerFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint unit, GLuint sampler);
using GLBindVertexArrayFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint array);
using GLBlendBarrierFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLBlendColorFn = GLvoid GR_GL_FUNCTION_TYPE(GLclampf red,
                                                  GLclampf green,
                                                  GLclampf blue,
                                                  GLclampf alpha);
using GLBlendEquationFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode);
using GLBlendFuncFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum sfactor, GLenum dfactor);
using GLBlitFramebufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLint srcX0,
                                                       GLint srcY0,
                                                       GLint srcX1,
                                                       GLint srcY1,
                                                       GLint dstX0,
                                                       GLint dstY0,
                                                       GLint dstX1,
                                                       GLint dstY1,
                                                       GLbitfield mask,
                                                       GLenum filter);
using GLBufferDataFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                  GLsizeiptr size,
                                                  const GLvoid* data,
                                                  GLenum usage);
using GLBufferSubDataFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                     GLintptr offset,
                                                     GLsizeiptr size,
                                                     const GLvoid* data);
using GLCheckFramebufferStatusFn = GLenum GR_GL_FUNCTION_TYPE(GLenum target);
using GLClearFn = GLvoid GR_GL_FUNCTION_TYPE(GLbitfield mask);
using GLClearColorFn = GLvoid GR_GL_FUNCTION_TYPE(GLclampf red,
                                                  GLclampf green,
                                                  GLclampf blue,
                                                  GLclampf alpha);
using GLClearStencilFn = GLvoid GR_GL_FUNCTION_TYPE(GLint s);
using GLClearTexImageFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLuint texture, GLint level, GLenum format, GLenum type, const GLvoid* data);
using GLClearTexSubImageFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint texture,
                                                        GLint level,
                                                        GLint xoffset,
                                                        GLint yoffset,
                                                        GLint zoffset,
                                                        GLsizei width,
                                                        GLsizei height,
                                                        GLsizei depth,
                                                        GLenum format,
                                                        GLenum type,
                                                        const GLvoid* data);
using GLColorMaskFn = GLvoid GR_GL_FUNCTION_TYPE(GLboolean red,
                                                 GLboolean green,
                                                 GLboolean blue,
                                                 GLboolean alpha);
using GLCompileShaderFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint shader);
using GLCompressedTexImage2DFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                            GLint level,
                                                            GLenum internalformat,
                                                            GLsizei width,
                                                            GLsizei height,
                                                            GLint border,
                                                            GLsizei imageSize,
                                                            const GLvoid* data);
using GLCompressedTexSubImage2DFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                               GLint level,
                                                               GLint xoffset,
                                                               GLint yoffset,
                                                               GLsizei width,
                                                               GLsizei height,
                                                               GLenum format,
                                                               GLsizei imageSize,
                                                               const GLvoid* data);
using GLCopyTexSubImage2DFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                         GLint level,
                                                         GLint xoffset,
                                                         GLint yoffset,
                                                         GLint x,
                                                         GLint y,
                                                         GLsizei width,
                                                         GLsizei height);
using GLCreateProgramFn = GLuint GR_GL_FUNCTION_TYPE();
using GLCreateShaderFn = GLuint GR_GL_FUNCTION_TYPE(GLenum type);
using GLCullFaceFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode);
using GLDeleteBuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* buffers);
using GLDeleteFencesFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* fences);
using GLDeleteFramebuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* framebuffers);
using GLDeleteProgramFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program);
using GLDeleteQueriesFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* ids);
using GLDeleteRenderbuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* renderbuffers);
using GLDeleteSamplersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei count, const GLuint* samplers);
using GLDeleteShaderFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint shader);
using GLDeleteTexturesFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* textures);
using GLDeleteVertexArraysFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLuint* arrays);
using GLDepthMaskFn = GLvoid GR_GL_FUNCTION_TYPE(GLboolean flag);
using GLDisableFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum cap);
using GLDisableVertexAttribArrayFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint index);
using GLDrawArraysFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode, GLint first, GLsizei count);
using GLDrawArraysInstancedFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                                           GLint first,
                                                           GLsizei count,
                                                           GLsizei primcount);
using GLDrawArraysIndirectFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode, const GLvoid* indirect);
using GLDrawBufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode);
using GLDrawBuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, const GLenum* bufs);
using GLDrawElementsFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                                    GLsizei count,
                                                    GLenum type,
                                                    const GLvoid* indices);
using GLDrawElementsInstancedFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount);
using GLDrawElementsIndirectFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                                            GLenum type,
                                                            const GLvoid* indirect);
using GLDrawRangeElementsFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);
using GLEnableFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum cap);
using GLEnableVertexAttribArrayFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint index);
using GLEndQueryFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target);
using GLFinishFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLFinishFenceFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint fence);
using GLFlushFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLFlushMappedBufferRangeFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                              GLintptr offset,
                                                              GLsizeiptr length);
using GLFramebufferRenderbufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                               GLenum attachment,
                                                               GLenum renderbuffertarget,
                                                               GLuint renderbuffer);
using GLFramebufferTexture2DFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
using GLFramebufferTexture2DMultisampleFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                                       GLenum attachment,
                                                                       GLenum textarget,
                                                                       GLuint texture,
                                                                       GLint level,
                                                                       GLsizei samples);
using GLFrontFaceFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode);
using GLGenBuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* buffers);
using GLGenFencesFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* fences);
using GLGenFramebuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* framebuffers);
using GLGenerateMipmapFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target);
using GLGenQueriesFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* ids);
using GLGenRenderbuffersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* renderbuffers);
using GLGenSamplersFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei count, GLuint* samplers);
using GLGenTexturesFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* textures);
using GLGenVertexArraysFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei n, GLuint* arrays);
using GLGetBufferParameterivFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                            GLenum pname,
                                                            GLint* params);
using GLGetErrorFn = GLenum GR_GL_FUNCTION_TYPE();
using GLGetFramebufferAttachmentParameterivFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                                           GLenum attachment,
                                                                           GLenum pname,
                                                                           GLint* params);
using GLGetIntegervFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum pname, GLint* params);
using GLGetMultisamplefvFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum pname, GLuint index, GLfloat* val);
using GLGetProgramBinaryFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLuint program, GLsizei bufsize, GLsizei* length, GLenum* binaryFormat, void* binary);
using GLGetProgramInfoLogFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program,
                                                         GLsizei bufsize,
                                                         GLsizei* length,
                                                         char* infolog);
using GLGetProgramivFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program, GLenum pname, GLint* params);
using GLGetQueryivFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum GLtarget, GLenum pname, GLint* params);
using GLGetQueryObjecti64vFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint id, GLenum pname, GLint64* params);
using GLGetQueryObjectivFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint id, GLenum pname, GLint* params);
using GLGetQueryObjectui64vFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint id,
                                                           GLenum pname,
                                                           GLuint64* params);
using GLGetQueryObjectuivFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint id, GLenum pname, GLuint* params);
using GLGetRenderbufferParameterivFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                                  GLenum pname,
                                                                  GLint* params);
using GLGetShaderInfoLogFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint shader,
                                                        GLsizei bufsize,
                                                        GLsizei* length,
                                                        char* infolog);
using GLGetShaderivFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint shader, GLenum pname, GLint* params);
using GLGetShaderPrecisionFormatFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum shadertype,
                                                                GLenum precisiontype,
                                                                GLint* range,
                                                                GLint* precision);
using GLGetStringFn = const GLubyte* GR_GL_FUNCTION_TYPE(GLenum name);
using GLGetStringiFn = const GLubyte* GR_GL_FUNCTION_TYPE(GLenum name, GLuint index);
using GLGetTexLevelParameterivFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                              GLint level,
                                                              GLenum pname,
                                                              GLint* params);
using GLGetUniformLocationFn = GLint GR_GL_FUNCTION_TYPE(GLuint program, const char* name);
using GLInsertEventMarkerFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei length, const char* marker);
using GLInvalidateBufferDataFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint buffer);
using GLInvalidateBufferSubDataFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint buffer,
                                                               GLintptr offset,
                                                               GLsizeiptr length);
using GLInvalidateFramebufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                             GLsizei numAttachments,
                                                             const GLenum* attachments);
using GLInvalidateSubFramebufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                                GLsizei numAttachments,
                                                                const GLenum* attachments,
                                                                GLint x,
                                                                GLint y,
                                                                GLsizei width,
                                                                GLsizei height);
using GLInvalidateTexImageFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint texture, GLint level);
using GLInvalidateTexSubImageFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint texture,
                                                             GLint level,
                                                             GLint xoffset,
                                                             GLint yoffset,
                                                             GLint zoffset,
                                                             GLsizei width,
                                                             GLsizei height,
                                                             GLsizei depth);
using GLIsTextureFn = GLboolean GR_GL_FUNCTION_TYPE(GLuint texture);
using GLLineWidthFn = GLvoid GR_GL_FUNCTION_TYPE(GLfloat width);
using GLLinkProgramFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program);
using GLMapBufferFn = GLvoid* GR_GL_FUNCTION_TYPE(GLenum target, GLenum access);
using GLMapBufferRangeFn = GLvoid* GR_GL_FUNCTION_TYPE(GLenum target,
                                                       GLintptr offset,
                                                       GLsizeiptr length,
                                                       GLbitfield access);
using GLMapBufferSubDataFn = GLvoid* GR_GL_FUNCTION_TYPE(GLuint target,
                                                         GLintptr offset,
                                                         GLsizeiptr size,
                                                         GLenum access);
using GLMapTexSubImage2DFn = GLvoid* GR_GL_FUNCTION_TYPE(GLenum target,
                                                         GLint level,
                                                         GLint xoffset,
                                                         GLint yoffset,
                                                         GLsizei width,
                                                         GLsizei height,
                                                         GLenum format,
                                                         GLenum type,
                                                         GLenum access);
using GLMemoryBarrierFn = GLvoid* GR_GL_FUNCTION_TYPE(GLbitfield barriers);
using GLPatchParameteriFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum pname, GLint value);
using GLPixelStoreiFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum pname, GLint param);
using GLPolygonModeFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum face, GLenum mode);
using GLPopGroupMarkerFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLProgramBinaryFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program,
                                                     GLenum binaryFormat,
                                                     void* binary,
                                                     GLsizei length);
using GLProgramParameteriFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program, GLenum pname, GLint value);
using GLPushGroupMarkerFn = GLvoid GR_GL_FUNCTION_TYPE(GLsizei length, const char* marker);
using GLQueryCounterFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint id, GLenum target);
using GLReadBufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum src);
using GLReadPixelsFn = GLvoid GR_GL_FUNCTION_TYPE(GLint x,
                                                  GLint y,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  GLenum format,
                                                  GLenum type,
                                                  GLvoid* pixels);
using GLRenderbufferStorageFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                           GLenum internalformat,
                                                           GLsizei width,
                                                           GLsizei height);
using GLRenderbufferStorageMultisampleFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
using GLResolveMultisampleFramebufferFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLSamplerParameteriFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint sampler,
                                                         GLenum pname,
                                                         GLint params);
using GLSamplerParameterivFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint sampler,
                                                          GLenum pname,
                                                          const GLint* params);
using GLScissorFn = GLvoid GR_GL_FUNCTION_TYPE(GLint x, GLint y, GLsizei width, GLsizei height);
// GL_CHROMIUM_bind_uniform_location
using GLBindUniformLocationFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program,
                                                           GLint location,
                                                           const char* name);
using GLSetFenceFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint fence, GLenum condition);
using GLShaderSourceFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint shader,
                                                    GLsizei count,
                                                    const char* const* str,
                                                    const GLint* length);
using GLStencilFuncFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum func, GLint ref, GLuint mask);
using GLStencilFuncSeparateFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum face,
                                                           GLenum func,
                                                           GLint ref,
                                                           GLuint mask);
using GLStencilMaskFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint mask);
using GLStencilMaskSeparateFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum face, GLuint mask);
using GLStencilOpFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum fail, GLenum zfail, GLenum zpass);
using GLStencilOpSeparateFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum face,
                                                         GLenum fail,
                                                         GLenum zfail,
                                                         GLenum zpass);
using GLTexBufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                 GLenum internalformat,
                                                 GLuint buffer);
using GLTexBufferRangeFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
using GLTexImage2DFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                  GLint level,
                                                  GLint internalformat,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  GLint border,
                                                  GLenum format,
                                                  GLenum type,
                                                  const GLvoid* pixels);
using GLTexParameterfFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLenum pname, GLfloat param);
using GLTexParameterfvFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                      GLenum pname,
                                                      const GLfloat* params);
using GLTexParameteriFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target, GLenum pname, GLint param);
using GLTexParameterivFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                      GLenum pname,
                                                      const GLint* params);
using GLTexStorage2DFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
using GLDiscardFramebufferFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                          GLsizei numAttachments,
                                                          const GLenum* attachments);
using GLTestFenceFn = GLboolean GR_GL_FUNCTION_TYPE(GLuint fence);
using GLTexSubImage2DFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum target,
                                                     GLint level,
                                                     GLint xoffset,
                                                     GLint yoffset,
                                                     GLsizei width,
                                                     GLsizei height,
                                                     GLenum format,
                                                     GLenum type,
                                                     const GLvoid* pixels);
using GLTextureBarrierFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLUniform1fFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLfloat v0);
using GLUniform1iFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLint v0);
using GLUniform1fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLfloat* v);
using GLUniform1ivFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLint* v);
using GLUniform2fFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLfloat v0, GLfloat v1);
using GLUniform2iFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLint v0, GLint v1);
using GLUniform2fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLfloat* v);
using GLUniform2ivFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLint* v);
using GLUniform3fFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location,
                                                 GLfloat v0,
                                                 GLfloat v1,
                                                 GLfloat v2);
using GLUniform3iFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLint v0, GLint v1, GLint v2);
using GLUniform3fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLfloat* v);
using GLUniform3ivFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLint* v);
using GLUniform4fFn =
        GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
using GLUniform4iFn =
        GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
using GLUniform4fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLfloat* v);
using GLUniform4ivFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location, GLsizei count, const GLint* v);
using GLUniformMatrix2fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location,
                                                        GLsizei count,
                                                        GLboolean transpose,
                                                        const GLfloat* value);
using GLUniformMatrix3fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location,
                                                        GLsizei count,
                                                        GLboolean transpose,
                                                        const GLfloat* value);
using GLUniformMatrix4fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLint location,
                                                        GLsizei count,
                                                        GLboolean transpose,
                                                        const GLfloat* value);
using GLUnmapBufferFn = GLboolean GR_GL_FUNCTION_TYPE(GLenum target);
using GLUnmapBufferSubDataFn = GLvoid GR_GL_FUNCTION_TYPE(const GLvoid* mem);
using GLUnmapTexSubImage2DFn = GLvoid GR_GL_FUNCTION_TYPE(const GLvoid* mem);
using GLUseProgramFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint program);
using GLVertexAttrib1fFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint indx, const GLfloat value);
using GLVertexAttrib2fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint indx, const GLfloat* values);
using GLVertexAttrib3fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint indx, const GLfloat* values);
using GLVertexAttrib4fvFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint indx, const GLfloat* values);
using GLVertexAttribDivisorFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint index, GLuint divisor);
using GLVertexAttribIPointerFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLuint indx, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr);
using GLVertexAttribPointerFn = GLvoid GR_GL_FUNCTION_TYPE(GLuint indx,
                                                           GLint size,
                                                           GLenum type,
                                                           GLboolean normalized,
                                                           GLsizei stride,
                                                           const GLvoid* ptr);
using GLViewportFn = GLvoid GR_GL_FUNCTION_TYPE(GLint x, GLint y, GLsizei width, GLsizei height);

/* GL_NV_framebuffer_mixed_samples */
using GLCoverageModulationFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum components);

/* EXT_base_instance */
using GLDrawArraysInstancedBaseInstanceFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
using GLDrawElementsInstancedBaseVertexBaseInstanceFn =
        GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                   GLsizei count,
                                   GLenum type,
                                   const void* indices,
                                   GLsizei instancecount,
                                   GLint basevertex,
                                   GLuint baseinstance);

/* EXT_multi_draw_indirect */
using GLMultiDrawArraysIndirectFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                                               const GLvoid* indirect,
                                                               GLsizei drawcount,
                                                               GLsizei stride);
using GLMultiDrawElementsIndirectFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum mode, GLenum type, const GLvoid* indirect, GLsizei drawcount, GLsizei stride);

/* ANGLE_base_vertex_base_instance */
using GLMultiDrawArraysInstancedBaseInstanceFn =
        GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                   const GLint* firsts,
                                   const GLsizei* counts,
                                   const GLsizei* instanceCounts,
                                   const GLuint* baseInstances,
                                   const GLsizei drawcount);
using GLMultiDrawElementsInstancedBaseVertexBaseInstanceFn =
        GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                   const GLint* counts,
                                   GLenum type,
                                   const GLvoid* const* indices,
                                   const GLsizei* instanceCounts,
                                   const GLint* baseVertices,
                                   const GLuint* baseInstances,
                                   const GLsizei drawcount);

/* ARB_sync */
using GLFenceSyncFn = GLsync GR_GL_FUNCTION_TYPE(GLenum condition, GLbitfield flags);
using GLIsSyncFn = GLboolean GR_GL_FUNCTION_TYPE(GLsync sync);
using GLClientWaitSyncFn = GLenum GR_GL_FUNCTION_TYPE(GLsync sync,
                                                      GLbitfield flags,
                                                      GLuint64 timeout);
using GLWaitSyncFn = GLvoid GR_GL_FUNCTION_TYPE(GLsync sync, GLbitfield flags, GLuint64 timeout);
using GLDeleteSyncFn = GLvoid GR_GL_FUNCTION_TYPE(GLsync sync);

/* ARB_internalformat_query */
using GLGetInternalformativFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

/* KHR_debug */
using GLDebugMessageControlFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum source,
                                                           GLenum type,
                                                           GLenum severity,
                                                           GLsizei count,
                                                           const GLuint* ids,
                                                           GLboolean enabled);
using GLDebugMessageInsertFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
using GLDebugMessageCallbackFn = GLvoid GR_GL_FUNCTION_TYPE(GRGLDEBUGPROC callback,
                                                            const GLvoid* userParam);
using GLGetDebugMessageLogFn = GLuint GR_GL_FUNCTION_TYPE(GLuint count,
                                                          GLsizei bufSize,
                                                          GLenum* sources,
                                                          GLenum* types,
                                                          GLuint* ids,
                                                          GLenum* severities,
                                                          GLsizei* lengths,
                                                          GLchar* messageLog);
using GLPushDebugGroupFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum source,
                                                      GLuint id,
                                                      GLsizei length,
                                                      const GLchar* message);
using GLPopDebugGroupFn = GLvoid GR_GL_FUNCTION_TYPE();
using GLObjectLabelFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum identifier,
                                                   GLuint name,
                                                   GLsizei length,
                                                   const GLchar* label);

/** EXT_window_rectangles */
using GLWindowRectanglesFn = GLvoid GR_GL_FUNCTION_TYPE(GLenum mode,
                                                        GLsizei count,
                                                        const GLint box[]);

/** GL_QCOM_tiled_rendering */
using GLStartTilingFn = GLvoid GR_GL_FUNCTION_TYPE(
        GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
using GLEndTilingFn = GLvoid GR_GL_FUNCTION_TYPE(GLbitfield preserveMask);

/** EGL functions */
using EGLQueryStringFn = const char* GR_GL_FUNCTION_TYPE(EGLDisplay dpy, EGLint name);
using EGLGetCurrentDisplayFn = EGLDisplay GR_GL_FUNCTION_TYPE();
using EGLCreateImageFn = EGLImage GR_GL_FUNCTION_TYPE(EGLDisplay dpy,
                                                      EGLContext ctx,
                                                      EGLenum target,
                                                      EGLClientBuffer buffer,
                                                      const GLint* attrib_list);
using EGLDestroyImageFn = EGLBoolean GR_GL_FUNCTION_TYPE(EGLDisplay dpy, EGLImage image);
}  // extern "C"

// This is a lighter-weight std::function, trying to reduce code size and compile time
// by only supporting the exact use cases we require.
template <typename T> class Function;

template <typename R, typename... Args> class Function<R GR_GL_FUNCTION_TYPE(Args...)> {
public:
    using Fn = R GR_GL_FUNCTION_TYPE(Args...);
    // Construct empty.
    Function() = default;
    Function(std::nullptr_t) {}

    // Construct from a simple function pointer.
    Function(Fn* fn_ptr) {
        static_assert(sizeof(fn_ptr) <= sizeof(fBuf), "fBuf is too small");
        if (fn_ptr) {
            memcpy(fBuf, &fn_ptr, sizeof(fn_ptr));
            fCall = [](const void* buf, Args... args) {
                return (*(Fn**)buf)(std::forward<Args>(args)...);
            };
        }
    }

    // Construct from a small closure.
    template <typename Closure> Function(Closure closure) : Function() {
        static_assert(sizeof(Closure) <= sizeof(fBuf), "fBuf is too small");
#if defined(__APPLE__)  // I am having serious trouble getting these to work with all STLs...
        static_assert(std::is_trivially_copyable<Closure>::value, "");
        static_assert(std::is_trivially_destructible<Closure>::value, "");
#endif

        memcpy(fBuf, &closure, sizeof(closure));
        fCall = [](const void* buf, Args... args) {
            auto closure = (const Closure*)buf;
            return (*closure)(args...);
        };
    }

    R operator()(Args... args) const {
        SkASSERT(fCall);
        return fCall(fBuf, std::forward<Args>(args)...);
    }

    explicit operator bool() const { return fCall != nullptr; }

    void reset() { fCall = nullptr; }

private:
    using Call = R(const void* buf, Args...);
    Call* fCall = nullptr;
    size_t fBuf[4];
};

}  // namespace skgpu::gl

#endif  // skgpu_gl_Functions_DEFINED
