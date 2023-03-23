// Copyright 2023 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

fn main() {
    cxx_build::bridge("src/lib.rs")
        .flag_if_supported("-std=c++14")
        .compile("vello_shaders_cpp");
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=include/");
}
