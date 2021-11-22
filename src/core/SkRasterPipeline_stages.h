/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_stages_DEFINED
#define SkRasterPipeline_stages_DEFINED

/**
 * SkRasterPipeline provides a cheap way to chain together a pixel processing pipeline.
 *
 * It's particularly designed for situations where the potential pipeline is extremely
 * combinatoric: {N dst formats} x {M source formats} x {K mask formats} x {C transfer modes} ...
 * No one wants to write specialized routines for all those combinations, and if we did, we'd
 * end up bloating our code size dramatically.  SkRasterPipeline stages can be chained together
 * at runtime, so we can scale this problem linearly rather than combinatorically.
 *
 * Each stage is represented by a function conforming to a common interface and by an
 * arbitrary context pointer.  The stage function arguments and calling convention are
 * designed to maximize the amount of data we can pass along the pipeline cheaply, and
 * vary depending on CPU feature detection.
 */

#define SK_RASTER_PIPELINE_STAGES(M)                               \
    M(callback)                                                    \
    M(move_src_dst) M(move_dst_src) M(swap_src_dst)                \
    M(clamp_0) M(clamp_1) M(clamp_a) M(clamp_gamut)                \
    M(unpremul) M(premul) M(premul_dst)                            \
    M(force_opaque) M(force_opaque_dst)                            \
    M(set_rgb) M(unbounded_set_rgb) M(swap_rb) M(swap_rb_dst)      \
    M(black_color) M(white_color)                                  \
    M(uniform_color) M(unbounded_uniform_color) M(uniform_color_dst) \
    M(seed_shader) M(dither)                                       \
    M(load_a8)     M(load_a8_dst)   M(store_a8)    M(gather_a8)    \
    M(load_565)    M(load_565_dst)  M(store_565)   M(gather_565)   \
    M(load_4444)   M(load_4444_dst) M(store_4444)  M(gather_4444)  \
    M(load_f16)    M(load_f16_dst)  M(store_f16)   M(gather_f16)   \
    M(load_af16)   M(load_af16_dst) M(store_af16)  M(gather_af16)  \
    M(load_rgf16)  M(load_rgf16_dst) M(store_rgf16) M(gather_rgf16) \
    M(load_f32)    M(load_f32_dst)  M(store_f32)   M(gather_f32)   \
    M(load_rgf32)                   M(store_rgf32)                 \
    M(load_8888)   M(load_8888_dst) M(store_8888)  M(gather_8888)  \
    M(load_rg88)   M(load_rg88_dst) M(store_rg88)  M(gather_rg88)  \
    M(load_a16)    M(load_a16_dst)  M(store_a16)   M(gather_a16)   \
    M(load_rg1616) M(load_rg1616_dst) M(store_rg1616) M(gather_rg1616) \
    M(load_16161616) M(load_16161616_dst) M(store_16161616) M(gather_16161616) \
    M(load_1010102) M(load_1010102_dst) M(store_1010102) M(gather_1010102) \
    M(alpha_to_gray) M(alpha_to_gray_dst)                          \
    M(bt709_luminance_or_luma_to_alpha) M(bt709_luminance_or_luma_to_rgb) \
    M(bilerp_clamp_8888) M(bicubic_clamp_8888)                     \
    M(store_u16_be)                                                \
    M(load_src) M(store_src) M(store_src_a) M(load_dst) M(store_dst) \
    M(scale_u8) M(scale_565) M(scale_1_float) M(scale_native)      \
    M( lerp_u8) M( lerp_565) M( lerp_1_float) M(lerp_native)       \
    M(dstatop) M(dstin) M(dstout) M(dstover)                       \
    M(srcatop) M(srcin) M(srcout) M(srcover)                       \
    M(clear) M(modulate) M(multiply) M(plus_) M(screen) M(xor_)    \
    M(colorburn) M(colordodge) M(darken) M(difference)             \
    M(exclusion) M(hardlight) M(lighten) M(overlay) M(softlight)   \
    M(hue) M(saturation) M(color) M(luminosity)                    \
    M(srcover_rgba_8888)                                           \
    M(matrix_translate) M(matrix_scale_translate)                  \
    M(matrix_2x3) M(matrix_3x3) M(matrix_3x4) M(matrix_4x5) M(matrix_4x3) \
    M(matrix_perspective)                                          \
    M(parametric) M(gamma_) M(PQish) M(HLGish) M(HLGinvish)        \
    M(mirror_x)   M(repeat_x)                                      \
    M(mirror_y)   M(repeat_y)                                      \
    M(decal_x)    M(decal_y)   M(decal_x_and_y)                    \
    M(check_decal_mask)                                            \
    M(negate_x)                                                    \
    M(bilinear) M(bicubic)                                         \
    M(bilinear_nx) M(bilinear_px) M(bilinear_ny) M(bilinear_py)    \
    M(bicubic_n3x) M(bicubic_n1x) M(bicubic_p1x) M(bicubic_p3x)    \
    M(bicubic_n3y) M(bicubic_n1y) M(bicubic_p1y) M(bicubic_p3y)    \
    M(save_xy) M(accumulate)                                       \
    M(clamp_x_1) M(mirror_x_1) M(repeat_x_1)                       \
    M(evenly_spaced_gradient)                                      \
    M(gradient)                                                    \
    M(evenly_spaced_2_stop_gradient)                               \
    M(xy_to_unit_angle)                                            \
    M(xy_to_radius)                                                \
    M(xy_to_2pt_conical_strip)                                     \
    M(xy_to_2pt_conical_focal_on_circle)                           \
    M(xy_to_2pt_conical_well_behaved)                              \
    M(xy_to_2pt_conical_smaller)                                   \
    M(xy_to_2pt_conical_greater)                                   \
    M(alter_2pt_conical_compensate_focal)                          \
    M(alter_2pt_conical_unswap)                                    \
    M(mask_2pt_conical_nan)                                        \
    M(mask_2pt_conical_degenerates) M(apply_vector_mask)           \
    M(byte_tables)                                                 \
    M(rgb_to_hsl) M(hsl_to_rgb)                                    \
    M(gauss_a_to_rgba)                                             \
    M(emboss)                                                      \
    M(swizzle)

#endif  // SkRasterPipeline_stages_DEFINED
