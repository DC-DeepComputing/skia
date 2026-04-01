// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.

use read_fonts::{
    tables::{
        bitmap::{BitmapContent, BitmapData, BitmapDataFormat, BitmapMetrics, BitmapSize},
        sbix::{GlyphData, Strike},
    },
    FontRef, TableProvider,
};

use font_types::{BoundingBox, GlyphId};
use skrifa::{
    instance::{LocationRef, Size},
    metrics::GlyphMetrics,
};

use crate::{ffi::BitmapMetrics as FfiBitmapMetrics, BridgeFontRef};

pub enum BitmapPixelData<'a> {
    PngData(&'a [u8]),
    DecodedAlpha {
        data: Vec<u8>,
        width: u32,
        height: u32,
    },
}

struct CblcGlyph<'a> {
    bitmap_data: BitmapData<'a>,
    ppem_x: u8,
    ppem_y: u8,
}

struct SbixGlyph<'a> {
    glyph_data: GlyphData<'a>,
    ppem: u16,
}

#[derive(Default)]
pub struct BridgeBitmapGlyph<'a> {
    pub data: Option<BitmapPixelData<'a>>,
    pub metrics: FfiBitmapMetrics,
}

trait StrikeSizeRetrievable {
    fn strike_size(&self) -> f32;
}

impl StrikeSizeRetrievable for &BitmapSize {
    fn strike_size(&self) -> f32 {
        self.ppem_y() as f32
    }
}

impl StrikeSizeRetrievable for Strike<'_> {
    fn strike_size(&self) -> f32 {
        self.ppem() as f32
    }
}

// Find the nearest larger strike size, or if no larger one is available, the nearest smaller.
fn best_strike_size<T>(strikes: impl Iterator<Item = T>, font_size: f32) -> Option<T>
where
    T: StrikeSizeRetrievable,
{
    // After a bigger strike size is found, the order of strike sizes smaller
    // than the requested font size does not matter anymore. A new strike size
    // is only an improvement if it gets closer to the requested font size (and
    // is smaller than the current best, but bigger than font size). And vice
    // versa: As long as we have found only smaller ones so far, only any strike
    // size matters that is bigger than the current best.
    strikes.reduce(|best, entry| {
        let entry_size = entry.strike_size();
        if (entry_size >= font_size && entry_size < best.strike_size())
            || (best.strike_size() < font_size && entry_size > best.strike_size())
        {
            entry
        } else {
            best
        }
    })
}

fn sbix_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<SbixGlyph<'a>> {
    let sbix = font_ref.sbix().ok()?;
    let mut strikes = sbix.strikes().iter().filter_map(|strike| strike.ok());

    let best_strike = match font_size {
        Some(size) => best_strike_size(strikes, size),
        _ => strikes.next(),
    }?;

    Some(SbixGlyph {
        ppem: best_strike.ppem(),
        glyph_data: best_strike.glyph_data(glyph_id).ok()??,
    })
}

fn cblc_glyph<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<CblcGlyph<'a>> {
    let cblc = font_ref.cblc().ok()?;
    let cbdt = font_ref.cbdt().ok()?;

    let strikes = &cblc.bitmap_sizes();
    let best_strike = font_size
        .and_then(|size| best_strike_size(strikes.iter(), size))
        .or(strikes.get(0))?;

    let location = best_strike.location(cblc.offset_data(), glyph_id).ok()?;

    Some(CblcGlyph {
        bitmap_data: cbdt.data(&location).ok()?,
        ppem_x: best_strike.ppem_x,
        ppem_y: best_strike.ppem_y,
    })
}

/// Decode byte-aligned CBDT bitmap data (formats 1, 6) into 8-bit alpha.
/// Each row is padded to a byte boundary.
fn decode_byte_aligned(src: &[u8], width: u32, height: u32, bit_depth: u8) -> Option<Vec<u8>> {
    let w = width as usize;
    let h = height as usize;
    if w == 0 || h == 0 || bit_depth == 0 {
        return None;
    }
    let row_bytes = (w * bit_depth as usize + 7) / 8;
    let expected_len = row_bytes.checked_mul(h)?;
    if src.len() < expected_len {
        return None;
    }
    let total_pixels = w.checked_mul(h)?;
    let mut dst = vec![0u8; total_pixels];
    let mut dst_idx = 0;
    for row in src[..expected_len].chunks(row_bytes) {
        for x in 0..w {
            if let Some(pixel) = unpack_pixel_from_row(row, x, bit_depth) {
                dst[dst_idx] = pixel;
            }
            dst_idx += 1;
        }
    }
    Some(dst)
}

/// Decode bit-aligned CBDT bitmap data (formats 2, 5, 7) into 8-bit alpha.
/// Pixels are tightly packed with no row padding.
fn decode_bit_aligned(src: &[u8], width: u32, height: u32, bit_depth: u8) -> Option<Vec<u8>> {
    let w = width as usize;
    let h = height as usize;
    if w == 0 || h == 0 || bit_depth == 0 {
        return None;
    }
    let total_pixels = w.checked_mul(h)?;
    let total_bits = total_pixels.checked_mul(bit_depth as usize)?;
    let expected_bytes = (total_bits + 7) / 8;
    if src.len() < expected_bytes {
        return None;
    }
    let mut dst = vec![0u8; total_pixels];
    match bit_depth {
        1 => {
            for x in 0..total_pixels {
                let byte_idx = x >> 3;
                if byte_idx < src.len() {
                    dst[x] = ((src[byte_idx] >> (7 - (x & 7))) & 1) * 255;
                }
            }
        }
        2 => {
            for x in 0..total_pixels {
                let byte_idx = x >> 2;
                if byte_idx < src.len() {
                    let shift = 6 - ((x & 3) * 2);
                    dst[x] = ((src[byte_idx] >> shift) & 3) * 85;
                }
            }
        }
        4 => {
            for x in 0..total_pixels {
                let byte_idx = x >> 1;
                if byte_idx < src.len() {
                    let shift = if (x & 1) == 0 { 4 } else { 0 };
                    dst[x] = ((src[byte_idx] >> shift) & 0xF) * 17;
                }
            }
        }
        8 => {
            if src.len() >= total_pixels {
                dst.copy_from_slice(&src[..total_pixels]);
            }
        }
        _ => return None,
    }
    Some(dst)
}

/// Unpack a single pixel value from a byte-aligned row.
fn unpack_pixel_from_row(row: &[u8], x: usize, bit_depth: u8) -> Option<u8> {
    match bit_depth {
        1 => {
            let byte_idx = x >> 3;
            if byte_idx >= row.len() {
                return None;
            }
            Some(((row[byte_idx] >> (7 - (x & 7))) & 1) * 255)
        }
        2 => {
            let byte_idx = x >> 2;
            if byte_idx >= row.len() {
                return None;
            }
            let shift = 6 - ((x & 3) * 2);
            Some(((row[byte_idx] >> shift) & 3) * 85)
        }
        4 => {
            let byte_idx = x >> 1;
            if byte_idx >= row.len() {
                return None;
            }
            let shift = if (x & 1) == 0 { 4 } else { 0 };
            Some(((row[byte_idx] >> shift) & 0xF) * 17)
        }
        8 => {
            if x >= row.len() {
                return None;
            }
            Some(row[x])
        }
        _ => None,
    }
}

/// Extract width, height, and bit_depth from bitmap metrics and strike info.
/// When bitDepth is 0 in CBLC, infer from actual data size.
fn bitmap_dimensions(
    metrics: &BitmapMetrics,
    font_ref: &FontRef,
    font_size: Option<f32>,
    raw_data_len: usize,
) -> (u32, u32, u8) {
    let (w, h) = match metrics {
        BitmapMetrics::Small(m) => (m.width as u32, m.height as u32),
        BitmapMetrics::Big(m) => (m.width as u32, m.height as u32),
    };
    // bit_depth comes from the BitmapSize record in CBLC.
    let cblc_bit_depth = font_ref
        .cblc()
        .ok()
        .and_then(|cblc| {
            let strikes = cblc.bitmap_sizes();
            let best = font_size
                .and_then(|size| best_strike_size(strikes.iter(), size))
                .or(strikes.get(0))?;
            Some(best.bit_depth)
        })
        .unwrap_or(0);

    let bit_depth = if cblc_bit_depth > 0 {
        cblc_bit_depth
    } else if w > 0 && h > 0 {
        // Infer bit depth from data size when CBLC bitDepth is 0.
        let pixels = (w * h) as usize;
        let bits_per_pixel = (raw_data_len * 8) / pixels;
        match bits_per_pixel {
            0 => 1,
            1 => 1,
            2 => 2,
            3..=4 => 4,
            5..=8 => 8,
            _ => 8, // 32bpp or larger — treat as 8bpp alpha
        }
    } else {
        1
    };
    (w, h, bit_depth)
}

pub fn has_bitmap_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    font_ref
        .with_font(|font| {
            let glyph_id = GlyphId::from(glyph_id);
            let has_sbix = sbix_glyph(font, glyph_id, None).is_some();
            let has_cblc = cblc_glyph(font, glyph_id, None).is_some();
            Some(has_sbix || has_cblc)
        })
        .unwrap_or_default()
}

fn glyf_bounds(font_ref: &FontRef, glyph_id: GlyphId) -> Option<BoundingBox<i16>> {
    let glyf_table = font_ref.glyf().ok()?;
    let glyph = font_ref
        .loca(None)
        .ok()?
        .get_glyf(glyph_id, &glyf_table)
        .ok()??;
    Some(BoundingBox {
        x_min: glyph.x_min(),
        y_min: glyph.y_min(),
        x_max: glyph.x_max(),
        y_max: glyph.y_max(),
    })
}

pub unsafe fn bitmap_glyph<'a>(
    font_ref: &'a BridgeFontRef,
    glyph_id: u16,
    font_size: f32,
) -> Box<BridgeBitmapGlyph<'a>> {
    let glyph_id = GlyphId::from(glyph_id);
    font_ref
        .with_font(|font| {
            if let Some(sbix_glyph) = sbix_glyph(font, glyph_id, Some(font_size)) {
                // https://learn.microsoft.com/en-us/typography/opentype/spec/sbix
                // "If there is a glyph contour, the glyph design space
                // origin for the graphic is placed at the lower left corner
                // of the glyph bounding box (xMin, yMin)."
                let glyf_bb = glyf_bounds(font, glyph_id).unwrap_or_default();
                let glyf_left_side_bearing =
                    GlyphMetrics::new(font, Size::unscaled(), LocationRef::default())
                        .left_side_bearing(glyph_id)
                        .unwrap_or_default();

                return Some(Box::new(BridgeBitmapGlyph {
                    data: Some(BitmapPixelData::PngData(sbix_glyph.glyph_data.data())),
                    metrics: FfiBitmapMetrics {
                        bearing_x: glyf_left_side_bearing,
                        bearing_y: glyf_bb.y_min as f32,
                        inner_bearing_x: sbix_glyph.glyph_data.origin_offset_x() as f32,
                        inner_bearing_y: sbix_glyph.glyph_data.origin_offset_y() as f32,
                        ppem_x: sbix_glyph.ppem as f32,
                        ppem_y: sbix_glyph.ppem as f32,
                        placement_origin_bottom_left: true,
                        advance: f32::NAN,
                    },
                }));
            } else if let Some(cblc_glyph) = cblc_glyph(font, glyph_id, Some(font_size)) {
                let (bearing_x, bearing_y, advance) = match cblc_glyph.bitmap_data.metrics {
                    BitmapMetrics::Small(small_metrics) => (
                        small_metrics.bearing_x() as f32,
                        small_metrics.bearing_y() as f32,
                        small_metrics.advance as f32,
                    ),
                    BitmapMetrics::Big(big_metrics) => (
                        big_metrics.hori_bearing_x() as f32,
                        big_metrics.hori_bearing_y() as f32,
                        big_metrics.hori_advance as f32,
                    ),
                };
                // Use requested font size as ppem when CBLC ppem is 0 or missing.
                // Some fonts (e.g., ARIB broadcast fonts) have ppem=0 in CBLC.
                let effective_ppem_x = if cblc_glyph.ppem_x > 0 {
                    cblc_glyph.ppem_x as f32
                } else {
                    font_size
                };
                let effective_ppem_y = if cblc_glyph.ppem_y > 0 {
                    cblc_glyph.ppem_y as f32
                } else {
                    font_size
                };
                let metrics = FfiBitmapMetrics {
                    bearing_x: 0.0,
                    bearing_y: 0.0,
                    inner_bearing_x: bearing_x,
                    inner_bearing_y: bearing_y,
                    ppem_x: effective_ppem_x,
                    ppem_y: effective_ppem_y,
                    placement_origin_bottom_left: false,
                    advance: advance,
                };
                match cblc_glyph.bitmap_data.content {
                    BitmapContent::Data(BitmapDataFormat::Png, png_buffer) => {
                        return Some(Box::new(BridgeBitmapGlyph {
                            data: Some(BitmapPixelData::PngData(png_buffer)),
                            metrics,
                        }));
                    }
                    BitmapContent::Data(format, raw_data) => {
                        let (w, h, bit_depth) = bitmap_dimensions(
                            &cblc_glyph.bitmap_data.metrics,
                            font,
                            Some(font_size),
                            raw_data.len(),
                        );
                        let decoded = match format {
                            BitmapDataFormat::ByteAligned =>
                                decode_byte_aligned(raw_data, w, h, bit_depth),
                            BitmapDataFormat::BitAligned =>
                                decode_bit_aligned(raw_data, w, h, bit_depth),
                            _ => None,
                        };
                        if let Some(alpha_data) = decoded {
                            return Some(Box::new(BridgeBitmapGlyph {
                                data: Some(BitmapPixelData::DecodedAlpha {
                                    data: alpha_data,
                                    width: w,
                                    height: h,
                                }),
                                metrics,
                            }));
                        }
                    }
                    _ => {} // Composite or other unsupported
                }
            }
            None
        })
        .unwrap_or_default()
}

pub unsafe fn png_data<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a [u8] {
    match bitmap_glyph.data {
        Some(BitmapPixelData::PngData(glyph_data)) => glyph_data,
        _ => &[],
    }
}

pub unsafe fn bitmap_metrics<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a FfiBitmapMetrics {
    &bitmap_glyph.metrics
}

pub fn has_decoded_alpha(bitmap_glyph: &BridgeBitmapGlyph) -> bool {
    matches!(&bitmap_glyph.data, Some(BitmapPixelData::DecodedAlpha { .. }))
}

pub unsafe fn decoded_alpha_data<'a>(bitmap_glyph: &'a BridgeBitmapGlyph) -> &'a [u8] {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::DecodedAlpha { data, .. }) => data.as_slice(),
        _ => &[],
    }
}

pub fn decoded_alpha_width(bitmap_glyph: &BridgeBitmapGlyph) -> u32 {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::DecodedAlpha { width, .. }) => *width,
        _ => 0,
    }
}

pub fn decoded_alpha_height(bitmap_glyph: &BridgeBitmapGlyph) -> u32 {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::DecodedAlpha { height, .. }) => *height,
        _ => 0,
    }
}
