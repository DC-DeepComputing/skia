// Copyright 2024 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chromium::import! {
    "//third_party/skia/experimental/rust_png:sk_stream_rust_bindings";
}

use rust_gtest_interop::prelude::*;
use sk_stream_rust_bindings::ffi::{SkMemoryStreamMakeCopy, SkStream};
use std::io::Read;

/// Test coverage for `impl Read for Pin<&mut ffi::SkStream>`.
#[gtest(SkStreamRustBindingsTest, ReadImpl)]
fn read_impl() {
    let mut s = SkMemoryStreamMakeCopy(b"foobar");
    let mut output = Vec::new();
    let read_bytes = s.pin_mut().read_to_end(&mut output).unwrap();
    assert_eq!(read_bytes, 6);
    assert_eq!(&output, b"foobar");
}
