// Copyright 2025 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! FFI library entry point for clean BMP decoder
//! All decoder logic is consolidated in FFI.rs to avoid module complexity.

// Include the consolidated FFI module
#[path = "FFI.rs"]
mod ffi_impl;

// Re-export everything from the FFI implementation
pub use ffi_impl::*;
