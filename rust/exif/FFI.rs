// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! EXIF metadata parser FFI bindings.
//!
//! Provides C++ bindings for parsing EXIF metadata using the `exif` crate
//! (kamadak-exif). All parsing happens in Rust for memory safety, then the
//! result is passed to the C++ side for conversion to SkExif::Metadata.

#[cxx::bridge(namespace = "rust_exif")]
mod ffi {
    /// Parsed EXIF metadata fields.
    ///
    /// CXX does not support Rust's `Option<T>`, so each field is represented
    /// as a `has_*` boolean paired with a value field. The value is only valid
    /// when `has_*` is true.
    struct ExifMetadata {
        /// Image orientation (SkEncodedOrigin), values 1-8 per EXIF tag 0x0112.
        has_origin: bool,
        origin: u32,

        /// Apple HDR headroom (computed from MakerNote tags 33 and 48).
        /// See: https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
        has_hdr_headroom: bool,
        hdr_headroom: f32,

        /// Resolution unit (EXIF tag 0x0128): 2 = inch, 3 = centimeter.
        has_resolution_unit: bool,
        resolution_unit: u16,

        /// X resolution (EXIF tag 0x011a).
        has_x_resolution: bool,
        x_resolution: f32,

        /// Y resolution (EXIF tag 0x011b).
        has_y_resolution: bool,
        y_resolution: f32,

        /// Image width in pixels (EXIF tag 0xa002, can be SHORT or LONG).
        has_pixel_x_dimension: bool,
        pixel_x_dimension: u32,

        /// Image height in pixels (EXIF tag 0xa003, can be SHORT or LONG).
        has_pixel_y_dimension: bool,
        pixel_y_dimension: u32,
    }

    extern "Rust" {
        /// Parses raw EXIF data (TIFF-formatted bytes starting at the byte-order
        /// marker) from `data`. Returns `true` on any successful parse attempt
        /// and populates `out` with all found fields. Returns `false` only if
        /// the data cannot be parsed at all (e.g., missing/invalid TIFF header).
        /// Individual missing fields are indicated by their `has_*` flags.
        fn parse_exif(data: &[u8], out: &mut ExifMetadata) -> bool;
    }
}

/// Apple MakerNote signature (14 bytes).
const APPLE_MAKER_NOTE_SIG: &[u8] = b"Apple iOS\x00\x00\x01MM";

/// Computes Apple HDR headroom from raw MakerNote bytes.
///
/// The Apple MakerNote is a big-endian TIFF-like IFD structure that starts
/// immediately after a 14-byte signature. IFD tags 33 and 48 are signed
/// rationals whose values determine the HDR headroom (in stops above SDR).
///
/// The formula is documented at:
/// https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
fn parse_apple_hdr_headroom(data: &[u8]) -> Option<f32> {
    if !data.starts_with(APPLE_MAKER_NOTE_SIG) {
        return None;
    }

    let ifd_start = APPLE_MAKER_NOTE_SIG.len();

    // IFD entry count: big-endian u16 immediately after the signature.
    let count_bytes = data.get(ifd_start..ifd_start + 2)?;
    let entry_count = u16::from_be_bytes([count_bytes[0], count_bytes[1]]) as usize;

    let entries_start = ifd_start + 2;
    const ENTRY_SIZE: usize = 12;

    let mut maker33: Option<f32> = None;
    let mut maker48: Option<f32> = None;

    for i in 0..entry_count {
        let off = entries_start + i * ENTRY_SIZE;
        let entry = match data.get(off..off + ENTRY_SIZE) {
            Some(e) => e,
            None => break, // Truncated; stop gracefully.
        };

        let tag = u16::from_be_bytes([entry[0], entry[1]]);
        let type_ = u16::from_be_bytes([entry[2], entry[3]]);
        let count = u32::from_be_bytes([entry[4], entry[5], entry[6], entry[7]]);

        // Only process tags 33 and 48, type SRATIONAL (10), count 1.
        if (tag == 33 || tag == 48) && type_ == 10 && count == 1 {
            // The 4-byte value field is an offset from start of MakerNote data
            // to the 8-byte SRATIONAL value (4-byte numerator + 4-byte denominator).
            let value_offset =
                u32::from_be_bytes([entry[8], entry[9], entry[10], entry[11]]) as usize;
            let srational = match data.get(value_offset..value_offset + 8) {
                Some(s) => s,
                None => continue,
            };
            let numer =
                i32::from_be_bytes([srational[0], srational[1], srational[2], srational[3]]);
            let denom =
                i32::from_be_bytes([srational[4], srational[5], srational[6], srational[7]]);
            let value = if denom != 0 {
                numer as f32 / denom as f32
            } else {
                0.0
            };

            match tag {
                33 => {
                    if maker33.is_none() {
                        maker33 = Some(value);
                    }
                }
                48 => {
                    if maker48.is_none() {
                        maker48 = Some(value);
                    }
                }
                _ => {}
            }
        }
    }

    // maker33 is required; maker48 defaults to 0 when absent (matches C++ logic).
    let maker33 = maker33?;
    let maker48 = maker48.unwrap_or(0.0);

    // Compute HDR headroom in stops and convert to a linear multiplier.
    let stops = if maker33 < 1.0 {
        if maker48 <= 0.01 {
            -20.0 * maker48 + 1.8
        } else {
            -0.101 * maker48 + 1.601
        }
    } else {
        if maker48 <= 0.01 {
            -70.0 * maker48 + 3.0
        } else {
            -0.303 * maker48 + 2.303
        }
    };

    Some(f32::powf(2.0, stops.max(0.0)))
}

/// Parses raw EXIF bytes into `out`. Returns `false` if the data cannot be
/// parsed at all; returns `true` and sets appropriate `has_*` flags otherwise.
pub fn parse_exif(data: &[u8], out: &mut ffi::ExifMetadata) -> bool {
    use exif::{Reader, Tag, Value};

    *out = ffi::ExifMetadata {
        has_origin: false,
        origin: 0,
        has_hdr_headroom: false,
        hdr_headroom: 0.0,
        has_resolution_unit: false,
        resolution_unit: 0,
        has_x_resolution: false,
        x_resolution: 0.0,
        has_y_resolution: false,
        y_resolution: 0.0,
        has_pixel_x_dimension: false,
        pixel_x_dimension: 0,
        has_pixel_y_dimension: false,
        pixel_y_dimension: 0,
    };

    if data.is_empty() {
        return false;
    }

    let exif = match Reader::new().read_raw(data.to_vec()) {
        Ok(e) => e,
        Err(_) => return false,
    };

    // Helper: find the first field with the given raw tag number, searched
    // across ALL IFDs and contexts. Some files (e.g., Olympus cameras) place
    // TIFF-level tags like Orientation inside the ExifIFD sub-IFD. In
    // kamadak-exif, `Tag` is context-sensitive: the same numeric tag in a
    // different IFD context (ExifIFD vs TIFF IFD) becomes a different `Tag`
    // value. Matching by raw number (tag.number()) replicates the C++ parser's
    // behavior of finding a tag wherever it appears. Prefer primary fields by
    // skipping thumbnail IFD (In(1)) when a primary occurrence exists.
    let find_field_by_num = |tag_num: u16| -> Option<&exif::Field> {
        exif.fields()
            .find(|f| f.tag.number() == tag_num && f.ifd_num != exif::In::THUMBNAIL)
            .or_else(|| exif.fields().find(|f| f.tag.number() == tag_num))
    };

    // Orientation (tag 0x0112): SHORT, values 1-8.
    if let Some(field) = find_field_by_num(Tag::Orientation.number()) {
        if let Some(v) = field.value.get_uint(0) {
            if v >= 1 && v <= 8 {
                out.has_origin = true;
                out.origin = v;
            }
        }
    }

    // ResolutionUnit (tag 0x0128): SHORT.
    if let Some(field) = find_field_by_num(Tag::ResolutionUnit.number()) {
        if let Some(v) = field.value.get_uint(0) {
            if v <= u16::MAX as u32 {
                out.has_resolution_unit = true;
                out.resolution_unit = v as u16;
            }
        }
    }

    // XResolution (tag 0x011a): RATIONAL (unsigned). Zero denominator → 0.0.
    if let Some(field) = find_field_by_num(Tag::XResolution.number()) {
        if let Value::Rational(ref vals) = field.value {
            if let Some(r) = vals.first() {
                out.has_x_resolution = true;
                out.x_resolution = if r.denom != 0 {
                    r.num as f32 / r.denom as f32
                } else {
                    0.0
                };
            }
        }
    }

    // YResolution (tag 0x011b): RATIONAL (unsigned). Zero denominator → 0.0.
    if let Some(field) = find_field_by_num(Tag::YResolution.number()) {
        if let Value::Rational(ref vals) = field.value {
            if let Some(r) = vals.first() {
                out.has_y_resolution = true;
                out.y_resolution = if r.denom != 0 {
                    r.num as f32 / r.denom as f32
                } else {
                    0.0
                };
            }
        }
    }

    // PixelXDimension (tag 0xa002): SHORT or LONG (as per Exif 2.3 spec).
    if let Some(field) = find_field_by_num(Tag::PixelXDimension.number()) {
        if let Some(v) = field.value.get_uint(0) {
            out.has_pixel_x_dimension = true;
            out.pixel_x_dimension = v;
        }
    }

    // PixelYDimension (tag 0xa003): SHORT or LONG.
    if let Some(field) = find_field_by_num(Tag::PixelYDimension.number()) {
        if let Some(v) = field.value.get_uint(0) {
            out.has_pixel_y_dimension = true;
            out.pixel_y_dimension = v;
        }
    }

    // MakerNote (tag 0x927c): UNDEFINED bytes. Parse Apple HDR headroom.
    if let Some(field) = find_field_by_num(Tag::MakerNote.number()) {
        if let Value::Undefined(ref bytes, _) = field.value {
            if let Some(headroom) = parse_apple_hdr_headroom(bytes) {
                out.has_hdr_headroom = true;
                out.hdr_headroom = headroom;
            }
        }
    }

    true
}

#[cfg(test)]
mod tests {
    use super::*;

    fn empty_metadata() -> ffi::ExifMetadata {
        ffi::ExifMetadata {
            has_origin: false,
            origin: 0,
            has_hdr_headroom: false,
            hdr_headroom: 0.0,
            has_resolution_unit: false,
            resolution_unit: 0,
            has_x_resolution: false,
            x_resolution: 0.0,
            has_y_resolution: false,
            y_resolution: 0.0,
            has_pixel_x_dimension: false,
            pixel_x_dimension: 0,
            has_pixel_y_dimension: false,
            pixel_y_dimension: 0,
        }
    }

    #[test]
    fn test_empty_data_returns_false() {
        let mut out = empty_metadata();
        assert!(!parse_exif(&[], &mut out));
    }

    #[test]
    fn test_invalid_data_returns_false() {
        let mut out = empty_metadata();
        // Random bytes that do not form a valid TIFF header.
        assert!(!parse_exif(b"not valid exif data at all", &mut out));
        // Partial TIFF header (just byte-order mark, no IFD offset).
        assert!(!parse_exif(b"MM\x00\x2a", &mut out));
    }

    #[test]
    fn test_apple_hdr_headroom_sig_mismatch() {
        assert!(parse_apple_hdr_headroom(b"").is_none());
        assert!(parse_apple_hdr_headroom(b"Not Apple MakerNote data").is_none());
    }

    #[test]
    fn test_apple_hdr_headroom_truncated() {
        // Signature present but no IFD data following it.
        assert!(parse_apple_hdr_headroom(APPLE_MAKER_NOTE_SIG).is_none());
    }
}
