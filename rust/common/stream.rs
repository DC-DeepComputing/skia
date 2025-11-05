// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Shared streaming utilities for Rust codecs.
//
// This file provides a macro for implementing Read/Seek adapters that bridge
// C++ StreamAdapter to Rust's std::io traits.
//
// The actual C++ implementation is in StreamAdapter.cpp/.h which provides
// a single shared implementation that all codecs can use.

/// Macro to implement Read and Seek traits for StreamAdapter.
///
/// Codecs must still declare the StreamAdapter type and methods in their
/// CXX bridge (due to CXX limitations), but this macro eliminates duplicating
/// the trait implementations.
///
/// # Usage
///
/// In your FFI.rs:
/// ```rust,ignore
/// use std::pin::Pin;
/// use std::io::{Read, Seek, SeekFrom, ErrorKind};
///
/// include!("../common/stream.rs");
///
/// #[cxx::bridge(namespace = "your_codec")]
/// mod ffi {
///     unsafe extern "C++" {
///         include!("rust/common/StreamAdapter.h");
///         
///         // These declarations are required (CXX limitation):
///         #[namespace = "rust::stream"]
///         type StreamAdapter;
///         fn read(self: Pin<&mut StreamAdapter>, buffer: &mut [u8]) -> usize;
///         fn seek_from_start(...) -> bool;
///         fn seek_from_end(...) -> bool;
///         fn seek_relative(...) -> bool;
///     }
/// }
///
/// // Then use the macro to implement the adapters:
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
