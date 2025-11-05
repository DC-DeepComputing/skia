// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Common I/O trait implementations for Rust codecs.
//
// This module provides Read and Seek trait implementations for StreamAdapter,
// which is a C++ type that wraps SkStream. Each codec declares StreamAdapter
// in its own CXX bridge with namespace="rust::stream", and CXX's type
// unification ensures they all refer to the same C++ type.
//
// Note: This file is included via `include!()` into codec FFI modules.
// The including module must have imported: std::io::*, std::pin::Pin

/// Macro to implement Read and Seek traits for StreamAdapter.
///
/// Usage in your codec's FFI.rs:
/// ```rust,ignore
/// use std::io::{ErrorKind, Read, Seek, SeekFrom};
/// use std::pin::Pin;
///
/// include!("../common/io_traits_ffi.rs");
///
/// #[cxx::bridge(namespace = "your_codec")]
/// mod ffi {
///     unsafe extern "C++" {
///         include!("rust/common/SkStreamAdapter.h");
///         
///         // Declare StreamAdapter with rust::stream namespace for type unification
///         #[namespace = "rust::stream"]
///         type StreamAdapter;
///         fn read(self: Pin<&mut StreamAdapter>, buffer: &mut [u8]) -> usize;
///         fn seek_from_start(
///             self: Pin<&mut StreamAdapter>,
///             requested_pos: u64,
///             final_pos: &mut u64,
///         ) -> bool;
///         fn seek_from_end(
///             self: Pin<&mut StreamAdapter>,
///             requested_offset: i64,
///             final_pos: &mut u64,
///         ) -> bool;
///         fn seek_relative(
///             self: Pin<&mut StreamAdapter>,
///             requested_offset: i64,
///             final_pos: &mut u64,
///         ) -> bool;
///     }
/// }
///
/// // Then use the macro:
/// impl_stream_adapter!(ffi::StreamAdapter);
/// ```
macro_rules! impl_stream_adapter {
    ($ffi_type:ty) => {
        impl<'a> Read for Pin<&'a mut $ffi_type> {
            fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
                let bytes_read = self.as_mut().read(buf);
                Ok(bytes_read)
            }
        }

        impl<'a> Seek for Pin<&'a mut $ffi_type> {
            fn seek(&mut self, pos: SeekFrom) -> std::io::Result<u64> {
                let mut final_pos: u64 = 0;
                let success = match pos {
                    SeekFrom::Start(requested_pos) => {
                        self.as_mut()
                            .seek_from_start(requested_pos, &mut final_pos)
                    }
                    SeekFrom::End(requested_offset) => {
                        self.as_mut()
                            .seek_from_end(requested_offset, &mut final_pos)
                    }
                    SeekFrom::Current(requested_offset) => {
                        self.as_mut()
                            .seek_relative(requested_offset, &mut final_pos)
                    }
                };
                if success {
                    Ok(final_pos)
                } else {
                    Err(ErrorKind::Other.into())
                }
            }
        }
    };
}
