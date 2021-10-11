/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu_cpp.h>

using namespace emscripten;

const char* triangleVertWGSL = R"(
[[stage(vertex)]]
fn main([[builtin(vertex_index)]] VertexIndex : u32)
     -> [[builtin(position)]] vec4<f32> {
  var pos = array<vec2<f32>, 3>(
      vec2<f32>(0.0, 0.5),
      vec2<f32>(-0.5, -0.5),
      vec2<f32>(0.5, -0.5));

  return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
}
)";

const char* redFragWGSL = R"(
[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
  return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}
)";

wgpu::ShaderModule createShaderModule(wgpu::Device device, const char* source) {
        // https://github.com/emscripten-core/emscripten/blob/da842597941f425e92df0b902d3af53f1bcc2713/system/include/webgpu/webgpu_cpp.h#L1415
    wgpu::ShaderModuleWGSLDescriptor wDesc;
    wDesc.source = triangleVertWGSL;
    wgpu::ShaderModuleDescriptor desc = {.nextInChain = &wDesc};
    return device.CreateShaderModule(&desc);
}

void doSomething() {
    // https://github.com/emscripten-core/emscripten/blob/da842597941f425e92df0b902d3af53f1bcc2713/system/include/webgpu/webgpu_cpp.h#L922
    wgpu::Device device = wgpu::Device::Acquire(emscripten_webgpu_get_device());

    wgpu::ShaderModule triangleVertShader = createShaderModule(device, triangleVertWGSL);
    wgpu::ShaderModule redFragShader = createShaderModule(device, redFragWGSL);

    wgpu::ColorTargetState colorTargetState{};
    colorTargetState.format = wgpu::TextureFormat::BGRA8Unorm;

    wgpu::FragmentState fragmentState{};
    fragmentState.module = redFragShader;
    fragmentState.entryPoint = "main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    wgpu::PipelineLayoutDescriptor pl{};

    // Inspired by https://github.com/kainino0x/webgpu-cross-platform-demo/blob/4061dd13096580eb5525619714145087b0d5acf6/main.cpp#L129
    wgpu::RenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.layout = device.CreatePipelineLayout(&pl);
    pipelineDescriptor.vertex.module = triangleVertShader;
    pipelineDescriptor.vertex.entryPoint = "main";
    pipelineDescriptor.fragment = &fragmentState;
    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    printf("device %p\n", device.Get());
    printf("pipeline %p\n", pipeline.Get());

    wgpu::SurfaceDescriptorFromCanvasHTMLSelector selector;
    selector.selector = "#webgpu-demo-canvas";

    wgpu::SurfaceDescriptor surface_desc;
    surface_desc.nextInChain = &selector;
    wgpu::Instance instance;
    wgpu::Surface surface = instance.CreateSurface(&surface_desc);

    printf("surface %p\n", surface.Get());
}

EMSCRIPTEN_BINDINGS(Skia) {
    function("doSomething", &doSomething);
}
