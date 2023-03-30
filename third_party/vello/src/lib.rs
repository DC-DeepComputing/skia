// Copyright 2023 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#[cxx::bridge(namespace = "vello_cpp")]
mod ffi {
    // === vello_shaders FFI types ===

    enum ShaderStage {
        Backdrop,
        BackdropDyn,
        BboxClear,
        Binning,
        ClipLeaf,
        ClipReduce,
        Coarse,
        DrawLeaf,
        DrawReduce,
        Fine,
        PathCoarse,
        PathCoarseFull,
        Pathseg,
        PathtagReduce,
        PathtagReduce2,
        PathtagScan1,
        PathtagScanLarge,
        PathtagScanSmall,
        TileAlloc,
    }

    enum BindType {
        Buffer,
        BufReadOnly,
        Uniform,
        Image,
        ImageRead,
    }

    struct WorkgroupBufferInfo {
        size_in_bytes: u32,
        index: u32,
    }

    // === vello_encoding FFI types ===

    // Represents a 3x3 affine transformation matrix. The coordinates are laid out using the
    // following column-major construction, where the entries
    //
    //    [a, b, c, d, e, f]
    //
    // correspond to the matrix
    //
    //    | a c e |
    //    | b d f |
    //    | 0 0 1 |
    //
    struct Affine {
        matrix: [f32; 6],
    }

    struct Color {
        r: u8,
        g: u8,
        b: u8,
        a: u8,
    }

    enum BrushKind {
        Solid,
    }

    struct BrushData {
        solid: Color,
    }

    struct Brush {
        kind: BrushKind,
        data: BrushData,
    }

    enum Fill {
        NonZero,
        EvenOdd,
    }

    #[derive(Copy, Clone, Default)]
    struct Point {
        x: f32,
        y: f32,
    }

    enum PathVerb {
        MoveTo,
        LineTo,
        QuadTo,
        CurveTo,
        Close,
    }

    #[derive(Default)]
    struct PathElement {
        verb: PathVerb,
        points: [Point; 3],
    }

    extern "Rust" {
        type Shaders;
        fn wgsl() -> &'static Shaders;
        fn msl() -> &'static Shaders;
        fn name(self: &Shaders, stage: ShaderStage) -> &str;
        fn code(self: &Shaders, stage: ShaderStage) -> &[u8];
        fn workgroup_size(self: &Shaders, stage: ShaderStage) -> &[u32; 3];
        fn bindings(self: &Shaders, stage: ShaderStage) -> Vec<BindType>;
        fn workgroup_buffers(self: &Shaders, stage: ShaderStage) -> Vec<WorkgroupBufferInfo>;

        type Encoding;
        fn new_encoding() -> Box<Encoding>;
        fn fill(self: &mut Encoding,
                style: Fill,
                transform: Affine,
                brush: &Brush,
                mut path_iter: Pin<&mut PathIterator>);
    }

    unsafe extern "C++" {
        include!("vello_cpp/include/vello_cpp/path_iterator.h");

        type PathIterator;
        unsafe fn next_element(self: Pin<&mut PathIterator>, out_elem: *mut PathElement) -> bool;
    }
}

mod encoding;
mod shaders;

use {
    encoding::{Encoding, new_encoding},
    shaders::{Shaders, msl, wgsl},
};
