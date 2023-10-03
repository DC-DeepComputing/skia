// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
use cxx;
use font_types::{GlyphId, Pen};
use read_fonts::{FileRef, FontRef, ReadError, TableProvider};
use skrifa::{
    instance::{Location, Size},
    metrics::{GlyphMetrics, Metrics},
    scale::Context,
    setting::VariationSetting,
    string::{LocalizedStrings, StringId},
    MetadataProvider, Tag,
};
use std::pin::Pin;

use crate::ffi::AxisWrapper;
use crate::ffi::BridgeScalerMetrics;
use crate::ffi::PathWrapper;

fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16 {
    font_ref
        .with_font(|f| Some(f.charmap().map(codepoint)?.to_u16()))
        .unwrap_or_default()
}

fn num_glyphs(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .with_font(|f| Some(f.maxp().ok()?.num_glyphs()))
        .unwrap_or_default()
}

struct PathWrapperPen<'a> {
    path_wrapper: Pin<&'a mut ffi::PathWrapper>,
}

// We need to wrap ffi::PathWrapper in PathWrapperPen and forward the path
// recording calls to the path wrapper as we can't define trait implementations
// inside the cxx::bridge section.
impl<'a> Pen for PathWrapperPen<'a> {
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
    coords: &BridgeNormalizedCoords,
    path_wrapper: Pin<&mut PathWrapper>,
    scaler_metrics: &mut BridgeScalerMetrics,
) -> bool {
    font_ref
        .with_font(|f| {
            let mut cx = Context::new();
            let mut scaler = cx
                .new_scaler()
                .size(Size::new(size))
                .normalized_coords(coords.normalized_coords.into_iter())
                .build(f);
            let mut pen_dump = PathWrapperPen {
                path_wrapper: path_wrapper,
            };
            match scaler.outline(GlyphId::new(glyph_id), &mut pen_dump) {
                Err(_) => None,
                Ok(metrics) => {
                    scaler_metrics.has_overlaps = metrics.has_overlaps;
                    Some(())
                }
            }
        })
        .is_some()
}

fn advance_width_or_zero(
    font_ref: &BridgeFontRef,
    size: f32,
    coords: &BridgeNormalizedCoords,
    glyph_id: u16,
) -> f32 {
    font_ref
        .with_font(|f| {
            GlyphMetrics::new(f, Size::new(size), coords.normalized_coords.coords())
                .advance_width(GlyphId::new(glyph_id))
        })
        .unwrap_or_default()
}

fn units_per_em_or_zero(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .with_font(|f| Some(f.head().ok()?.units_per_em()))
        .unwrap_or_default()
}

fn convert_metrics(skrifa_metrics: &Metrics) -> ffi::Metrics {
    ffi::Metrics {
        top: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.y_max),
        bottom: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.y_min),
        x_min: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.x_min),
        x_max: skrifa_metrics.bounds.map_or_else(|| 0.0, |b| b.x_max),
        ascent: skrifa_metrics.ascent,
        descent: skrifa_metrics.descent,
        leading: skrifa_metrics.leading,
        avg_char_width: skrifa_metrics.average_width.unwrap_or_else(|| 0.0),
        max_char_width: skrifa_metrics.max_width.unwrap_or_else(|| 0.0),
        x_height: skrifa_metrics.x_height.unwrap_or_else(|| 0.0),
        cap_height: skrifa_metrics.cap_height.unwrap_or_else(|| 0.0),
    }
}

fn get_skia_metrics(
    font_ref: &BridgeFontRef,
    size: f32,
    coords: &BridgeNormalizedCoords,
) -> ffi::Metrics {
    font_ref
        .with_font(|f| {
            let fontations_metrics =
                Metrics::new(f, Size::new(size), coords.normalized_coords.coords());
            Some(convert_metrics(&fontations_metrics))
        })
        .unwrap_or_default()
}

fn get_localized_strings<'a>(font_ref: &'a BridgeFontRef<'a>) -> Box<BridgeLocalizedStrings<'a>> {
    Box::new(BridgeLocalizedStrings {
        localized_strings: font_ref
            .with_font(|f| Some(f.localized_strings(StringId::FAMILY_NAME)))
            .unwrap_or_default(),
    })
}

use crate::ffi::BridgeLocalizedName;

fn localized_name_next(
    bridge_localized_strings: &mut BridgeLocalizedStrings,
    out_localized_name: &mut BridgeLocalizedName,
) -> bool {
    match bridge_localized_strings.localized_strings.next() {
        Some(localized_string) => {
            out_localized_name.string = localized_string.to_string();
            out_localized_name.language = localized_string
                .language()
                .map(|l| l.to_string())
                .unwrap_or_default();
            true
        }
        _ => false,
    }
}

fn english_or_first_font_name(font_ref: &BridgeFontRef, name_id: StringId) -> Option<String> {
    font_ref.with_font(|f| {
        f.localized_strings(name_id)
            .english_or_first()
            .map(|localized_string| localized_string.to_string())
    })
}

fn family_name(font_ref: &BridgeFontRef) -> String {
    english_or_first_font_name(font_ref, StringId::FAMILY_NAME).unwrap_or_default()
}

fn postscript_name(font_ref: &BridgeFontRef, out_string: &mut String) -> bool {
    let postscript_name = english_or_first_font_name(font_ref, StringId::POSTSCRIPT_NAME);
    match postscript_name {
        Some(name) => {
            *out_string = name;
            true
        }
        _ => false,
    }
}

use crate::ffi::{ColrV0GlyphLayerRange, ColrV0Layer};

fn num_palettes(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .with_font(|f| {
            let cpal = f.cpal().ok()?;
            Some(cpal.num_palettes())
        })
        .unwrap_or_default()
}

fn num_palette_entries(font_ref: &BridgeFontRef) -> u16 {
    font_ref
        .with_font(|f| {
            let cpal = f.cpal().ok()?;
            Some(cpal.num_palette_entries())
        })
        .unwrap_or_default()
}

fn palette_colors(font_ref: &BridgeFontRef, palette_index: u16, colors: &mut [u32]) -> bool {
    let num_entries: usize = num_palette_entries(font_ref).into();
    if colors.len() == 0 || num_entries != colors.len() {
        return false;
    }

    font_ref
        .with_font(|f| {
            let cpal_table = f.cpal().ok()?;

            let start_index =
                usize::from(cpal_table.color_record_indices()[usize::from(palette_index)].get());

            cpal_table
                .color_records_array()
                .and_then(|color_records| match color_records {
                    Ok(records) => {
                        for (&(mut target_color), color) in colors
                            .iter()
                            .zip(records[start_index..start_index + num_entries].iter())
                        {
                            target_color = u32::from_be_bytes([
                                color.alpha(),
                                color.red(),
                                color.green(),
                                color.blue(),
                            ]);
                        }
                        Some(true)
                    }
                    _ => None,
                })
        })
        .unwrap_or_default()
}

fn colrv0_layer_range(font_ref: &BridgeFontRef, glyph_id: u16) -> ColrV0GlyphLayerRange {
    font_ref
        .with_font(|f| {
            let layer_range = f.colr().ok()?.v0_base_glyph(GlyphId::new(glyph_id)).ok()?;
            layer_range.map(|r| ColrV0GlyphLayerRange {
                has_v0_layers: true,
                start_index: r.start,
                end_index: r.end,
            })
        })
        .unwrap_or_default()
}

fn colrv0_glyph_layer(
    font_ref: &BridgeFontRef,
    layer_index: usize,
    out_layer: &mut ColrV0Layer,
) -> bool {
    match font_ref.with_font(|f| f.colr().ok()?.v0_layer(layer_index).ok()) {
        Some(t) => {
            out_layer.glyph_id = t.0.to_u16();
            out_layer.palette_index = t.1;
            true
        }
        _ => false,
    }
}

/// Implements the behavior expected for `SkTypeface::getTableData`, compare
/// documentation for this method and the FreeType implementation in Skia.
/// * If the target data array is empty, do not copy any data into it, but
///   return the size of the table.
/// * If the target data buffer is shorted than from offset to the end of the
///   table, truncate the data.
/// * If offset is longer than the table's length, return 0.
fn table_data(font_ref: &BridgeFontRef, tag: u32, offset: usize, data: &mut [u8]) -> usize {
    let table_data = font_ref
        .with_font(|f| f.table_data(Tag::from_be_bytes(tag.to_be_bytes())))
        .unwrap_or_default();
    let table_data = table_data.as_ref();
    // Remaining table data size measured from offset to end, or 0 if offset is
    // too large.
    let mut to_copy_length = table_data.len().saturating_sub(offset);
    match data.len() {
        0 => to_copy_length,
        _ => {
            to_copy_length = to_copy_length.min(data.len());
            let table_offset_data = table_data
                .get(offset..offset + to_copy_length)
                .unwrap_or_default();
            data.get_mut(..table_offset_data.len())
                .map_or(0, |data_slice| {
                    data_slice.copy_from_slice(table_offset_data);
                    data_slice.len()
                })
        }
    }
}

fn table_tags(font_ref: &BridgeFontRef, tags: &mut [u32]) -> u16 {
    return font_ref
        .with_font(|f| {
            let table_directory = &f.table_directory;
            let table_tags_iter = table_directory
                .table_records()
                .iter()
                .map(|table| u32::from_be_bytes(table.tag.get().into_bytes()));
            tags.iter_mut()
                .zip(table_tags_iter)
                .for_each(|(out_tag, table_tag)| *out_tag = table_tag);
            Some(table_directory.num_tables())
        })
        .unwrap_or_default();
}

fn variation_position(
    coords: &BridgeNormalizedCoords,
    coordinates: &mut [SkiaDesignCoordinate],
) -> isize {
    if coordinates.len() > 0 {
        if coords.filtered_user_coords.len() > coordinates.len() {
            return -1;
        }
        let skia_design_coordinates =
            coords
                .filtered_user_coords
                .iter()
                .map(|setting| SkiaDesignCoordinate {
                    axis: u32::from_be_bytes(setting.selector.into_bytes()),
                    value: setting.value,
                });
        for (i, coord) in skia_design_coordinates.enumerate() {
            coordinates[i] = coord;
        }
    }
    coords.filtered_user_coords.len().try_into().unwrap()
}

fn populate_axes(font_ref: &BridgeFontRef, mut axis_wrapper: Pin<&mut AxisWrapper>) -> isize {
    font_ref
        .with_font(|f| {
            let axes = f.axes();
            // Populate incoming allocated SkFontParameters::Variation::Axis[] only when a
            // buffer is passed.
            if axis_wrapper.as_ref().size() > 0 {
                for (i, axis) in axes.iter().enumerate() {
                    if !axis_wrapper.as_mut().populate_axis(
                        i,
                        u32::from_be_bytes(axis.tag().into_bytes()),
                        axis.min_value(),
                        axis.default_value(),
                        axis.max_value(),
                        axis.is_hidden(),
                    ) {
                        return None;
                    }
                }
            }
            isize::try_from(axes.len()).ok()
        })
        .unwrap_or(-1)
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
    Box::new(BridgeFontRef(make_font_ref_internal(font_data, index).ok()))
}

fn font_ref_is_valid(bridge_font_ref: &BridgeFontRef) -> bool {
    bridge_font_ref.0.is_some()
}

use crate::ffi::SkiaDesignCoordinate;

fn resolve_into_normalized_coords(
    font_ref: &BridgeFontRef,
    design_coords: &[SkiaDesignCoordinate],
) -> Box<BridgeNormalizedCoords> {
    let variation_tuples = design_coords
        .into_iter()
        .map(|coord| (Tag::from_be_bytes(coord.axis.to_be_bytes()), coord.value));
    let bridge_normalized_coords = font_ref
        .with_font(|f| {
            Some(BridgeNormalizedCoords {
                filtered_user_coords: f.axes().filter(variation_tuples.clone()).collect(),
                normalized_coords: f.axes().location(variation_tuples),
            })
        })
        .unwrap_or_default();
    Box::new(bridge_normalized_coords)
}

struct BridgeFontRef<'a>(Option<FontRef<'a>>);

impl<'a> BridgeFontRef<'a> {
    fn with_font<T>(&'a self, f: impl FnOnce(&'a FontRef) -> Option<T>) -> Option<T> {
        f(self.0.as_ref()?)
    }
}

#[derive(Default)]
struct BridgeNormalizedCoords {
    normalized_coords: Location,
    filtered_user_coords: Vec<VariationSetting>,
}

struct BridgeLocalizedStrings<'a> {
    #[allow(dead_code)]
    localized_strings: LocalizedStrings<'a>,
}

#[cxx::bridge(namespace = "fontations_ffi")]
mod ffi {

    #[derive(Default)]
    struct Metrics {
        top: f32,
        ascent: f32,
        descent: f32,
        bottom: f32,
        leading: f32,
        avg_char_width: f32,
        max_char_width: f32,
        x_min: f32,
        x_max: f32,
        x_height: f32,
        cap_height: f32,
    }

    struct BridgeLocalizedName {
        string: String,
        language: String,
    }

    struct SkiaDesignCoordinate {
        axis: u32,
        value: f32,
    }

    struct BridgeScalerMetrics {
        has_overlaps: bool,
    }

    #[derive(Default)]
    /// Information on whether COLRv0 glyph coverage exists for a
    /// certain glyph and if yes, which layers need to be drawn.
    struct ColrV0GlyphLayerRange {
        has_v0_layers: bool,
        start_index: usize,
        end_index: usize,
    }

    /// Representation of a COLRv0 layer consisting of glyph id
    /// and palette index.
    struct ColrV0Layer {
        glyph_id: u16,
        palette_index: u16,
    }

    extern "Rust" {

        type BridgeFontRef<'a>;
        unsafe fn make_font_ref<'a>(font_data: &'a [u8], index: u32) -> Box<BridgeFontRef<'a>>;
        // Returns whether BridgeFontRef is a valid font containing at
        // least a valid sfnt structure from which tables can be
        // accessed. This is what instantiation in make_font_ref checks
        // for. (see FontRef::new in read_fonts's lib.rs). Implemented
        // by returning whether the option is Some() and thus whether a
        // FontRef instantiation succeeded and a table directory was
        // accessible.
        fn font_ref_is_valid(bridge_font_ref: &BridgeFontRef) -> bool;

        fn lookup_glyph_or_zero(font_ref: &BridgeFontRef, codepoint: u32) -> u16;
        fn get_path(
            font_ref: &BridgeFontRef,
            glyph_id: u16,
            size: f32,
            coords: &BridgeNormalizedCoords,
            path_wrapper: Pin<&mut PathWrapper>,
            scaler_metrics: &mut BridgeScalerMetrics,
        ) -> bool;
        fn advance_width_or_zero(
            font_ref: &BridgeFontRef,
            size: f32,
            coords: &BridgeNormalizedCoords,
            glyph_id: u16,
        ) -> f32;
        fn units_per_em_or_zero(font_ref: &BridgeFontRef) -> u16;
        fn get_skia_metrics(
            font_ref: &BridgeFontRef,
            size: f32,
            coords: &BridgeNormalizedCoords,
        ) -> Metrics;
        fn num_glyphs(font_ref: &BridgeFontRef) -> u16;
        fn family_name(font_ref: &BridgeFontRef) -> String;
        fn postscript_name(font_ref: &BridgeFontRef, out_string: &mut String) -> bool;

        fn num_palettes(font_ref: &BridgeFontRef) -> u16;
        fn num_palette_entries(font_ref: &BridgeFontRef) -> u16;
        /// Copies into `colors` slice `SkColor`-compatible uint32_t ARGB color values for a specified `palette_index`.
        /// # Returns
        /// `true` on success, `false` on failure.
        fn palette_colors(font_ref: &BridgeFontRef, palette_index: u16, colors: &mut [u32])
            -> bool;

        /// Provides information on whether the specified glyph can be drawn
        /// as a COLRv0 colored glyph.
        ///
        /// # Arguments
        ///
        /// * `font_ref` - font instance as created with `make_font_ref`
        /// * `glyph_id` - glyph id of the glyph to check for COLRv0 coverage for
        ///
        /// # Returns
        ///
        /// `ColrV0GlyphLayerRange` - struct containing information on whether COLRv0
        /// coverage exists, and if yes, start and end layer index.
        fn colrv0_layer_range(font_ref: &BridgeFontRef, glyph_id: u16) -> ColrV0GlyphLayerRange;

        /// Provides access to COLRv0 layers in the COLR table.
        /// Using a layer index between start and end index
        /// retrieved with [colrv0_layer_range], use this method
        /// to retrieve glyph id and palette index of a specific layer.
        fn colrv0_glyph_layer(
            font_ref: &BridgeFontRef,
            layer_index: usize,
            out_layer: &mut ColrV0Layer,
        ) -> bool;

        fn table_data(font_ref: &BridgeFontRef, tag: u32, offset: usize, data: &mut [u8]) -> usize;
        fn table_tags(font_ref: &BridgeFontRef, tags: &mut [u32]) -> u16;
        fn variation_position(
            coords: &BridgeNormalizedCoords,
            coordinates: &mut [SkiaDesignCoordinate],
        ) -> isize;

        fn populate_axes(font_ref: &BridgeFontRef, axis_wrapper: Pin<&mut AxisWrapper>) -> isize;

        type BridgeLocalizedStrings<'a>;
        unsafe fn get_localized_strings<'a>(
            font_ref: &'a BridgeFontRef<'a>,
        ) -> Box<BridgeLocalizedStrings<'a>>;
        fn localized_name_next(
            bridge_localized_strings: &mut BridgeLocalizedStrings,
            out_localized_name: &mut BridgeLocalizedName,
        ) -> bool;

        type BridgeNormalizedCoords;
        fn resolve_into_normalized_coords(
            font_ref: &BridgeFontRef,
            design_coords: &[SkiaDesignCoordinate],
        ) -> Box<BridgeNormalizedCoords>;

    }

    unsafe extern "C++" {

        include!("src/ports/fontations/src/skpath_bridge.h");

        type PathWrapper;

        #[allow(dead_code)]
        fn move_to(self: Pin<&mut PathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn line_to(self: Pin<&mut PathWrapper>, x: f32, y: f32);
        #[allow(dead_code)]
        fn quad_to(self: Pin<&mut PathWrapper>, cx0: f32, cy0: f32, x: f32, y: f32);
        #[allow(dead_code)]
        fn curve_to(
            self: Pin<&mut PathWrapper>,
            cx0: f32,
            cy0: f32,
            cx1: f32,
            cy1: f32,
            x: f32,
            y: f32,
        );
        #[allow(dead_code)]
        fn close(self: Pin<&mut PathWrapper>);

        type AxisWrapper;

        fn populate_axis(
            self: Pin<&mut AxisWrapper>,
            i: usize,
            axis: u32,
            min: f32,
            def: f32,
            max: f32,
            hidden: bool,
        ) -> bool;
        fn size(self: Pin<&AxisWrapper>) -> usize;

    }
}

/// Test function to exercise COLRv0 functions.
/// Run using `$ bazel test -c dbg --sandbox_base=/dev/shm --with_fontations //src/ports/fontations:test_colrv0`
#[cfg(test)]
mod test {
    use super::ffi;
    #[test]
    fn test_colrv0() {
        assert_eq!("Hi Rust", "Yo Rust");
    }
}
