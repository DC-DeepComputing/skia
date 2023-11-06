// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPathBridge_DEFINED
#define SkPathBridge_DEFINED

#include <cstddef>
#include <cstdint>

namespace fontations_ffi {

/** C++ pure virtual interface type, exposed to Rust side to be able to write
 * from Skrifa path output functions to an SkPath type to capture and convert a
 * glyph path. */
class PathWrapper {
public:
    virtual ~PathWrapper() = default;
    virtual void move_to(float x, float y) = 0;
    virtual void line_to(float x, float y) = 0;
    virtual void quad_to(float cx0, float cy0, float x, float y) = 0;
    virtual void curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) = 0;
    virtual void close() = 0;
};

/** C++ pure virtual interface type, exposed to Rust side to be able to write
 * out variation design parameters to the caller-side allocated
 * SkFontParameters::Variation::Axis. A direct cast or mapping between a shared
 * C++/Rust struct and a Skia side struct is not possible because the
 * hidden-axis flag is private on SkFontParameters::Variation::Axis.  */
class AxisWrapper {
public:
    virtual ~AxisWrapper() = default;
    virtual bool populate_axis(
            size_t i, uint32_t axisTag, float min, float def, float max, bool hidden) = 0;
    virtual size_t size() const = 0;
};

/** C++ pure virtual interface, exposed to Rust side for receiving COLRv0/COLRv1 drawing callback
 * matching Skrifa's ColorPainter trait. */
class ColorPainter {
public:
    virtual ~ColorPainter() = default;
    virtual void push_transform(float xx, float xy, float yx, float yy, float dx, float dy);
    virtual void pop_transform();
    virtual void push_clip_glyph(uint16_t glyph_id);
    virtual void pop_clip();

    // Paint*Gradient equivalents:
    virtual void fill_solid(uint16_t palette_index, float alpha);
    // TODO gradients

    // TODO: Enum? Use Skia numbers? Make them match with Fontations numbers?
    virtual void push_layer(uint16_t compositeMode);
    virtual void pop_layer();
};

}  // namespace fontations_ffi

#endif
