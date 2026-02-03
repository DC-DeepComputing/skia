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

pub enum BitmapPixelData {
    PngData(Vec<u8>),
    BgraData(Vec<u8>),
}

struct SbixGlyph<'a> {
    glyph_data: GlyphData<'a>,
    ppem: u16,
}

/// Decodes bitpacked bitmap data to 8-bit alpha values.
/// Supports 1, 2, 4, and 8 bit depths.
/// Reference: https://github.com/dfrg/swash/blob/56ea551ac49e51580c466f03d6c75252a8c8e700/src/strike.rs#L402
fn decode_bitpacked_to_alpha(
    src: &[u8],
    width: u32,
    height: u32,
    bit_depth: u8,
    is_byte_aligned: bool,
) -> Vec<u8> {
    let w = width as usize;
    let h = height as usize;
    let mut dst = vec![0u8; w * h];

    match bit_depth {
        1 => {
            if is_byte_aligned {
                let row_bytes = (w + 7) / 8;
                for y in 0..h {
                    let row = &src[y * row_bytes..];
                    for x in 0..w {
                        dst[y * w + x] = ((row[x >> 3] >> (7 - (x & 7))) & 1) * 255;
                    }
                }
            } else {
                for i in 0..(w * h) {
                    dst[i] = ((src[i >> 3] >> (7 - (i & 7))) & 1) * 255;
                }
            }
        }
        2 => {
            if is_byte_aligned {
                let row_bytes = (w * 2 + 7) / 8;
                for y in 0..h {
                    let row = &src[y * row_bytes..];
                    for x in 0..w {
                        dst[y * w + x] = ((row[x >> 2] >> (6 - (x & 3) * 2)) & 3) * 85;
                    }
                }
            } else {
                for i in 0..(w * h) {
                    dst[i] = ((src[i >> 2] >> (6 - (i & 3) * 2)) & 3) * 85;
                }
            }
        }
        4 => {
            if is_byte_aligned {
                let row_bytes = (w * 4 + 7) / 8;
                for y in 0..h {
                    let row = &src[y * row_bytes..];
                    for x in 0..w {
                        dst[y * w + x] = ((row[x >> 1] >> (4 - (x & 1) * 4)) & 15) * 17;
                    }
                }
            } else {
                for i in 0..(w * h) {
                    dst[i] = ((src[i >> 1] >> (4 - (i & 1) * 4)) & 15) * 17;
                }
            }
        }
        8 => {
            let size = (w * h).min(src.len());
            dst[..size].copy_from_slice(&src[..size]);
        }
        32 => {
            let size = (w * h).min(src.len());
            dst[..size].copy_from_slice(&src[..size]);
        }
        _ => {}
    }

    dst
}

/// Converts 8-bit alpha data to BGRA format (white with alpha).
fn alpha_to_bgra(alpha: &[u8], width: u32, height: u32) -> Vec<u8> {
    let w = width as usize;
    let h = height as usize;
    let mut bgra = Vec::with_capacity(w * h * 4);

    for y in 0..h {
        for x in 0..w {
            let a = alpha.get(y * w + x).copied().unwrap_or(0);
            bgra.push(255); // B
            bgra.push(255); // G
            bgra.push(255); // R
            bgra.push(a);   // A
        }
    }

    bgra
}

#[derive(Default)]
pub struct BridgeBitmapGlyph {
    pub data: Option<BitmapPixelData>,
    pub metrics: FfiBitmapMetrics,
    pub bit_depth: u8,
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

struct CblcGlyphInfo<'a> {
    bitmap_data: BitmapData<'a>,
    ppem_x: u8,
    ppem_y: u8,
    bit_depth: u8,
}

fn cblc_glyph_with_info<'a>(
    font_ref: &'a FontRef,
    glyph_id: GlyphId,
    font_size: Option<f32>,
) -> Option<CblcGlyphInfo<'a>> {
    let cblc = font_ref.cblc().ok()?;
    let cbdt = font_ref.cbdt().ok()?;

    let strikes = &cblc.bitmap_sizes();
    let best_strike = font_size
        .and_then(|size| best_strike_size(strikes.iter(), size))
        .or(strikes.get(0))?;

    let location = best_strike.location(cblc.offset_data(), glyph_id).ok()?;

    Some(CblcGlyphInfo {
        bitmap_data: cbdt.data(&location).ok()?,
        ppem_x: best_strike.ppem_x,
        ppem_y: best_strike.ppem_y,
        bit_depth: best_strike.bit_depth,
    })
}

pub fn has_bitmap_glyph(font_ref: &BridgeFontRef, glyph_id: u16) -> bool {
    font_ref
        .with_font(|font| {
            let glyph_id = GlyphId::from(glyph_id);
            let has_sbix = sbix_glyph(font, glyph_id, None).is_some();
            let has_cblc = cblc_glyph_with_info(font, glyph_id, None).is_some();
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

pub unsafe fn bitmap_glyph(
    font_ref: &BridgeFontRef,
    glyph_id: u16,
    font_size: f32,
) -> Box<BridgeBitmapGlyph> {
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
                    data: Some(BitmapPixelData::PngData(sbix_glyph.glyph_data.data().to_vec())),
                    metrics: FfiBitmapMetrics {
                        bearing_x: glyf_left_side_bearing,
                        bearing_y: glyf_bb.y_min as f32,
                        inner_bearing_x: sbix_glyph.glyph_data.origin_offset_x() as f32,
                        inner_bearing_y: sbix_glyph.glyph_data.origin_offset_y() as f32,
                        ppem_x: sbix_glyph.ppem as f32,
                        ppem_y: sbix_glyph.ppem as f32,
                        placement_origin_bottom_left: true,
                        advance: f32::NAN,
                        width: 0,
                        height: 0,
                    },
                    bit_depth: 32,
                }));
            } else if let Some(cblc_glyph) = cblc_glyph_with_info(font, glyph_id, Some(font_size)) {
                // Extract width, height, and metrics
                let (bearing_x, bearing_y, advance, width, height) = match cblc_glyph.bitmap_data.metrics {
                    BitmapMetrics::Small(small_metrics) => (
                        small_metrics.bearing_x() as f32,
                        small_metrics.bearing_y() as f32,
                        small_metrics.advance as f32,
                        small_metrics.width as u32,
                        small_metrics.height as u32,
                    ),
                    BitmapMetrics::Big(big_metrics) => (
                        big_metrics.hori_bearing_x() as f32,
                        big_metrics.hori_bearing_y() as f32,
                        big_metrics.hori_advance as f32,
                        big_metrics.width as u32,
                        big_metrics.height as u32,
                    ),
                };

                let base_metrics = FfiBitmapMetrics {
                    bearing_x: 0.0,
                    bearing_y: 0.0,
                    inner_bearing_x: bearing_x,
                    inner_bearing_y: bearing_y,
                    ppem_x: cblc_glyph.ppem_x as f32,
                    ppem_y: cblc_glyph.ppem_y as f32,
                    placement_origin_bottom_left: false,
                    advance,
                    width,
                    height,
                };

                // Handle different bitmap data formats
                // Reference: https://github.com/dfrg/swash/blob/56ea551ac49e51580c466f03d6c75252a8c8e700/src/strike.rs#L402
                match cblc_glyph.bitmap_data.content {
                    BitmapContent::Data(BitmapDataFormat::Png, png_buffer) => {
                        return Some(Box::new(BridgeBitmapGlyph {
                            data: Some(BitmapPixelData::PngData(png_buffer.to_vec())),
                            metrics: base_metrics,
                            bit_depth: 32,
                        }));
                    }
                    BitmapContent::Data(BitmapDataFormat::ByteAligned, raw_data) => {
                        // Byte-aligned raw bitmap (CBDT Formats 1, 6)
                        let alpha = decode_bitpacked_to_alpha(
                            raw_data,
                            width,
                            height,
                            cblc_glyph.bit_depth,
                            true,
                        );
                        let bgra = alpha_to_bgra(&alpha, width, height);
                        return Some(Box::new(BridgeBitmapGlyph {
                            data: Some(BitmapPixelData::BgraData(bgra)),
                            metrics: base_metrics,
                            bit_depth: cblc_glyph.bit_depth,
                        }));
                    }
                    BitmapContent::Data(BitmapDataFormat::BitAligned, raw_data) => {
                        // Bit-aligned raw bitmap (CBDT Formats 2, 5, 7)
                        let alpha = decode_bitpacked_to_alpha(
                            raw_data,
                            width,
                            height,
                            cblc_glyph.bit_depth,
                            false,
                        );
                        let bgra = alpha_to_bgra(&alpha, width, height);
                        return Some(Box::new(BridgeBitmapGlyph {
                            data: Some(BitmapPixelData::BgraData(bgra)),
                            metrics: base_metrics,
                            bit_depth: cblc_glyph.bit_depth,
                        }));
                    }
                    BitmapContent::Composite(_) => {
                        return None;
                    }
                }
            }
            None
        })
        .unwrap_or_default()
}

pub unsafe fn png_data(bitmap_glyph: &BridgeBitmapGlyph) -> &[u8] {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::PngData(data)) => data.as_slice(),
        _ => &[],
    }
}

/// Returns the decoded BGRA pixel data if available.
pub unsafe fn bgra_data(bitmap_glyph: &BridgeBitmapGlyph) -> &[u8] {
    match &bitmap_glyph.data {
        Some(BitmapPixelData::BgraData(data)) => data.as_slice(),
        _ => &[],
    }
}

/// Returns true if the bitmap glyph contains PNG data.
pub fn is_png_bitmap(bitmap_glyph: &BridgeBitmapGlyph) -> bool {
    matches!(&bitmap_glyph.data, Some(BitmapPixelData::PngData(_)))
}

/// Returns true if the bitmap glyph contains decoded BGRA data.
pub fn is_bgra_bitmap(bitmap_glyph: &BridgeBitmapGlyph) -> bool {
    matches!(&bitmap_glyph.data, Some(BitmapPixelData::BgraData(_)))
}

/// Returns true if the bitmap glyph is an alpha mask (non-PNG, bit_depth <= 8).
pub fn is_alpha_mask_bitmap(bitmap_glyph: &BridgeBitmapGlyph) -> bool {
    is_bgra_bitmap(bitmap_glyph) && bitmap_glyph.bit_depth <= 8
}

pub unsafe fn bitmap_metrics(bitmap_glyph: &BridgeBitmapGlyph) -> &FfiBitmapMetrics {
    &bitmap_glyph.metrics
}
