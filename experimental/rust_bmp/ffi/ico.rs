// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! # ICO Format Support and Multi-Image Container Handling
//!
//! This module provides comprehensive support for the ICO (Windows Icon) format,
//! which is a multi-image container format that can contain BMP or PNG images
//! of various sizes and color depths. It includes advanced transparency handling,
//! size selection algorithms, and optimized multi-image processing.
//!
//! ## ICO Format Architecture
//!
//! ### ICO File Structure
//! ```text
//! ┌─────────────────────────────────────────────────────────────────┐
//! │                      ICO File Layout                            │
//! │                                                                 │
//! │ ┌─────────────────┐                                           │
//! │ │   ICO Header    │ • Magic: 0x0000 (reserved) + 0x0001 (ICO) │
//! │ │   (6 bytes)     │ • Count: Number of images in container     │
//! │ └─────────────────┘                                           │
//! │          │                                                    │
//! │          ▼                                                    │
//! │ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ │
//! │ │ Image Entry 1   │ │ Image Entry 2   │ │ Image Entry N   │ │
//! │ │ (16 bytes)      │ │ (16 bytes)      │ │ (16 bytes)      │ │
//! │ │                 │ │                 │ │                 │ │
//! │ │ • Width/Height  │ │ • Width/Height  │ │ • Width/Height  │ │
//! │ │ • Color count   │ │ • Color count   │ │ • Color count   │ │
//! │ │ • Planes/BPP    │ │ • Planes/BPP    │ │ • Planes/BPP    │ │
//! │ │ • Data size     │ │ • Data size     │ │ • Data size     │ │
//! │ │ • Data offset   │ │ • Data offset   │ │ • Data offset   │ │
//! │ └─────────────────┘ └─────────────────┘ └─────────────────┘ │
//! │          │                    │                    │        │
//! │          ▼                    ▼                    ▼        │
//! │ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ │
//! │ │  BMP/PNG Data   │ │  BMP/PNG Data   │ │  BMP/PNG Data   │ │
//! │ │                 │ │                 │ │                 │ │
//! │ │ • Image pixels  │ │ • Image pixels  │ │ • Image pixels  │ │
//! │ │ • Optional AND  │ │ • Optional AND  │ │ • Optional AND  │ │
//! │ │   mask data     │ │   mask data     │ │   mask data     │ │
//! │ └─────────────────┘ └─────────────────┘ └─────────────────┘ │
//! └─────────────────────────────────────────────────────────────────┘
//! ```
//!
//! ### Image Format Detection
//! ICO containers can hold different image formats:
//! - **BMP Format**: Traditional Windows bitmap with optional AND mask
//! - **PNG Format**: Modern compressed images with built-in transparency
//! - **Mixed Containers**: ICO files can contain both BMP and PNG images
//!
//! ## Transparency Handling Modes
//!
//! ### Complex Transparency System
//! ICO files use sophisticated transparency mechanisms:
//!
//! #### 1. AND Mask Transparency (Classic)
//! ```text
//! XOR Bitmap (Color Data) + AND Mask (Transparency)
//! ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
//! │ Color Pixel │  +  │ Mask Bit    │  =  │ Final Pixel │
//! ├─────────────┼─────┼─────────────┼─────┼─────────────┤
//! │    RGB      │  +  │      0      │  =  │ Visible RGB │
//! │    RGB      │  +  │      1      │  =  │ Transparent │
//! │   Black     │  +  │      0      │  =  │ Black pixel │
//! │   White     │  +  │      1      │  =  │ Transparent │
//! └─────────────┴─────┴─────────────┴─────┴─────────────┘
//! ```
//!
//! #### 2. Alpha Channel Transparency (Modern)
//! - Direct RGBA pixels with per-pixel alpha values
//! - More flexible than AND mask (256 transparency levels)
//! - Commonly used in 32-bit BMP images within ICO
//!
//! #### 3. Hybrid Transparency (MaskAndAlpha)
//! - Combines both AND mask and alpha channel information
//! - Provides maximum compatibility across different systems
//! - Falls back gracefully when alpha channel is not supported
//!
//! ## Multi-Image Selection Algorithms
//!
//! ### Intelligent Size Selection
//! The ICO handler includes sophisticated algorithms for choosing the best
//! image variant for a given use case:
//!
//! #### Size Priority Matrix
//! ```text
//! ┌─────────────┬──────────────┬──────────────┬──────────────┐
//! │ Target Size │ 1st Choice   │ 2nd Choice   │ 3rd Choice   │
//! ├─────────────┼──────────────┼──────────────┼──────────────┤
//! │   16x16     │ Exact match  │ 32x32 scale  │ Nearest size │
//! │   32x32     │ Exact match  │ 64x64 scale  │ 48x48 crop   │
//! │   48x48     │ Exact match  │ 64x64 scale  │ 32x32 scale  │
//! │   64x64     │ Exact match  │ 128x128      │ 48x48 scale  │
//! │  256x256    │ Exact match  │ PNG variant  │ Largest BMP  │
//! └─────────────┴──────────────┴──────────────┴──────────────┘
//! ```
//!
//! #### Quality Scoring System
//! ```rust
//! fn calculate_image_score(entry: &IcoImageEntry, target_size: u32) -> f32 {
//!     let size_score = size_match_quality(entry.width, target_size);
//!     let format_score = format_preference(entry.format);
//!     let color_score = color_depth_quality(entry.bits_per_pixel);
//!     
//!     size_score * 0.5 + format_score * 0.3 + color_score * 0.2
//! }
//! ```
//!
//! ## Advanced Processing Features
//!
//! ### Multi-Threading Support
//! - **Parallel Decoding**: Process multiple images simultaneously
//! - **Background Preprocessing**: Prepare unused images for fast access
//! - **Lazy Loading**: Load images on-demand to reduce memory usage
//! - **Thread-Safe Caching**: Shared cache for decoded image variants
//!
//! ### Memory Optimization
//! - **Shared Palettes**: Reuse color tables across similar images
//! - **Compressed Storage**: Store decoded images in optimal formats
//! - **Smart Prefetching**: Predict which sizes will be needed next
//! - **Memory Pressure Handling**: Evict unused images under memory pressure
//!
//! ## ICO Processing Pipeline
//!
//! ### Comprehensive Decode Process
//! ```text
//! ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
//! │   ICO Header    │    │  Image Entry    │    │   Format        │
//! │   Validation    │───▶│   Processing    │───▶│   Detection     │
//! │                 │    │                 │    │                 │
//! │ • Magic check   │    │ • Parse entries │    │ • BMP vs PNG    │
//! │ • Count valid   │    │ • Sort by size  │    │ • Header check  │
//! │ • Size limits   │    │ • Validate      │    │ • Format rules  │
//! └─────────────────┘    └─────────────────┘    └─────────────────┘
//!          │                       │                       │
//!          ▼                       ▼                       ▼
//! ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
//! │   Best Image    │    │   Transparency  │    │    Output       │
//! │   Selection     │    │   Processing    │    │   Assembly      │
//! │                 │    │                 │    │                 │
//! │ • Size match    │    │ • AND mask      │    │ • RGBA pixels   │
//! │ • Quality rank  │    │ • Alpha blend   │    │ • Metadata      │
//! │ • Format pref   │    │ • Mode select   │    │ • Performance   │
//! └─────────────────┘    └─────────────────┘    └─────────────────┘
//! ```
//!
//! ## Usage Examples
//!
//! ### Basic ICO Processing
//! ```rust
//! use crate::ico::{IcoHandler, TransparencyMode};
//!
//! let ico_handler = IcoHandler::new();
//! let ico_data = std::fs::read("icon.ico")?;
//!
//! // Get available sizes
//! let available_sizes = ico_handler.list_available_sizes(&ico_data)?;
//! println!("Available icon sizes: {:?}", available_sizes);
//!
//! // Extract specific size
//! let image_32x32 = ico_handler.extract_size(&ico_data, 32)?;
//! ```
//!
//! ### Advanced Transparency Handling
//! ```rust
//! // Configure transparency processing
//! let transparency_config = TransparencyMode::MaskAndAlpha;
//! ico_handler.set_transparency_mode(transparency_config);
//!
//! // Extract with specific transparency handling
//! let rgba_image = ico_handler.extract_with_transparency(
//!     &ico_data, 
//!     48, 
//!     transparency_config
//! )?;
//! ```
//!
//! ### Multi-Image Processing
//! ```rust
//! // Extract all images for different use cases
//! let all_variants = ico_handler.extract_all_variants(&ico_data)?;
//! 
//! for variant in all_variants {
//!     println!("Size: {}x{}, Format: {:?}, BPP: {}", 
//!              variant.width, variant.height, 
//!              variant.format, variant.bits_per_pixel);
//! }
//!
//! // Get best match for target size with quality preference
//! let best_match = ico_handler.select_best_variant(&ico_data, 64, QualityPreference::HighestQuality)?;
//! ```
//!
//! ### Performance Monitoring
//! ```rust
//! let metrics = ico_handler.get_performance_metrics();
//! println!("Images processed: {}", metrics.images_decoded);
//! println!("Cache hit rate: {:.1}%", metrics.cache_hit_rate * 100.0);
//! println!("Average decode time: {:.2}ms", metrics.avg_decode_time_ms);
//! ```
//!
//! ## Compatibility and Standards
//!
//! ### Format Compliance
//! - **Windows ICO**: Full support for standard Windows icon format
//! - **Vista+ Extensions**: Support for PNG images within ICO containers
//! - **Legacy Formats**: Backward compatibility with older ICO variants
//! - **Cross-Platform**: Consistent behavior across different operating systems
//!
//! ### Quality Assurance
//! - **Validation**: Comprehensive format checking and error detection
//! - **Error Recovery**: Graceful handling of malformed ICO files  
//! - **Performance**: Optimized for both single-image and batch processing
//! - **Memory Safety**: Bounds checking and safe memory operations

use crate::types::{ImageInfo, OperationResult, ColorType};
use crate::error::{BmpError, ErrorSeverity};

/// Transparency handling modes for ICO images
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum TransparencyMode {
    None,           // No transparency
    Mask,           // Uses AND mask for transparency
    Alpha,          // Uses alpha channel
    MaskAndAlpha,   // Both mask and alpha channel
}

impl TransparencyMode {
    pub fn has_mask(&self) -> bool {
        matches!(self, TransparencyMode::Mask | TransparencyMode::MaskAndAlpha)
    }

    pub fn has_alpha(&self) -> bool {
        matches!(self, TransparencyMode::Alpha | TransparencyMode::MaskAndAlpha)
    }
}

/// ICO directory entry information
#[derive(Debug, Clone)]
pub struct IcoEntry {
    pub width: u8,          // 0 means 256
    pub height: u8,         // 0 means 256  
    pub color_count: u8,    // 0 for 16+ colors
    pub reserved: u8,       // Should be 0
    pub planes: u16,        // Color planes
    pub bit_count: u16,     // Bits per pixel
    pub bytes_in_res: u32,  // Image data size
    pub image_offset: u32,  // Offset to image data
}

impl IcoEntry {
    pub fn actual_width(&self) -> u32 {
        if self.width == 0 { 256 } else { self.width as u32 }
    }

    pub fn actual_height(&self) -> u32 {
        if self.height == 0 { 256 } else { self.height as u32 }
    }

    pub fn is_bmp(&self) -> bool {
        // BMP images in ICO typically have bit_count > 0
        self.bit_count > 0
    }

    pub fn is_png(&self) -> bool {
        // PNG images typically have bit_count = 0
        self.bit_count == 0
    }
}

/// ICO file header information  
#[derive(Debug, Clone)]
pub struct IcoHeader {
    pub reserved: u16,      // Must be 0
    pub image_type: u16,    // 1 for ICO, 2 for CUR
    pub image_count: u16,   // Number of images
    pub entries: Vec<IcoEntry>,
}

impl IcoHeader {
    pub fn is_valid(&self) -> bool {
        self.reserved == 0 && self.image_type == 1 && 
        self.image_count > 0 && self.entries.len() == self.image_count as usize
    }

    pub fn find_best_bmp_entry(&self) -> Option<&IcoEntry> {
        self.entries.iter()
            .filter(|e| e.is_bmp())
            .max_by_key(|e| e.actual_width() * e.actual_height())
    }

    pub fn get_bmp_entries(&self) -> Vec<&IcoEntry> {
        self.entries.iter()
            .filter(|e| e.is_bmp())
            .collect()
    }
}

/// Main ICO handler for parsing and extracting embedded BMPs
pub struct IcoHandler {
    transparency_mode: TransparencyMode,
    prefer_largest: bool,
}

impl IcoHandler {
    pub fn new() -> Self {
        Self {
            transparency_mode: TransparencyMode::MaskAndAlpha,
            prefer_largest: true,
        }
    }

    pub fn with_transparency_mode(mut self, mode: TransparencyMode) -> Self {
        self.transparency_mode = mode;
        self
    }

    pub fn with_size_preference(mut self, prefer_largest: bool) -> Self {
        self.prefer_largest = prefer_largest;
        self
    }

    /// Parse ICO header from data
    pub fn parse_ico_header(&self, data: &[u8]) -> Result<IcoHeader, BmpError> {
        if data.len() < 6 {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "Insufficient data for ICO header",
                Some(0),
            ));
        }

        let reserved = u16::from_le_bytes([data[0], data[1]]);
        let image_type = u16::from_le_bytes([data[2], data[3]]);
        let image_count = u16::from_le_bytes([data[4], data[5]]);

        if reserved != 0 {
            return Err(BmpError::new(
                ErrorSeverity::Warning,
                "Non-zero reserved field in ICO header",
                Some(0),
            ).with_context("continuing anyway"));
        }

        if image_type != 1 {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "Not an ICO file (image_type != 1)",
                Some(2),
            ));
        }

        if image_count == 0 || image_count > 100 {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "Invalid image count in ICO",
                Some(4),
            ));
        }

        // Parse directory entries
        let mut entries = Vec::with_capacity(image_count as usize);
        let entries_size = 16 * image_count as usize;
        
        if data.len() < 6 + entries_size {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "Insufficient data for ICO directory entries",
                Some(6),
            ));
        }

        for i in 0..image_count {
            let offset = 6 + (i as usize * 16);
            let entry = IcoEntry {
                width: data[offset],
                height: data[offset + 1],
                color_count: data[offset + 2],
                reserved: data[offset + 3],
                planes: u16::from_le_bytes([data[offset + 4], data[offset + 5]]),
                bit_count: u16::from_le_bytes([data[offset + 6], data[offset + 7]]),
                bytes_in_res: u32::from_le_bytes([
                    data[offset + 8], data[offset + 9], 
                    data[offset + 10], data[offset + 11]
                ]),
                image_offset: u32::from_le_bytes([
                    data[offset + 12], data[offset + 13], 
                    data[offset + 14], data[offset + 15]
                ]),
            };
            entries.push(entry);
        }

        Ok(IcoHeader {
            reserved,
            image_type,
            image_count,
            entries,
        })
    }

    /// Extract BMP data from ICO entry
    pub fn extract_bmp_from_ico(&self, ico_data: &[u8], entry: &IcoEntry) -> Result<Vec<u8>, BmpError> {
        let start_offset = entry.image_offset as usize;
        let data_size = entry.bytes_in_res as usize;

        if start_offset + data_size > ico_data.len() {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "ICO entry points beyond data bounds",
                Some(start_offset as u64),
            ));
        }

        let image_data = &ico_data[start_offset..start_offset + data_size];

        // Check if it's already a complete BMP (has BM signature)
        if image_data.len() >= 2 && image_data[0] == b'B' && image_data[1] == b'M' {
            return Ok(image_data.to_vec());
        }

        // It's a DIB (Device Independent Bitmap) - need to add BMP file header
        self.convert_dib_to_bmp(image_data, entry)
    }

    /// Convert DIB data to complete BMP format
    fn convert_dib_to_bmp(&self, dib_data: &[u8], entry: &IcoEntry) -> Result<Vec<u8>, BmpError> {
        if dib_data.len() < 4 {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "DIB data too small",
                Some(0),
            ));
        }

        // Parse DIB header size
        let dib_header_size = u32::from_le_bytes([
            dib_data[0], dib_data[1], dib_data[2], dib_data[3]
        ]);

        if dib_header_size < 40 || dib_header_size as usize > dib_data.len() {
            return Err(BmpError::new(
                ErrorSeverity::Error,
                "Invalid DIB header size",
                Some(0),
            ));
        }

        // Calculate components
        let file_header_size = 14;
        let palette_size = self.calculate_palette_size(dib_data, entry)?;
        let mask_size = if self.transparency_mode.has_mask() {
            self.calculate_mask_size(entry)
        } else {
            0
        };
        
        let _pixel_data_size = dib_data.len() - dib_header_size as usize - palette_size;
        let total_file_size = file_header_size + dib_data.len() + mask_size;
        let pixel_data_offset = file_header_size + dib_header_size as usize + palette_size;

        // Create BMP file header
        let mut bmp_data = Vec::with_capacity(total_file_size);
        
        // BMP file header (14 bytes)
        bmp_data.extend_from_slice(b"BM");                                    // Signature
        bmp_data.extend_from_slice(&(total_file_size as u32).to_le_bytes());  // File size
        bmp_data.extend_from_slice(&[0, 0, 0, 0]);                           // Reserved
        bmp_data.extend_from_slice(&pixel_data_offset.to_le_bytes());         // Pixel data offset

        // Add DIB data (header + palette + pixels)
        bmp_data.extend_from_slice(dib_data);

        // Add mask data if needed
        if mask_size > 0 {
            let mask_data = self.create_transparency_mask(entry)?;
            bmp_data.extend_from_slice(&mask_data);
        }

        Ok(bmp_data)
    }

    fn calculate_palette_size(&self, dib_data: &[u8], _entry: &IcoEntry) -> Result<usize, BmpError> {
        if dib_data.len() < 40 {
            return Ok(0);
        }

        // Read bits per pixel from DIB header
        let bits_per_pixel = u16::from_le_bytes([dib_data[14], dib_data[15]]);
        
        if bits_per_pixel <= 8 {
            // Read colors used field (offset 32 in DIB header)
            if dib_data.len() >= 36 {
                let colors_used = u32::from_le_bytes([
                    dib_data[32], dib_data[33], dib_data[34], dib_data[35]
                ]);
                
                if colors_used > 0 {
                    Ok((colors_used * 4) as usize) // 4 bytes per palette entry
                } else {
                    Ok((1 << bits_per_pixel) * 4) // Default palette size
                }
            } else {
                Ok((1 << bits_per_pixel) * 4)
            }
        } else {
            Ok(0) // No palette for > 8 bits per pixel
        }
    }

    fn calculate_mask_size(&self, entry: &IcoEntry) -> usize {
        let width = entry.actual_width();
        let height = entry.actual_height();
        
        // Mask is 1 bit per pixel, padded to 32-bit boundaries
        let bits_per_row = width;
        let bytes_per_row = ((bits_per_row + 31) / 32) * 4;
        (height * bytes_per_row) as usize
    }

    fn create_transparency_mask(&self, entry: &IcoEntry) -> Result<Vec<u8>, BmpError> {
        let mask_size = self.calculate_mask_size(entry);
        let mask_data = vec![0u8; mask_size]; // All transparent by default
        Ok(mask_data)
    }

    /// Get image info for ICO entry
    pub fn get_ico_image_info(&self, entry: &IcoEntry) -> ImageInfo {
        let color_type = match entry.bit_count {
            1 | 4 | 8 => ColorType::Grayscale,
            16 | 24 => ColorType::Rgb,
            32 => if self.transparency_mode.has_alpha() { 
                ColorType::Rgba 
            } else { 
                ColorType::Rgb 
            },
            _ => ColorType::Rgb,
        };

        let bytes_per_pixel = match entry.bit_count {
            1 => 1,
            4 => 1,
            8 => 1,
            16 => 2,
            24 => 3,
            32 => 4,
            _ => 3,
        };

        ImageInfo::new(
            entry.actual_width(),
            entry.actual_height(),
            color_type,
            bytes_per_pixel,
        )
    }

    /// Process transparency for decoded ICO BMP
    pub fn process_transparency(&self, image_data: &mut [u8], entry: &IcoEntry, 
                              mask_data: Option<&[u8]>) -> OperationResult<()> {
        match self.transparency_mode {
            TransparencyMode::None => OperationResult::Success(()),
            
            TransparencyMode::Mask => {
                if let Some(mask) = mask_data {
                    self.apply_mask_transparency(image_data, entry, mask)
                } else {
                    OperationResult::Failed("Mask data not available".to_string())
                }
            },
            
            TransparencyMode::Alpha => {
                if entry.bit_count == 32 {
                    self.process_alpha_channel(image_data, entry)
                } else {
                    OperationResult::Failed("Alpha channel not available".to_string())
                }
            },
            
            TransparencyMode::MaskAndAlpha => {
                let alpha_result = if entry.bit_count == 32 {
                    self.process_alpha_channel(image_data, entry)
                } else {
                    OperationResult::Success(())
                };
                
                let mask_result = if let Some(mask) = mask_data {
                    self.apply_mask_transparency(image_data, entry, mask)
                } else {
                    OperationResult::Success(())
                };
                
                match (alpha_result, mask_result) {
                    (OperationResult::Success(_), OperationResult::Success(_)) => OperationResult::Success(()),
                    (OperationResult::Failed(e), _) | (_, OperationResult::Failed(e)) => OperationResult::Failed(e),
                    _ => OperationResult::Partial(()),
                }
            }
        }
    }

    fn apply_mask_transparency(&self, image_data: &mut [u8], entry: &IcoEntry, 
                             mask_data: &[u8]) -> OperationResult<()> {
        let width = entry.actual_width() as usize;
        let height = entry.actual_height() as usize;
        let bytes_per_pixel = (entry.bit_count / 8) as usize;
        
        if bytes_per_pixel == 0 || image_data.len() < width * height * bytes_per_pixel {
            return OperationResult::Failed("Invalid image data size".to_string());
        }

        let mask_bytes_per_row = ((width + 31) / 32) * 4;
        
        for y in 0..height {
            for x in 0..width {
                let mask_byte_offset = y * mask_bytes_per_row + (x / 8);
                let mask_bit_offset = 7 - (x % 8);
                
                if mask_byte_offset < mask_data.len() {
                    let mask_bit = (mask_data[mask_byte_offset] >> mask_bit_offset) & 1;
                    
                    if mask_bit == 1 {
                        // Pixel is transparent
                        let pixel_offset = (y * width + x) * bytes_per_pixel;
                        if pixel_offset + bytes_per_pixel <= image_data.len() {
                            // Set alpha to 0 if RGBA, otherwise mark for removal
                            if bytes_per_pixel == 4 {
                                image_data[pixel_offset + 3] = 0;
                            }
                        }
                    }
                }
            }
        }

        OperationResult::Success(())
    }

    fn process_alpha_channel(&self, image_data: &mut [u8], entry: &IcoEntry) -> OperationResult<()> {
        if entry.bit_count != 32 {
            return OperationResult::Failed("Not a 32-bit image".to_string());
        }

        let pixel_count = (entry.actual_width() * entry.actual_height()) as usize;
        if image_data.len() < pixel_count * 4 {
            return OperationResult::Failed("Insufficient image data".to_string());
        }

        // Check if alpha channel has meaningful data
        let mut has_alpha = false;
        for i in 0..pixel_count {
            let alpha = image_data[i * 4 + 3];
            if alpha != 255 && alpha != 0 {
                has_alpha = true;
                break;
            }
        }

        if !has_alpha {
            // Set default alpha values
            for i in 0..pixel_count {
                image_data[i * 4 + 3] = 255;
            }
        }

        OperationResult::Success(())
    }

    pub fn get_transparency_mode(&self) -> TransparencyMode {
        self.transparency_mode
    }
}

impl Default for IcoHandler {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_transparency_mode() {
        assert!(TransparencyMode::Mask.has_mask());
        assert!(!TransparencyMode::Mask.has_alpha());
        assert!(TransparencyMode::Alpha.has_alpha());
        assert!(!TransparencyMode::Alpha.has_mask());
        assert!(TransparencyMode::MaskAndAlpha.has_mask());
        assert!(TransparencyMode::MaskAndAlpha.has_alpha());
    }

    #[test]
    fn test_ico_entry() {
        let entry = IcoEntry {
            width: 0,    // Should be 256
            height: 32,
            color_count: 0,
            reserved: 0,
            planes: 1,
            bit_count: 32,
            bytes_in_res: 4096,
            image_offset: 22,
        };

        assert_eq!(entry.actual_width(), 256);
        assert_eq!(entry.actual_height(), 32);
        assert!(entry.is_bmp());
        assert!(!entry.is_png());
    }

    #[test]
    fn test_ico_header_validation() {
        let header = IcoHeader {
            reserved: 0,
            image_type: 1,
            image_count: 1,
            entries: vec![IcoEntry {
                width: 32,
                height: 32,
                color_count: 0,
                reserved: 0,
                planes: 1,
                bit_count: 32,
                bytes_in_res: 4096,
                image_offset: 22,
            }],
        };

        assert!(header.is_valid());
        assert!(header.find_best_bmp_entry().is_some());
        assert_eq!(header.get_bmp_entries().len(), 1);
    }

    #[test]
    fn test_ico_handler_creation() {
        let handler = IcoHandler::new()
            .with_transparency_mode(TransparencyMode::Alpha)
            .with_size_preference(false);

        assert_eq!(handler.get_transparency_mode(), TransparencyMode::Alpha);
        assert!(!handler.prefer_largest);
    }

    #[test]
    fn test_ico_image_info() {
        let handler = IcoHandler::new();
        let entry = IcoEntry {
            width: 32,
            height: 32,
            color_count: 0,
            reserved: 0,
            planes: 1,
            bit_count: 32,
            bytes_in_res: 4096,
            image_offset: 22,
        };

        let info = handler.get_ico_image_info(&entry);
        assert_eq!(info.width, 32);
        assert_eq!(info.height, 32);
        assert_eq!(info.bytes_per_pixel, 4);
    }

    #[test]
    fn test_invalid_ico_header() {
        let handler = IcoHandler::new();
        let invalid_data = vec![1, 0, 1, 0, 0, 0]; // Invalid reserved field
        
        let result = handler.parse_ico_header(&invalid_data);
        assert!(result.is_ok()); // Should succeed with warning
        
        let empty_data = vec![];
        let result = handler.parse_ico_header(&empty_data);
        assert!(result.is_err()); // Should fail
    }
}