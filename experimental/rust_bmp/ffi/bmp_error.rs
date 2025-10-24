// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Error types for BMP decoder

use thiserror::Error;

/// Core BMP decoder error types
#[derive(Debug, Error)]
pub enum BmpError {
    #[error("Insufficient data available")]
    InsufficientData,
    
    #[error("Invalid data: {0}")]
    InvalidData(String),
    
    #[error("Unsupported feature: {0}")]
    UnsupportedFeature(String),
    
    #[error("Invalid file type signature")]
    InvalidFileType,
    
    #[error("Header too small: got {got}, expected at least {expected}")]
    UnsupportedHeaderSize { got: u32, expected: u32 },
    
    #[error("Invalid header data")]
    InvalidHeader,
        
    #[error("JPEG decoding failed: {0}")]
    JpegDecodingFailed(String),
    
    #[error("PNG decoding failed: {0}")]
    PngDecodingFailed(String),
    
    #[error("Invalid data offset")]
    InvalidDataOffset,
    
    #[error("RLE decoding failed")]
    RleDecodingFailed,
}
