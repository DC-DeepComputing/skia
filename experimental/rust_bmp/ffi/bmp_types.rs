// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Type definitions for BMP decoder
//! 
//! Note: ColorType and AlphaType are defined in FFI.rs to avoid conflicts
//! This module contains only BMP-specific internal types.

use super::bmp_constants::*;

/// BMP compression types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum CompressionType {
    None,
    Rle8,
    Rle4,
    Jpeg,
    Png,
    BitFields,
}

impl CompressionType {
    pub fn is_valid(value: u32) -> bool {
        value <= MAX_COMPRESSION_TYPE
    }
}

/// BMP header information combining file and info headers (matches Chromium's approach)
#[derive(Debug, Clone)]
pub struct BmpHeaderInfo {
    #[allow(dead_code)] // Chromium uses size for header validation
    pub size: u32,
    pub width: u32,
    pub height: u32,
    pub bits_per_pixel: u16,
    pub compression: CompressionType,
    pub colors_used: u32,
    pub data_offset: u32,
    pub file_size: u32,
    pub icc_profile_data: Option<u32>,
    pub icc_profile_size: Option<u32>,
}

/// Color masks for BITFIELDS compression
#[derive(Debug, Clone, Copy)]
pub struct ColorMasks {
    pub red_mask: u32,
    pub green_mask: u32,
    pub blue_mask: u32,
    pub alpha_mask: u32,
}