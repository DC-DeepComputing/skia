// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
use core::pin::Pin;
use cxx;
use font_types::GlyphId;
use read_fonts::{FileRef, FileRef::Font};
use skrifa::scale::Context;
use skrifa::{MetadataProvider, Size};
use std::u8;

fn lookup_glyph(font_data: &[u8], codepoint: u32) -> Option<GlyphId> {
    let file_ref = FileRef::new(font_data).unwrap();
    let Font(font) = file_ref else { return None; };
    return font.charmap().map(codepoint);
}

fn lookup_glyph_or_zero(font_data: &[u8], codepoint: u32) -> u16 {
    let lookup_result = lookup_glyph(font_data, codepoint);
    lookup_result.map_or_else(|| 0, |id| id.to_u16())
}

use crate::ffi::SkPathWrapper;

fn get_path(
    font_data: &[u8],
    glyph_id: u16,
    size: f32,
    path_wrapper: Pin<&mut SkPathWrapper>,
) -> bool {
    let file_ref = FileRef::new(font_data).unwrap();
    let Font(font) = file_ref else { return false; };
    let mut cx = Context::new();
    let mut scaler = cx.new_scaler().size(Size::new(size)).build(&font);
    assert!(scaler.has_outlines());
    let result = scaler.outline(GlyphId::new(glyph_id), path_wrapper.get_mut());
    match (result) {
        Ok() => True,
        _ => False,
    }
}

#[cxx::bridge(namespace = "fontations_ffi")]
mod ffi {
    extern "Rust" {
        fn lookup_glyph_or_zero(font_data: &[u8], codepoint: u32) -> u16;
        fn get_path(
            font_data: &[u8],
            glyph_id: u16,
            size: f32,
            path_wrapper: Pin<&mut SkPathWrapper>,
        ) -> bool;
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
