// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! C++ bindings for ICC profile parsing using moxcms.
//!
//! This crate provides a Rust-based ICC profile parser that can be used by
//! Skia codecs (SkPngRustCodec, SkBmpRustCodec) to parse ICC color profiles
//! embedded in image files. The parsed profile data is returned to C++ where
//! Skia's skcms handles the actual color space transformations.

//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

use moxcms::{ColorProfile, DataColorSpace};

#[cxx::bridge(namespace = "rust_icc")]
mod ffi {
    /// Result of ICC profile parsing.
    ///
    /// If parsing succeeds, `success` is true and the profile data fields are
    /// populated. The raw `profile_bytes` are included so skcms can use them
    /// for color transformations.
    struct IccProfileResult {
        /// Whether parsing succeeded.
        success: bool,

        /// Error message if parsing failed.
        error_message: String,

        /// Data color space (e.g., RGB, CMYK, Gray).
        data_color_space: ColorSpace,

        /// Profile Connection Space (PCS), typically XYZ or Lab.
        connection_space: ColorSpace,

        /// Rendering intent from the ICC header.
        rendering_intent: RenderingIntent,

        /// The raw ICC profile bytes for use by skcms.
        profile_bytes: Vec<u8>,
    }

    /// Color space types supported by ICC profiles.
    /// These match the color spaces recognized by skcms (see skcms_public.h).
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum ColorSpace {
        Unknown = 0,
        XYZ = 1,
        Lab = 2,
        Luv = 3,
        YCbCr = 4,
        Yxy = 5,
        RGB = 6,
        Gray = 7,
        HSV = 8,
        HLS = 9,
        CMYK = 10,
        CMY = 11,
        Color2 = 12,
        Color3 = 13,
        Color4 = 14,
        Color5 = 15,
        Color6 = 16,
        Color7 = 17,
        Color8 = 18,
        Color9 = 19,
        Color10 = 20,
        Color11 = 21,
        Color12 = 22,
        Color13 = 23,
        Color14 = 24,
        Color15 = 25,
    }

    /// ICC rendering intent.
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum RenderingIntent {
        Perceptual = 0,
        RelativeColorimetric = 1,
        Saturation = 2,
        AbsoluteColorimetric = 3,
    }

    extern "Rust" {
        /// Parse an ICC profile from raw bytes.
        fn parse_icc_profile(data: &[u8]) -> IccProfileResult;
    }
}

/// Parse an ICC profile from raw bytes.
///
/// Returns an `IccProfileResult` indicating success/failure and containing
/// the parsed profile data. The original bytes are included in `profile_bytes`
/// for use by skcms.
pub fn parse_icc_profile(data: &[u8]) -> ffi::IccProfileResult {
    use ffi::{ColorSpace, IccProfileResult, RenderingIntent};

    const ICC_SIGNATURE: &[u8; 4] = b"acsp";

    // Basic validation
    if data.is_empty() {
        return IccProfileResult {
            success: false,
            error_message: "Empty ICC profile data".to_string(),
            data_color_space: ColorSpace::Unknown,
            connection_space: ColorSpace::Unknown,
            rendering_intent: RenderingIntent::Perceptual,
            profile_bytes: Vec::new(),
        };
    }

    if data.len() < 40 {
        return IccProfileResult {
            success: false,
            error_message: "ICC profile too short".to_string(),
            data_color_space: ColorSpace::Unknown,
            connection_space: ColorSpace::Unknown,
            rendering_intent: RenderingIntent::Perceptual,
            profile_bytes: Vec::new(),
        };
    }

    // Check for ICC signature "acsp" at byte 36
    if &data[36..40] != ICC_SIGNATURE {
        return IccProfileResult {
            success: false,
            error_message: "Invalid ICC profile signature".to_string(),
            data_color_space: ColorSpace::Unknown,
            connection_space: ColorSpace::Unknown,
            rendering_intent: RenderingIntent::Perceptual,
            profile_bytes: Vec::new(),
        };
    }

    // Parse with moxcms
    match ColorProfile::new_from_slice(data) {
        Ok(profile) => {
            // Convert moxcms DataColorSpace to our ColorSpace enum
            let data_color_space = match profile.color_space {
                DataColorSpace::Xyz => ColorSpace::XYZ,
                DataColorSpace::Lab => ColorSpace::Lab,
                DataColorSpace::Luv => ColorSpace::Luv,
                DataColorSpace::YCbr => ColorSpace::YCbCr,
                DataColorSpace::Yxy => ColorSpace::Yxy,
                DataColorSpace::Rgb => ColorSpace::RGB,
                DataColorSpace::Gray => ColorSpace::Gray,
                DataColorSpace::Hsv => ColorSpace::HSV,
                DataColorSpace::Hls => ColorSpace::HLS,
                DataColorSpace::Cmyk => ColorSpace::CMYK,
                DataColorSpace::Cmy => ColorSpace::CMY,
                // Multi-channel color spaces recognized by skcms
                DataColorSpace::Color2 => ColorSpace::Color2,
                DataColorSpace::Color3 => ColorSpace::Color3,
                DataColorSpace::Color4 => ColorSpace::Color4,
                DataColorSpace::Color5 => ColorSpace::Color5,
                DataColorSpace::Color6 => ColorSpace::Color6,
                DataColorSpace::Color7 => ColorSpace::Color7,
                DataColorSpace::Color8 => ColorSpace::Color8,
                DataColorSpace::Color9 => ColorSpace::Color9,
                DataColorSpace::Color10 => ColorSpace::Color10,
                DataColorSpace::Color11 => ColorSpace::Color11,
                DataColorSpace::Color12 => ColorSpace::Color12,
                DataColorSpace::Color13 => ColorSpace::Color13,
                DataColorSpace::Color14 => ColorSpace::Color14,
                DataColorSpace::Color15 => ColorSpace::Color15,
            };

            // Convert PCS (Profile Connection Space)
            // PCS is typically only XYZ or Lab per ICC spec, but we exhaustively match
            // to catch any moxcms changes at compile time.
            let connection_space = match profile.pcs {
                DataColorSpace::Xyz => ColorSpace::XYZ,
                DataColorSpace::Lab => ColorSpace::Lab,
                // The rest of values are technically invalid as PCS
                _ => ColorSpace::Unknown,
            };
            // Convert rendering intent (stored as u32, values 0-3)
            let rendering_intent = match profile.rendering_intent as u32 {
                0 => RenderingIntent::Perceptual,
                1 => RenderingIntent::RelativeColorimetric,
                2 => RenderingIntent::Saturation,
                3 => RenderingIntent::AbsoluteColorimetric,
                _ => RenderingIntent::Perceptual, // Default to perceptual
            };

            IccProfileResult {
                success: true,
                error_message: String::new(),
                data_color_space,
                connection_space,
                rendering_intent,
                profile_bytes: data.to_vec(),
            }
        }
        Err(e) => IccProfileResult {
            success: false,
            error_message: format!("Failed to parse ICC profile: {:?}", e),
            data_color_space: ColorSpace::Unknown,
            connection_space: ColorSpace::Unknown,
            rendering_intent: RenderingIntent::Perceptual,
            profile_bytes: Vec::new(),
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_empty_profile() {
        let result = parse_icc_profile(&[]);
        assert!(!result.success);
        assert!(result.error_message.contains("Empty"));
    }

    #[test]
    fn test_too_short() {
        let result = parse_icc_profile(&[0; 39]);
        assert!(!result.success);
        assert!(result.error_message.contains("too short"));
    }

    #[test]
    fn test_invalid_signature() {
        let mut data = vec![0u8; 128];
        // Put wrong signature at offset 36
        data[36..40].copy_from_slice(b"badd");

        let result = parse_icc_profile(&data);
        assert!(!result.success);
        assert!(result.error_message.contains("signature"));
    }

    #[test]
    fn test_valid_signature_invalid_profile() {
        let mut data = vec![0u8; 128];
        // Put correct ICC signature "acsp" at offset 36
        data[36..40].copy_from_slice(b"acsp");

        let result = parse_icc_profile(&data);
        // This will fail moxcms parsing since it's not a real ICC profile
        assert!(!result.success);
        assert!(result.error_message.contains("parse"));
    }

    #[test]
    fn test_valid_profile() {
        // Sample from AdobeRGB1998.icc - a real 560-byte ICC profile
        let data: [u8; 560] = [
            0x00, 0x00, 0x02, 0x30, 0x41, 0x44, 0x42, 0x45, 0x02, 0x10, 0x00, 0x00, 0x6d, 0x6e,
            0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59, 0x5a, 0x20, 0x07, 0xd0, 0x00, 0x08,
            0x00, 0x0b, 0x00, 0x13, 0x00, 0x33, 0x00, 0x3b, 0x61, 0x63, 0x73, 0x70, 0x41, 0x50,
            0x50, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x6f, 0x6e, 0x65, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd3, 0x2d, 0x41, 0x44, 0x42, 0x45,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x00, 0xfc,
            0x00, 0x00, 0x00, 0x32, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x30, 0x00, 0x00,
            0x00, 0x6b, 0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x01, 0x9c, 0x00, 0x00, 0x00, 0x14,
            0x62, 0x6b, 0x70, 0x74, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x00, 0x14, 0x72, 0x54,
            0x52, 0x43, 0x00, 0x00, 0x01, 0xc4, 0x00, 0x00, 0x00, 0x0e, 0x67, 0x54, 0x52, 0x43,
            0x00, 0x00, 0x01, 0xd4, 0x00, 0x00, 0x00, 0x0e, 0x62, 0x54, 0x52, 0x43, 0x00, 0x00,
            0x01, 0xe4, 0x00, 0x00, 0x00, 0x0e, 0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xf4,
            0x00, 0x00, 0x00, 0x14, 0x67, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00,
            0x00, 0x14, 0x62, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x02, 0x1c, 0x00, 0x00, 0x00, 0x14,
            0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69,
            0x67, 0x68, 0x74, 0x20, 0x32, 0x30, 0x30, 0x30, 0x20, 0x41, 0x64, 0x6f, 0x62, 0x65,
            0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x73, 0x20, 0x49, 0x6e, 0x63, 0x6f, 0x72,
            0x70, 0x6f, 0x72, 0x61, 0x74, 0x65, 0x64, 0x00, 0x00, 0x00, 0x64, 0x65, 0x73, 0x63,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x41, 0x64, 0x6f, 0x62, 0x65, 0x20,
            0x52, 0x47, 0x42, 0x20, 0x28, 0x31, 0x39, 0x39, 0x38, 0x29, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xf3, 0x51, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x16, 0xcc, 0x58, 0x59,
            0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x63, 0x75, 0x72, 0x76, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x33, 0x00, 0x00, 0x58, 0x59, 0x5a, 0x20,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x18, 0x00, 0x00, 0x4f, 0xa5, 0x00, 0x00,
            0x04, 0xfc, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x8d,
            0x00, 0x00, 0xa0, 0x2c, 0x00, 0x00, 0x0f, 0x95, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x26, 0x31, 0x00, 0x00, 0x10, 0x2f, 0x00, 0x00, 0xbe, 0x9c,
        ];

        let result = parse_icc_profile(&data);
        assert!(
            result.success,
            "Failed to parse valid ICC profile: {}",
            result.error_message
        );
        assert_eq!(result.data_color_space, ffi::ColorSpace::RGB);
        assert_eq!(result.connection_space, ffi::ColorSpace::XYZ);
        assert_eq!(result.profile_bytes.len(), 560);
    }
}
