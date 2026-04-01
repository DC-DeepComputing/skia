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

/// Decode CBDT bitmap data into 8-bit alpha.
/// When `byte_aligned` is true (formats 1, 6), each row is padded to a byte boundary.
/// When false (formats 2, 5, 7), pixels are tightly packed with no row padding.
fn decode_bitmap(
    src: &[u8],
    width: u32,
    height: u32,
    bit_depth: u8,
    byte_aligned: bool,
) -> Option<Vec<u8>> {
    let w = width as usize;
    let h = height as usize;
    let total_pixels = w.checked_mul(h)?;
    let bits = bit_depth as usize;

    let mut dst = vec![0u8; total_pixels];
    if byte_aligned {
        let mut dst_idx = 0;
        match bit_depth {
            1 => {
                for row in src.chunks(((w * bits) + 7) / 8) {
                    for x in 0..w {
                        dst[dst_idx] = (row[x >> 3] >> (!x & 7) & 1) * 255;
                        dst_idx += 1;
                    }
                }
            }
            2 => {
                for row in src.chunks(((w * bits) + 7) / 8) {
                    for x in 0..w {
                        dst[dst_idx] = (row[x >> 2] >> (!(x * 2) & 2) & 3) * 85;
                        dst_idx += 1;
                    }
                }
            }
            4 => {
                for row in src.chunks(((w * bits) + 7) / 8) {
                    for x in 0..w {
                        dst[dst_idx] = (row[x >> 1] >> (!(x * 4) & 4) & 15) * 17;
                        dst_idx += 1;
                    }
                }
            }
            8 => {
                for row in src.chunks(((w * bits) + 7) / 8) {
                    dst[dst_idx..dst_idx + w].copy_from_slice(&row[..w]);
                    dst_idx += w;
                }
            }
            _ => return None,
        }
    } else {
        match bit_depth {
            1 => {
                for x in 0..total_pixels {
                    dst[x] = (src[x >> 3] >> (!x & 7) & 1) * 255;
                }
            }
            2 => {
                for x in 0..total_pixels {
                    dst[x] = (src[x >> 2] >> (!(x * 2) & 2) & 3) * 85;
                }
            }
            4 => {
                for x in 0..total_pixels {
                    dst[x] = (src[x >> 1] >> (!(x * 4) & 4) & 15) * 17;
                }
            }
            8 => {
                dst.copy_from_slice(&src[..total_pixels]);
            }
            _ => return None,
        }
    }
    Some(dst)
}

fn bitmap_dimensions(
    metrics: &BitmapMetrics,
    font_ref: &FontRef,
    font_size: Option<f32>,
) -> (u32, u32, u8) {
    let (w, h) = match metrics {
        BitmapMetrics::Small(m) => (m.width as u32, m.height as u32),
        BitmapMetrics::Big(m) => (m.width as u32, m.height as u32),
    };

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

    (w, h, cblc_bit_depth)
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

                let metrics = FfiBitmapMetrics {
                    bearing_x: 0.0,
                    bearing_y: 0.0,
                    inner_bearing_x: bearing_x,
                    inner_bearing_y: bearing_y,
                    ppem_x: cblc_glyph.ppem_x as f32,
                    ppem_y: cblc_glyph.ppem_y as f32,
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
                        );
                        let decoded = match format {
                            BitmapDataFormat::ByteAligned =>
                                decode_bitmap(raw_data, w, h, bit_depth, true),
                            BitmapDataFormat::BitAligned =>
                                decode_bitmap(raw_data, w, h, bit_depth, false),
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
