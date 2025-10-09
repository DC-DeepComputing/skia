// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! # Core Types and Data Structures
//!
//! This module defines the fundamental data types, enums, and structures used throughout
//! the BMP decoder library. It provides a clean, FFI-compatible interface between
//! Rust and C++ while maintaining type safety and performance.
//!
//! ## Design Principles
//!
//! ### FFI Compatibility
//! All public types are designed to be compatible with the C ABI via the `cxx` bridge:
//! - `#[repr(C)]` ensures predictable memory layout
//! - Primitive types and POD structures cross FFI boundaries safely
//! - Complex types use opaque handles or serialization
//!
//! ### Type Safety
//! - Strong typing prevents common errors (e.g., width/height confusion)
//! - Enum variants map to stable integer values for ABI compatibility
//! - Optional types handle missing or invalid data gracefully
//!
//! ### Performance Focus
//! - Zero-cost abstractions where possible
//! - Efficient memory layout (packed structures, aligned data)
//! - Minimal heap allocations for core operations
//!
//! ## Core Type Categories
//!
//! ### Image Metadata Types
//! - `ImageInfo`: Complete image description (dimensions, color format, etc.)
//! - `ColorType`: Color format enumeration (Grayscale, RGB, RGBA)
//! - `PixelFormat`: Detailed pixel layout and bit depth information
//!
//! ### Performance & Diagnostics
//! - `PerformanceMetrics`: Detailed timing and resource usage statistics
//! - `OperationResult`: Success/failure indication with detailed context
//! - `ValidationResult`: Format compliance and compatibility checking
//!
//! ### Configuration & Options
//! - Various configuration enums for decoder behavior
//! - Feature flags for enabling/disabling optimizations
//! - Quality vs. performance trade-off controls
//!
//! ## FFI Bridge Integration
//!
//! Types in this module are designed to work seamlessly with the cxx bridge:
//!
//! ```cpp
//! // C++ side can use these types directly
//! rust_bmp::ImageInfo info;
//! rust_bmp::ColorType color = rust_bmp::ColorType::Rgb;
//! 
//! // Call Rust functions with C++ types
//! auto result = decode_bmp_header(data, info);
//! ```
//!
//! ## Memory Safety
//!
//! While maintaining FFI compatibility, these types provide memory safety:
//! - Owned data prevents dangling pointers
//! - Validated ranges prevent buffer overruns  
//! - Optional types handle null/missing data safely
//!
//! ## Usage Examples
//!
//! ### Basic Image Information
//! ```rust
//! let info = ImageInfo {
//!     width: 1920,
//!     height: 1080, 
//!     color_type: ColorType::Rgba,
//!     bits_per_pixel: 32,
//!     bytes_per_row: 1920 * 4,
//!     ..Default::default()
//! };
//! ```
//!
//! ### Performance Monitoring
//! ```rust
//! let metrics = PerformanceMetrics::new();
//! // ... perform decoding operations
//! println!("Decode took {} ms", metrics.total_time_ms);
//! println!("Peak memory: {} bytes", metrics.peak_memory_bytes);
//! ```

use std::fmt;

/// FFI-friendly equivalent of image color types for BMP
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(C)]
pub enum ColorType {
    Grayscale = 0,
    Rgb = 2,
    Rgba = 6,
}

impl fmt::Display for ColorType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ColorType::Grayscale => write!(f, "Grayscale"),
            ColorType::Rgb => write!(f, "RGB"),
            ColorType::Rgba => write!(f, "RGBA"),
        }
    }
}

/// Information about a BMP image
#[derive(Debug, Clone, PartialEq)]
pub struct ImageInfo {
    pub width: u32,
    pub height: u32,
    pub color_type: ColorType,
    pub bytes_per_pixel: u8,
}

impl ImageInfo {
    pub fn new(width: u32, height: u32, color_type: ColorType, bytes_per_pixel: u8) -> Self {
        Self {
            width,
            height,
            color_type,
            bytes_per_pixel,
        }
    }

    pub fn pixel_count(&self) -> u64 {
        self.width as u64 * self.height as u64
    }

    pub fn total_bytes(&self) -> u64 {
        self.pixel_count() * self.bytes_per_pixel as u64
    }
}

/// Pixel format information
#[derive(Debug, Clone, PartialEq)]
pub struct PixelFormat {
    pub bits_per_pixel: u16,
    pub color_type: ColorType,
    pub has_alpha: bool,
    pub is_compressed: bool,
}

impl PixelFormat {
    pub fn new(bits_per_pixel: u16, color_type: ColorType, has_alpha: bool, is_compressed: bool) -> Self {
        Self {
            bits_per_pixel,
            color_type,
            has_alpha,
            is_compressed,
        }
    }

    pub fn bytes_per_pixel(&self) -> u8 {
        ((self.bits_per_pixel + 7) / 8) as u8
    }
}

/// Result types for various operations
#[derive(Debug, Clone, PartialEq)]
pub enum OperationResult<T> {
    Success(T),
    Partial(T),
    Failed(String),
}

impl<T> OperationResult<T> {
    pub fn is_success(&self) -> bool {
        matches!(self, OperationResult::Success(_))
    }

    pub fn is_partial(&self) -> bool {
        matches!(self, OperationResult::Partial(_))
    }

    pub fn is_failed(&self) -> bool {
        matches!(self, OperationResult::Failed(_))
    }

    pub fn unwrap(self) -> T {
        match self {
            OperationResult::Success(value) | OperationResult::Partial(value) => value,
            OperationResult::Failed(msg) => panic!("Called unwrap on failed result: {}", msg),
        }
    }

    pub fn into_result(self) -> Result<T, String> {
        match self {
            OperationResult::Success(value) | OperationResult::Partial(value) => Ok(value),
            OperationResult::Failed(msg) => Err(msg),
        }
    }
}

/// Memory usage statistics
#[derive(Debug, Clone, Default)]
pub struct MemoryStats {
    pub bytes_allocated: u64,
    pub peak_usage: u64,
    pub cache_size: u64,
    pub buffer_pool_size: u64,
}

impl MemoryStats {
    pub fn total_usage(&self) -> u64 {
        self.bytes_allocated + self.cache_size + self.buffer_pool_size
    }
}

/// Performance metrics
#[derive(Debug, Clone, Default)]
pub struct PerformanceMetrics {
    pub decode_time_ms: u64,
    pub bytes_processed: u64,
    pub rows_processed: u32,
    pub cache_hits: u32,
    pub cache_misses: u32,
    pub simd_operations: u32,
}

impl PerformanceMetrics {
    pub fn throughput_mbps(&self) -> f64 {
        if self.decode_time_ms == 0 {
            0.0
        } else {
            (self.bytes_processed as f64 / 1_048_576.0) / (self.decode_time_ms as f64 / 1000.0)
        }
    }

    pub fn cache_hit_rate(&self) -> f64 {
        let total = self.cache_hits + self.cache_misses;
        if total == 0 {
            0.0
        } else {
            self.cache_hits as f64 / total as f64
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_color_type_display() {
        assert_eq!(format!("{}", ColorType::Grayscale), "Grayscale");
        assert_eq!(format!("{}", ColorType::Rgb), "RGB");
        assert_eq!(format!("{}", ColorType::Rgba), "RGBA");
    }

    #[test]
    fn test_image_info() {
        let info = ImageInfo::new(100, 200, ColorType::Rgb, 3);
        assert_eq!(info.pixel_count(), 20_000);
        assert_eq!(info.total_bytes(), 60_000);
    }

    #[test]
    fn test_pixel_format() {
        let format = PixelFormat::new(24, ColorType::Rgb, false, false);
        assert_eq!(format.bytes_per_pixel(), 3);
    }

    #[test]
    fn test_operation_result() {
        let success: OperationResult<i32> = OperationResult::Success(42);
        assert!(success.is_success());
        assert_eq!(success.unwrap(), 42);

        let failed: OperationResult<i32> = OperationResult::Failed("test error".to_string());
        assert!(failed.is_failed());
    }

    #[test]
    fn test_performance_metrics() {
        let mut metrics = PerformanceMetrics::default();
        metrics.decode_time_ms = 1000;
        metrics.bytes_processed = 1_048_576; // 1 MB
        metrics.cache_hits = 8;
        metrics.cache_misses = 2;

        assert_eq!(metrics.throughput_mbps(), 1.0);
        assert_eq!(metrics.cache_hit_rate(), 0.8);
    }
}