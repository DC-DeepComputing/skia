/// Encapsulate a set of unsafe blocks into a separate file due to review rules of
/// unsafe codes blocks in a downstream repository.
use crate::ffi::ColorPainterWrapper;
use font_types::Point;
use read_fonts::tables::colr::Extend;
use skrifa::color::ColorStop;
use std::pin::Pin;

pub(crate) fn fill_linear(
    color_painter: Pin<&mut ColorPainterWrapper>,
    p0: Point<f32>,
    p1: Point<f32>,
    color_stops: &[ColorStop],
    extend: Extend,
) {
    // SAFETY: The cast from the Skrifa::ColorStop struct to ffi::ColorStop
    // is safe because they have the same layout. The interface function
    // is intentionally taking a raw pointer in order to avoid a copy of the ColorStop
    // array.
    unsafe {
        color_painter.fill_linear(
            p0.x,
            p0.y,
            p1.x,
            p1.y,
            color_stops.as_ptr() as *const crate::ffi::ColorStop,
            color_stops.len(),
            extend as u8,
        );
    }
}

pub(crate) fn fill_radial(
    color_painter: Pin<&mut ColorPainterWrapper>,
    c0: Point<f32>,
    r0: f32,
    c1: Point<f32>,
    r1: f32,
    color_stops: &[ColorStop],
    extend: Extend,
) {
    // SAFETY: The cast from the Skrifa::ColorStop struct to ffi::ColorStop
    // is safe because they have the same layout. The interface function
    // is intentionally taking a raw pointer in order to avoid a copy of the ColorStop
    // array.
    unsafe {
        color_painter.fill_radial(
            c0.x,
            c0.y,
            r0,
            c1.x,
            c1.y,
            r1,
            color_stops.as_ptr() as *const crate::ffi::ColorStop,
            color_stops.len(),
            extend as u8,
        );
    }
}

pub(crate) fn fill_sweep(
    color_painter: Pin<&mut ColorPainterWrapper>,
    c0: Point<f32>,
    start_angle: f32,
    end_angle: f32,
    color_stops: &[ColorStop],
    extend: Extend,
) {
    // SAFETY: The cast from the Skrifa::ColorStop struct to ffi::ColorStop
    // is safe because they have the same layout. The interface function
    // is intentionally taking a raw pointer in order to avoid a copy of the ColorStop
    // array.
    unsafe {
        color_painter.fill_sweep(
            c0.x,
            c0.y,
            start_angle,
            end_angle,
            color_stops.as_ptr() as *const crate::ffi::ColorStop,
            color_stops.len(),
            extend as u8,
        );
    }
}
