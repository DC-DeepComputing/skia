// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Custom Rust BMP Decoder Library
//!
//! This library provides a comprehensive BMP decoder implementation with two options:
//! 1. Custom implementation: Hand-optimized decoder with advanced features
//! 2. image.rs integration: Uses the popular Rust 'image' crate for BMP decoding
//! 
//! The library is designed to be used both standalone and through FFI bindings for C++ integration.

#[allow(non_snake_case)]
pub mod FFI;

// Custom implementation modules (enabled when NOT using image crate)
#[cfg(not(feature = "use_image_crate"))]
pub mod decoder;
#[cfg(not(feature = "use_image_crate"))]
pub mod header;
#[cfg(not(feature = "use_image_crate"))]
pub mod error;
#[cfg(not(feature = "use_image_crate"))]
pub mod compression;
#[cfg(not(feature = "use_image_crate"))]
pub mod optimization;
#[cfg(not(feature = "use_image_crate"))]
pub mod ico;
#[cfg(not(feature = "use_image_crate"))]
pub mod types;

// Re-export main types for convenience (custom implementation)
#[cfg(not(feature = "use_image_crate"))]
pub use decoder::{BmpDecoder, DecoderConfig, DecoderResult};
#[cfg(not(feature = "use_image_crate"))]
pub use header::{BmpHeaderType, HeaderInfo, HeaderDetector};
#[cfg(not(feature = "use_image_crate"))]
pub use error::{BmpError, ErrorSeverity, ErrorContext, ErrorRecoverySystem};
#[cfg(not(feature = "use_image_crate"))]
pub use compression::{CompressionType, RLEDecoder, CompressionHandler};
#[cfg(not(feature = "use_image_crate"))]
pub use optimization::{CacheManager, SimdProcessor, StreamingDecoder};
#[cfg(not(feature = "use_image_crate"))]
pub use ico::{IcoHandler, TransparencyMode};
#[cfg(not(feature = "use_image_crate"))]
pub use types::{ColorType, ImageInfo, PixelFormat};

/// Version information for the BMP decoder library
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

/// Library features and capabilities
#[derive(Debug, Clone)]
pub struct LibraryInfo {
    pub version: &'static str,
    pub features: Vec<&'static str>,
    pub supported_formats: Vec<&'static str>,
}

impl Default for LibraryInfo {
    fn default() -> Self {
        Self {
            version: VERSION,
            features: vec![
                "header_detection",
                "error_recovery", 
                "compression_support",
                "ico_support",
                "simd_optimization",
                "streaming_decode",
                "memory_optimization",
                "progressive_decode",
            ],
            supported_formats: vec![
                "BMP (all variants)",
                "ICO embedded BMP",
                "OS/2 bitmap",
                "RLE4/RLE8 compression",
                "Huffman compression",
                "JPEG/PNG delegation",
            ],
        }
    }
}

/// Get library information and capabilities
pub fn get_library_info() -> LibraryInfo {
    LibraryInfo::default()
}

/// Initialize the BMP decoder library with default configuration
pub fn init() -> Result<(), BmpError> {
    // Initialize any global state if needed
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_library_info() {
        let info = get_library_info();
        assert_eq!(info.version, VERSION);
        assert!(!info.features.is_empty());
        assert!(!info.supported_formats.is_empty());
    }

    #[test]
    fn test_init() {
        assert!(init().is_ok());
    }
}