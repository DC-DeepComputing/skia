// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

use cxx;
use font_types::GlyphId;
use read_fonts::{FileRef, FileRef::Font};
use skrifa::MetadataProvider;
use std::{u8};

fn lookup_glyph(font_data: &[u8], codepoint: u32) -> Option<GlyphId> {
    let file_ref = FileRef::new(font_data).unwrap();
    let Font(font) = file_ref else { return None; };
    return font.charmap().map(codepoint);
}

fn lookup_glyph_or_zero(font_data: &[u8], codepoint: u32) -> u16 {
    let lookup_result = lookup_glyph(font_data, codepoint);
    lookup_result.map_or_else(|| 0, |id| id.to_u16())
}

#[cxx::bridge(namespace = "fontations_ffi")]
mod ffi {
    extern "Rust" {
        fn lookup_glyph_or_zero(font_data: &[u8], codepoint: u32) -> u16;
    }
}
