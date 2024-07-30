// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! This crate provides Rust bindings for the `SkStream` C++ class.  Currently
//! this simply translates into providing implementation of Rust's `Read` trait.

use std::io::{Read, Result};
use std::pin::Pin;

#[cxx::bridge]
pub mod ffi {
    unsafe extern "C++" {
        include!("experimental/rust_png/SkStreamFFI.h");

        type SkStream;
        fn SkStreamRead(stream: Pin<&mut SkStream>, buffer: &mut [u8]) -> usize;

        fn SkMemoryStreamMakeCopy(data: &[u8]) -> UniquePtr<SkStream>;
    }
}

impl Read for Pin<&mut ffi::SkStream> {
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        Ok(ffi::SkStreamRead(self.as_mut(), buf))
    }
}
