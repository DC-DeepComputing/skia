// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Common FFI bridge for I/O traits shared across Rust codecs.
//!
//! This module defines the canonical CXX bridge declarations for StreamAdapter
//! and WriteAdapter. Other codec modules (PNG, BMP, JPEG, etc.) should reference
//! these types rather than declaring their own versions, to ensure type compatibility
//! across codec boundaries.

use std::pin::Pin;

#[cxx::bridge(namespace = "rust::stream")]
pub mod ffi {
    unsafe extern "C++" {
        include!("rust/common/SkRustIOAdaptersInterfaces.h");
        include!("rust/common/SkRustIOAdapters.h");

        /// Adapter for reading and seeking from C++ SkStream.
        /// Provides Rust's `Read + Seek` trait equivalents for Skia streams.
        type StreamAdapter;
        
        /// Read data from the stream into the provided buffer.
        /// Returns the number of bytes actually read.
        fn read(self: Pin<&mut StreamAdapter>, buffer: &mut [u8]) -> usize;
        
        /// Seek to an absolute position from the start of the stream.
        /// Returns true on success and sets final_pos to the new position.
        fn seek_from_start(
            self: Pin<&mut StreamAdapter>,
            requested_pos: u64,
            final_pos: &mut u64,
        ) -> bool;
        
        /// Seek relative to the end of the stream.
        /// Returns true on success and sets final_pos to the new position.
        fn seek_from_end(
            self: Pin<&mut StreamAdapter>,
            requested_offset: i64,
            final_pos: &mut u64,
        ) -> bool;
        
        /// Seek relative to the current position.
        /// Returns true on success and sets final_pos to the new position.
        fn seek_relative(
            self: Pin<&mut StreamAdapter>,
            requested_offset: i64,
            final_pos: &mut u64,
        ) -> bool;

        /// Adapter for writing to C++ SkWStream.
        /// Provides Rust's `Write` trait equivalent for Skia streams.
        type WriteAdapter;
        
        /// Write data from buffer to the stream.
        /// Returns true if all bytes were written successfully.
        fn write(self: Pin<&mut WriteAdapter>, buffer: &[u8]) -> bool;
        
        /// Flush any buffered data to the underlying stream.
        fn flush(self: Pin<&mut WriteAdapter>);
    }
}
