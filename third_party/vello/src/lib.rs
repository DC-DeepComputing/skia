// Copyright 2023 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#[cxx::bridge(namespace = "vello_shaders")]
mod ffi {
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
    extern "Rust" {
        type Shaders;
        fn wgsl() -> &'static Shaders;
        fn msl() -> &'static Shaders;
        fn name(&self, stage: ShaderStage) -> &str;
        fn code(&self, stage: ShaderStage) -> &[u8];
        fn workgroup_size(&self, stage: ShaderStage) -> &[u32; 3];
        fn bindings(&self, stage: ShaderStage) -> Vec<BindType>;
    }
}

use crate::ffi::{BindType, ShaderStage};
use vello_shaders::BindType as VelloBindType;

macro_rules! match_stage_field {
    ($shaders:ident, $stage:ident, $field:tt) => {
        match $stage {
            ShaderStage::Backdrop => &$shaders.0.backdrop.$field,
            ShaderStage::BackdropDyn => &$shaders.0.backdrop_dyn.$field,
            ShaderStage::BboxClear => &$shaders.0.bbox_clear.$field,
            ShaderStage::Binning => &$shaders.0.binning.$field,
            ShaderStage::ClipLeaf => &$shaders.0.clip_leaf.$field,
            ShaderStage::ClipReduce => &$shaders.0.clip_reduce.$field,
            ShaderStage::Coarse => &$shaders.0.coarse.$field,
            ShaderStage::DrawLeaf => &$shaders.0.draw_leaf.$field,
            ShaderStage::DrawReduce => &$shaders.0.draw_reduce.$field,
            ShaderStage::Fine => &$shaders.0.fine.$field,
            ShaderStage::PathCoarse => &$shaders.0.path_coarse.$field,
            ShaderStage::PathCoarseFull => &$shaders.0.path_coarse_full.$field,
            ShaderStage::Pathseg => &$shaders.0.pathseg.$field,
            ShaderStage::PathtagReduce => &$shaders.0.pathtag_reduce.$field,
            ShaderStage::PathtagReduce2 => &$shaders.0.pathtag_reduce2.$field,
            ShaderStage::PathtagScan1 => &$shaders.0.pathtag_scan1.$field,
            ShaderStage::PathtagScanLarge => &$shaders.0.pathtag_scan_large.$field,
            ShaderStage::PathtagScanSmall => &$shaders.0.pathtag_scan_small.$field,
            ShaderStage::TileAlloc => &$shaders.0.tile_alloc.$field,
            _ => unreachable!(),
        }
    }
}

struct Shaders(&'static vello_shaders::Shaders<'static>);

impl Shaders {
    fn name(&self, stage: ShaderStage) -> &str {
        match_stage_field!(self, stage, name)
    }

    fn code(&self, stage: ShaderStage) -> &[u8] {
        match_stage_field!(self, stage, code)
    }

    fn workgroup_size(&self, stage: ShaderStage) -> &[u32; 3] {
        match_stage_field!(self, stage, workgroup_size)
    }

    fn bindings(&self, stage: ShaderStage) -> Vec<BindType> {
        match_stage_field!(self, stage, bindings).iter().map(|t| t.into()).collect()
    }
}

const WGSL_SHADERS: Shaders = Shaders(&vello_shaders::wgsl::SHADERS);
const MSL_SHADERS: Shaders = Shaders(&vello_shaders::msl::SHADERS);

fn wgsl() -> &'static Shaders {
    &WGSL_SHADERS
}

fn msl() -> &'static Shaders {
    &MSL_SHADERS
}

impl From<&VelloBindType> for BindType {
    fn from(src: &VelloBindType) -> Self {
        match src {
            VelloBindType::Buffer => Self::Buffer,
            VelloBindType::BufReadOnly=> Self::BufReadOnly,
            VelloBindType::Uniform => Self::Uniform,
            VelloBindType::Image => Self::Image,
            VelloBindType::ImageRead => Self::ImageRead,
        }
    }
}
