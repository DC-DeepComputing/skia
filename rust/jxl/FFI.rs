// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides C++ bindings for the `jxl-rs` JPEG XL decoder.
//!
//! The public API of this crate is the C++ API declared by the `#[cxx::bridge]`
//! macro below and exposed through the auto-generated `FFI.rs.h` header.

use jxl::api::{
    JxlBasicInfo as InternalBasicInfo, JxlDecoder, JxlDecoderOptions, JxlOutputBuffer,
    JxlProgressiveMode, ProcessingResult, check_signature, states,
};
use jxl::headers::extra_channels::ExtraChannel;
use jxl::image::{Image, Rect};
use std::slice;

#[cxx::bridge(namespace = "rust_jxl")]
mod ffi {
    /// FFI-friendly equivalent of jxl-rs processing status.
    #[derive(Debug)]
    enum DecodingStatus {
        Success = 0,
        Error = 1,
        NeedMoreInput = 2,
        BasicInfo = 3,
        Frame = 5,
        FullImage = 6,
    }

    /// Pixel format for decoded output.
    #[derive(Debug)]
    enum PixelFormat {
        Rgba8 = 0,
        Rgba16 = 1,
        RgbaF32 = 2,
    }

    /// FFI-friendly equivalent of JXL basic image information.
    #[derive(Debug, Clone)]
    struct BasicInfo {
        width: u32,
        height: u32,
        bits_per_sample: u32,
        num_extra_channels: u32,
        has_alpha: bool,
        alpha_premultiplied: bool,
        have_animation: bool,
        animation_loop_count: u32,
        animation_tps_numerator: u32,
        animation_tps_denominator: u32,
        uses_original_profile: bool,
        orientation: u32,
    }

    /// FFI-friendly equivalent of JXL frame header.
    #[derive(Debug, Clone)]
    struct FrameHeader {
        duration_ms: u32,
        is_last: bool,
        name_length: u32,
    }

    // Rust functions, types, and methods that are exposed through FFI.
    //
    // To avoid duplication, there are no doc comments inside the `extern "Rust"`
    // section. The doc comments of these items can instead be found in the
    // actual Rust code, outside of the `#[cxx::bridge]` manifest.
    extern "Rust" {
        /// Check if data begins with valid JXL signature.
        fn signature_check(data: &[u8]) -> bool;

        /// Get library version string.
        fn version() -> &'static str;

        type Decoder;

        /// Create a new decoder instance.
        fn new_decoder() -> Box<Decoder>;

        /// Reset decoder to initial state for reuse.
        fn reset(self: &mut Decoder);

        /// Set input data for decoding.
        ///
        /// `data` - Input buffer containing JXL data
        /// `all_input` - Whether all data has been provided
        fn set_input(self: &mut Decoder, data: &[u8], all_input: bool) -> DecodingStatus;

        /// Process next decoding step.
        /// Returns status indicating what information is now available.
        fn process(self: &mut Decoder) -> DecodingStatus;

        /// Get basic image information (dimensions, color info, animation).
        /// Valid after receiving BasicInfo status.
        fn get_basic_info(self: &Decoder) -> BasicInfo;

        /// Get frame header information (duration, etc).
        /// Valid after receiving Frame status.
        fn get_frame_header(self: &Decoder) -> FrameHeader;

        /// Set output pixel format for decoded frames.
        fn set_pixel_format(self: &mut Decoder, format: PixelFormat);

        /// Get decoded pixels - writes to provided buffer.
        /// Valid after receiving FullImage status.
        /// Buffer must be large enough for width * height * 4 * bytes_per_component.
        fn get_pixels(self: &mut Decoder, buffer: &mut [u8]) -> DecodingStatus;

        /// Get ICC color profile data as slice.
        /// Valid after receiving BasicInfo status.
        fn get_icc_profile(self: &Decoder) -> &[u8];

        /// Get total frame count (for animations).
        fn get_frame_count(self: &Decoder) -> u32;

        /// Check if more frames are available for decoding.
        fn has_more_frames(self: &Decoder) -> bool;

        /// Get last error message.
        fn get_error(self: &Decoder) -> &str;
    }
}

// Re-export types for convenience.
pub use ffi::{BasicInfo, DecodingStatus, FrameHeader, PixelFormat};

// =============================================================================
// Decoder Implementation
// =============================================================================

enum DecoderState {
    Initialized(JxlDecoder<states::Initialized>),
    WithImageInfo(JxlDecoder<states::WithImageInfo>),
    WithFrameInfo(JxlDecoder<states::WithFrameInfo>),
    Empty,
}

pub struct Decoder {
    state: DecoderState,
    input_buffer: Vec<u8>,
    input_consumed: usize,
    all_input_received: bool,
    basic_info: BasicInfo,
    frame_header: FrameHeader,
    current_frame: u32,
    frame_count: u32,
    has_more_frames: bool,
    pixel_format: PixelFormat,
    icc_profile: Vec<u8>,
    error_message: String,
    pixel_buffer: Vec<u8>,
}

impl Decoder {
    fn new() -> Self {
        let mut options = JxlDecoderOptions::default();
        options.xyb_output_linear = false;
        options.progressive_mode = JxlProgressiveMode::FullFrame;

        Self {
            state: DecoderState::Initialized(JxlDecoder::new(options)),
            input_buffer: Vec::new(),
            input_consumed: 0,
            all_input_received: false,
            basic_info: BasicInfo::default(),
            frame_header: FrameHeader::default(),
            current_frame: 0,
            frame_count: 1,
            has_more_frames: true,
            pixel_format: PixelFormat::Rgba8,
            icc_profile: Vec::new(),
            error_message: String::new(),
            pixel_buffer: Vec::new(),
        }
    }

    fn set_error(&mut self, msg: &str) {
        self.error_message = msg.to_string();
    }

    fn process_internal(&mut self) -> DecodingStatus {
        let input_slice = &self.input_buffer[self.input_consumed..];
        if input_slice.is_empty() && !self.all_input_received {
            return DecodingStatus::NeedMoreInput;
        }

        let state = std::mem::replace(&mut self.state, DecoderState::Empty);

        match state {
            DecoderState::Initialized(decoder) => {
                let mut input = input_slice;
                let input_before = input.len();
                match decoder.process(&mut input) {
                    Ok(ProcessingResult::Complete { result }) => {
                        self.input_consumed += input_before - input.len();
                        self.basic_info = BasicInfo::from(result.basic_info());

                        // Extract ICC profile.
                        let color_profile = result.embedded_color_profile();
                        let icc = color_profile.as_icc();
                        if !icc.is_empty() {
                            self.icc_profile = icc.into_owned();
                        }

                        self.state = DecoderState::WithImageInfo(result);
                        DecodingStatus::BasicInfo
                    }
                    Ok(ProcessingResult::NeedsMoreInput { fallback, .. }) => {
                        self.input_consumed += input_before - input.len();
                        self.state = DecoderState::Initialized(fallback);
                        if self.all_input_received {
                            self.set_error("Incomplete JXL header");
                            DecodingStatus::Error
                        } else {
                            DecodingStatus::NeedMoreInput
                        }
                    }
                    Err(e) => {
                        self.set_error(&format!("Decoder error: {}", e));
                        DecodingStatus::Error
                    }
                }
            }

            DecoderState::WithImageInfo(decoder) => {
                let mut input = input_slice;
                let input_before = input.len();
                match decoder.process(&mut input) {
                    Ok(ProcessingResult::Complete { result }) => {
                        self.input_consumed += input_before - input.len();
                        let fh = result.frame_header();
                        self.frame_header.duration_ms = fh.duration.map(|d| d as u32).unwrap_or(0);
                        self.frame_header.is_last = false;
                        self.frame_header.name_length = fh.name.len() as u32;
                        self.state = DecoderState::WithFrameInfo(result);
                        DecodingStatus::Frame
                    }
                    Ok(ProcessingResult::NeedsMoreInput { fallback, .. }) => {
                        self.input_consumed += input_before - input.len();
                        self.state = DecoderState::WithImageInfo(fallback);
                        if self.all_input_received {
                            self.set_error("Incomplete frame header");
                            DecodingStatus::Error
                        } else {
                            DecodingStatus::NeedMoreInput
                        }
                    }
                    Err(e) => {
                        self.set_error(&format!("Frame header error: {}", e));
                        DecodingStatus::Error
                    }
                }
            }

            DecoderState::WithFrameInfo(decoder) => {
                let mut input = input_slice;
                let input_before = input.len();

                let width = self.basic_info.width as usize;
                let height = self.basic_info.height as usize;

                // Allocate RGB buffer.
                let mut rgb_image = match Image::<f32>::new_with_value((width * 3, height), 0.0f32) {
                    Ok(img) => img,
                    Err(e) => {
                        self.set_error(&format!("Failed to allocate RGB buffer: {}", e));
                        return DecodingStatus::Error;
                    }
                };

                // Allocate alpha buffer if needed.
                let mut alpha_image = if self.basic_info.has_alpha {
                    match Image::<f32>::new_with_value((width, height), 0.0f32) {
                        Ok(img) => Some(img),
                        Err(e) => {
                            self.set_error(&format!("Failed to allocate alpha buffer: {}", e));
                            return DecodingStatus::Error;
                        }
                    }
                } else {
                    None
                };

                // Decode the frame.
                let result = if let Some(ref mut alpha_img) = alpha_image {
                    let rgb_output = JxlOutputBuffer::from_image_rect_mut(
                        rgb_image.get_rect_mut(Rect { origin: (0, 0), size: (width * 3, height) }).into_raw()
                    );
                    let alpha_output = JxlOutputBuffer::from_image_rect_mut(
                        alpha_img.get_rect_mut(Rect { origin: (0, 0), size: (width, height) }).into_raw()
                    );
                    decoder.process(&mut input, &mut [rgb_output, alpha_output])
                } else {
                    let rgb_output = JxlOutputBuffer::from_image_rect_mut(
                        rgb_image.get_rect_mut(Rect { origin: (0, 0), size: (width * 3, height) }).into_raw()
                    );
                    decoder.process(&mut input, &mut [rgb_output])
                };

                // Copy decoded data to pixel buffer.
                let rgb_bytes_per_pixel = 3 * std::mem::size_of::<f32>();
                let rgb_buffer_size = width * height * rgb_bytes_per_pixel;
                let alpha_bytes_per_pixel = std::mem::size_of::<f32>();
                let alpha_buffer_size = width * height * alpha_bytes_per_pixel;
                let total_size = rgb_buffer_size + if self.basic_info.has_alpha { alpha_buffer_size } else { 0 };
                self.pixel_buffer.resize(total_size, 0u8);

                // Copy RGB data.
                for y in 0..height {
                    let src_row = rgb_image.row(y);
                    let dst_start = y * width * 3;
                    let dst_slice = unsafe {
                        slice::from_raw_parts_mut(
                            self.pixel_buffer.as_mut_ptr().add(dst_start * std::mem::size_of::<f32>()) as *mut f32,
                            width * 3,
                        )
                    };
                    dst_slice.copy_from_slice(&src_row[..width * 3]);
                }

                // Copy alpha data if present.
                if let Some(ref alpha_img) = alpha_image {
                    for y in 0..height {
                        let src_row = alpha_img.row(y);
                        let dst_start = rgb_buffer_size + y * width * std::mem::size_of::<f32>();
                        let dst_slice = unsafe {
                            slice::from_raw_parts_mut(
                                self.pixel_buffer.as_mut_ptr().add(dst_start) as *mut f32,
                                width,
                            )
                        };
                        dst_slice.copy_from_slice(&src_row[..width]);
                    }
                }

                match result {
                    Ok(ProcessingResult::Complete { result }) => {
                        self.input_consumed += input_before - input.len();
                        self.current_frame += 1;
                        self.has_more_frames = result.has_more_frames();
                        if self.has_more_frames {
                            self.state = DecoderState::WithImageInfo(result);
                        } else {
                            self.frame_count = self.current_frame;
                            self.state = DecoderState::WithImageInfo(result);
                        }
                        DecodingStatus::FullImage
                    }
                    Ok(ProcessingResult::NeedsMoreInput { fallback, .. }) => {
                        self.input_consumed += input_before - input.len();
                        self.state = DecoderState::WithFrameInfo(fallback);
                        if self.all_input_received {
                            self.set_error("Incomplete frame data");
                            DecodingStatus::Error
                        } else {
                            DecodingStatus::NeedMoreInput
                        }
                    }
                    Err(e) => {
                        self.set_error(&format!("Frame decode error: {}", e));
                        DecodingStatus::Error
                    }
                }
            }

            DecoderState::Empty => {
                self.set_error("Internal error: decoder in empty state");
                DecodingStatus::Error
            }
        }
    }

    /// Reset decoder to initial state for reuse.
    fn reset(&mut self) {
        let mut options = JxlDecoderOptions::default();
        options.xyb_output_linear = false;
        options.progressive_mode = JxlProgressiveMode::FullFrame;

        self.state = DecoderState::Initialized(JxlDecoder::new(options));
        self.input_buffer.clear();
        self.input_consumed = 0;
        self.all_input_received = false;
        self.basic_info = BasicInfo::default();
        self.frame_header = FrameHeader::default();
        self.current_frame = 0;
        self.frame_count = 1;
        self.has_more_frames = true;
        self.icc_profile.clear();
        self.error_message.clear();
        self.pixel_buffer.clear();
    }

    /// Set input data for decoding.
    fn set_input(&mut self, data: &[u8], all_input: bool) -> DecodingStatus {
        if !data.is_empty() {
            self.input_buffer.clear();
            self.input_buffer.extend_from_slice(data);
            self.input_consumed = 0;
        }
        self.all_input_received = all_input;
        DecodingStatus::Success
    }

    /// Process next decoding step.
    fn process(&mut self) -> DecodingStatus {
        self.process_internal()
    }

    /// Get basic image information.
    fn get_basic_info(&self) -> BasicInfo {
        self.basic_info.clone()
    }

    /// Get frame header information.
    fn get_frame_header(&self) -> FrameHeader {
        self.frame_header.clone()
    }

    /// Set output pixel format.
    fn set_pixel_format(&mut self, format: PixelFormat) {
        self.pixel_format = format;
    }

    /// Get decoded pixels - writes to provided buffer.
    fn get_pixels(&mut self, buffer: &mut [u8]) -> DecodingStatus {
        if self.pixel_buffer.is_empty() {
            self.error_message = "No decoded image available".to_string();
            return DecodingStatus::Error;
        }

        let width = self.basic_info.width as usize;
        let height = self.basic_info.height as usize;
        let has_alpha = self.basic_info.has_alpha;

        let rgb_f32_count = width * height * 3;
        let rgb_buffer = unsafe {
            slice::from_raw_parts(self.pixel_buffer.as_ptr() as *const f32, rgb_f32_count)
        };

        let alpha_buffer = if has_alpha {
            let rgb_byte_size = rgb_f32_count * std::mem::size_of::<f32>();
            let alpha_f32_count = width * height;
            unsafe {
                slice::from_raw_parts(
                    (self.pixel_buffer.as_ptr().add(rgb_byte_size)) as *const f32,
                    alpha_f32_count,
                )
            }
        } else {
            &[][..]
        };

        match self.pixel_format {
            PixelFormat::Rgba8 => {
                let required_size = width * height * 4;
                if buffer.len() < required_size {
                    self.error_message = "Buffer too small".to_string();
                    return DecodingStatus::Error;
                }
                for y in 0..height {
                    for x in 0..width {
                        let pixel_idx = y * width + x;
                        let rgb_idx = pixel_idx * 3;
                        let dst_idx = pixel_idx * 4;
                        buffer[dst_idx] = (rgb_buffer[rgb_idx].clamp(0.0, 1.0) * 255.0) as u8;
                        buffer[dst_idx + 1] = (rgb_buffer[rgb_idx + 1].clamp(0.0, 1.0) * 255.0) as u8;
                        buffer[dst_idx + 2] = (rgb_buffer[rgb_idx + 2].clamp(0.0, 1.0) * 255.0) as u8;
                        buffer[dst_idx + 3] = if has_alpha {
                            (alpha_buffer[pixel_idx].clamp(0.0, 1.0) * 255.0) as u8
                        } else {
                            255
                        };
                    }
                }
            }
            PixelFormat::Rgba16 => {
                let required_size = width * height * 8;
                if buffer.len() < required_size {
                    self.error_message = "Buffer too small".to_string();
                    return DecodingStatus::Error;
                }
                let output_slice = unsafe { slice::from_raw_parts_mut(buffer.as_mut_ptr() as *mut u16, width * height * 4) };
                for y in 0..height {
                    for x in 0..width {
                        let pixel_idx = y * width + x;
                        let rgb_idx = pixel_idx * 3;
                        let dst_idx = pixel_idx * 4;
                        output_slice[dst_idx] = (rgb_buffer[rgb_idx].clamp(0.0, 1.0) * 65535.0) as u16;
                        output_slice[dst_idx + 1] = (rgb_buffer[rgb_idx + 1].clamp(0.0, 1.0) * 65535.0) as u16;
                        output_slice[dst_idx + 2] = (rgb_buffer[rgb_idx + 2].clamp(0.0, 1.0) * 65535.0) as u16;
                        output_slice[dst_idx + 3] = if has_alpha {
                            (alpha_buffer[pixel_idx].clamp(0.0, 1.0) * 65535.0) as u16
                        } else {
                            65535
                        };
                    }
                }
            }
            PixelFormat::RgbaF32 => {
                let required_size = width * height * 16;
                if buffer.len() < required_size {
                    self.error_message = "Buffer too small".to_string();
                    return DecodingStatus::Error;
                }
                let output_slice = unsafe { slice::from_raw_parts_mut(buffer.as_mut_ptr() as *mut f32, width * height * 4) };
                for y in 0..height {
                    for x in 0..width {
                        let pixel_idx = y * width + x;
                        let rgb_idx = pixel_idx * 3;
                        let dst_idx = pixel_idx * 4;
                        output_slice[dst_idx] = rgb_buffer[rgb_idx];
                        output_slice[dst_idx + 1] = rgb_buffer[rgb_idx + 1];
                        output_slice[dst_idx + 2] = rgb_buffer[rgb_idx + 2];
                        output_slice[dst_idx + 3] = if has_alpha {
                            alpha_buffer[pixel_idx]
                        } else {
                            1.0
                        };
                    }
                }
            }
            _ => {
                self.error_message = "Unknown pixel format".to_string();
                return DecodingStatus::Error;
            }
        }

        DecodingStatus::Success
    }

    /// Get ICC color profile data.
    fn get_icc_profile(&self) -> &[u8] {
        &self.icc_profile
    }

    /// Get total frame count.
    fn get_frame_count(&self) -> u32 {
        self.frame_count
    }

    /// Check if more frames are available.
    fn has_more_frames(&self) -> bool {
        self.has_more_frames
    }

    /// Get last error message.
    fn get_error(&self) -> &str {
        &self.error_message
    }
}

// =============================================================================
// Public C++ API Functions
// =============================================================================

/// Check if data begins with valid JXL signature.
fn signature_check(data: &[u8]) -> bool {
    if data.len() < 2 {
        return false;
    }
    let bytes = &data[..data.len().min(12)];
    matches!(check_signature(bytes), ProcessingResult::Complete { result: Some(_) })
}

/// Create a new decoder instance.
fn new_decoder() -> Box<Decoder> {
    Box::new(Decoder::new())
}

/// Get library version string.
fn version() -> &'static str {
    "jxl-rs 0.1.0"
}

// =============================================================================
// Default Implementations
// =============================================================================

impl Default for BasicInfo {
    fn default() -> Self {
        Self {
            width: 0,
            height: 0,
            bits_per_sample: 8,
            num_extra_channels: 0,
            has_alpha: false,
            alpha_premultiplied: false,
            have_animation: false,
            animation_loop_count: 0,
            animation_tps_numerator: 1,
            animation_tps_denominator: 1000,
            uses_original_profile: false,
            orientation: 1,
        }
    }
}

impl From<&InternalBasicInfo> for BasicInfo {
    fn from(info: &InternalBasicInfo) -> Self {
        let has_alpha = info.extra_channels.iter().any(|ec| {
            matches!(ec.ec_type, ExtraChannel::Alpha)
        });
        let (animation_loop_count, animation_tps_numerator, animation_tps_denominator) =
            match &info.animation {
                Some(anim) => (anim.num_loops, anim.tps_numerator, anim.tps_denominator),
                None => (0, 1, 1000),
            };
        Self {
            width: info.size.0 as u32,
            height: info.size.1 as u32,
            bits_per_sample: info.bit_depth.bits_per_sample(),
            num_extra_channels: info.extra_channels.len() as u32,
            has_alpha,
            alpha_premultiplied: false,
            have_animation: info.animation.is_some(),
            animation_loop_count,
            animation_tps_numerator,
            animation_tps_denominator,
            uses_original_profile: info.uses_original_profile,
            orientation: info.orientation as u32,
        }
    }
}

impl Default for FrameHeader {
    fn default() -> Self {
        Self {
            duration_ms: 0,
            is_last: false,
            name_length: 0,
        }
    }
}
