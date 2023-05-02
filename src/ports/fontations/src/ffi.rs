// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
use core::pin::Pin;
use core::u8;
use cxx;
use font_types::{GlyphId, Pen};
use read_fonts::{FileRef, FileRef::Font, FontRef, ReadError};
use skrifa::{
    meta::metrics::GlyphMetrics, scale::Context, MetadataProvider, NormalizedCoords, Size,
};

use crate::ffi::SkPathWrapper;

fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16 {
    font_ref.0.as_ref().map_or(0, |f| {
        f.charmap()
            .map(codepoint)
            .map_or_else(|| 0, |id| id.to_u16())
    })
}

struct PenDump<'a> {
    path_wrapper: Pin<&'a mut ffi::SkPathWrapper>,
}

impl<'a> Pen for PenDump<'a> {
    fn move_to(&mut self, x: f32, y: f32) {
        self.path_wrapper.as_mut().move_to(x, -y);
    }

    fn line_to(&mut self, x: f32, y: f32) {
        self.path_wrapper.as_mut().line_to(x, -y);
    }

    fn quad_to(&mut self, cx0: f32, cy0: f32, x: f32, y: f32) {
        self.path_wrapper.as_mut().quad_to(cx0, -cy0, x, -y);
    }

    fn curve_to(&mut self, cx0: f32, cy0: f32, cx1: f32, cy1: f32, x: f32, y: f32) {
        self.path_wrapper
            .as_mut()
            .curve_to(cx0, -cy0, cx1, cy1, x, -y);
    }

    fn close(&mut self) {
        self.path_wrapper.as_mut().close();
    }
}

fn get_path(
    font_ref: &BridgeFontRef,
    glyph_id: u16,
    size: f32,
    path_wrapper: Pin<&mut SkPathWrapper>,
) -> bool {
    font_ref.0.as_ref().map_or(false, |f| {
        let mut cx = Context::new();
        let mut scaler = cx.new_scaler().size(Size::new(size)).build(f);
        // TODO(drott): Can we define an impl Pen for SkPathWrapper directly?
        // https://github.com/dtolnay/cxx/issues/1175 might have hints on that.
        let mut pen_dump = PenDump {
            path_wrapper: path_wrapper,
        };
        assert!(scaler.has_outlines());
        let result = scaler.outline(GlyphId::new(glyph_id), &mut pen_dump);
        match result {
            Ok(_) => true,
            _ => false,
        }
    })
}

fn advance_width_or_zero(font_ref: &BridgeFontRef, size: f32, glyph_id: u16) -> f32 {
    font_ref.0.as_ref().map_or(0.0, |f| {
        GlyphMetrics::new(f, Size::new(size), NormalizedCoords::default())
            .advance_width(GlyphId::new(glyph_id))
            .unwrap_or(0.0)
    })
}

fn make_font_ref_internal<'a>(font_data: &'a [u8], index: u32) -> Result<FontRef<'a>, ReadError> {
    match FileRef::new(font_data) {
        Ok(file_ref) => match file_ref {
            FileRef::Font(font_ref) => Ok(font_ref),
            FileRef::Collection(collection) => collection.get(index),
        },
        Err(e) => Err(e),
    }
}

fn make_font_ref<'a>(font_data: &'a [u8], index: u32) -> Box<BridgeFontRef<'a>> {
    let result = make_font_ref_internal(font_data, index);
    let return_value = match result {
        Ok(font_ref) => Box::new(BridgeFontRef(Some(font_ref))),
        _ => Box::new(BridgeFontRef(None)),
    };
    println!(
        "gid: {} ok: {:?}",
        lookup_glyph_or_zero(&*return_value, 'a' as u32),
        return_value.0.is_some()
    );
    return_value
}

struct BridgeFontRef<'a>(Option<FontRef<'a>>);

#[cxx::bridge(namespace = "fontations_ffi")]
mod ffi {
    extern "Rust" {

        type BridgeFontRef<'a>;
        // TODO(drott): Can the unsafe be removed? Used as a factory method on
        // the C++ side. Skia side would need to ensure that the data stays
        // alive for the duration of lifetime of BridgeFontRef.
        unsafe fn make_font_ref<'a>(font_data: &'a [u8], index: u32) -> Box<BridgeFontRef<'a>>;

        fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16;
        fn get_path(
            font_ref: &BridgeFontRef,
            glyph_id: u16,
            size: f32,
            path_wrapper: Pin<&mut SkPathWrapper>,
        ) -> bool;
        fn advance_width_or_zero(font_ref: &BridgeFontRef, size: f32, glyph_id: u16) -> f32;
    }

    unsafe extern "C++" {

        include!("src/ports/fontations/src/skpath_bridge.h");
        type SkPathWrapper;

        #[allow(dead_code)]
        fn move_to(self: Pin<&mut SkPathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn line_to(self: Pin<&mut SkPathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn quad_to(self: Pin<&mut SkPathWrapper>, cx0: f32, cy0: f32, x: f32, y: f32);
        #[allow(dead_code)]
        fn curve_to(
            self: Pin<&mut SkPathWrapper>,
            cx0: f32,
            cy0: f32,
            cx1: f32,
            cy1: f32,
            x: f32,
            y: f32,
        );
        #[allow(dead_code)]
        fn close(self: Pin<&mut SkPathWrapper>);
        #[allow(dead_code)]
        fn dump(self: Pin<&mut SkPathWrapper>);
    }
}
