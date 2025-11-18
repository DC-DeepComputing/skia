// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Error type definitions for BMP decoder FFI
//!
//! This module provides error mapping from image-rs errors to FFI-friendly error types.

use thiserror::Error;

/// BMP decoder error types for FFI
#[derive(Debug, Error)]
pub enum BmpError {
    #[error("Insufficient data available")]
    InsufficientData,

    #[error("Invalid data: {0}")]
    InvalidData(String),

    #[error("Unsupported feature: {0}")]
    UnsupportedFeature(String),

    #[error("Invalid header data")]
    InvalidHeader,

    #[error("I/O error: {0}")]
    IoError(String),

    #[error("Format error: {0}")]
    FormatError(String),
}

/// Convert image-rs ImageError to BmpError
impl From<image::ImageError> for BmpError {
    fn from(err: image::ImageError) -> Self {
        match err {
            image::ImageError::Decoding(_) => BmpError::FormatError(err.to_string()),
            image::ImageError::Encoding(_) => BmpError::FormatError(err.to_string()),
            image::ImageError::IoError(e) => BmpError::IoError(e.to_string()),
            image::ImageError::Limits(_) => BmpError::UnsupportedFeature(err.to_string()),
            image::ImageError::Unsupported(_) => BmpError::UnsupportedFeature(err.to_string()),
            _ => BmpError::InvalidData(err.to_string()),
        }
    }
}

/// Convert std::io::Error to BmpError
impl From<std::io::Error> for BmpError {
    fn from(err: std::io::Error) -> Self {
        use std::io::ErrorKind;
        match err.kind() {
            ErrorKind::UnexpectedEof => BmpError::InsufficientData,
            _ => BmpError::IoError(err.to_string()),
        }
    }
}
