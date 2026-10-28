---
name: skia/gpu/pass-device-to-encoder-methods
description: When creating transient helper or encoder objects, pass backend device references directly to execution methods rather than embedding them as member variables
rubric: design
severity: should_fix
---

**Rule.** Do not store or embed backend device or context references (such as
wgpu::Device or SharedContext) as member variables inside transient helper or
encoder objects. When the caller already owns or accesses the device, pass the
device reference directly into the specific execution or encode method instead.

**Why.** Embedding device references in transient helpers increases object size,
introduces unnecessary lifetime and ownership coupling, and restricts helper
reuse. Passing the device directly to the execution method keeps helper objects
lightweight, stateless, and focused strictly on configuration and pipeline
state.

## Examples

Bad:

```cpp
class BlitWithDrawEncoder {
public:
    BlitWithDrawEncoder(const wgpu::Device& device,
                        wgpu::RenderPipeline pipeline,
                        bool srcIsMSAA)
            : fDevice(device), fPipeline(std::move(pipeline)), fSrcIsMSAA(srcIsMSAA) {}

    void EncodeBlit(const wgpu::RenderPassEncoder& encoder, ...);
private:
    const wgpu::Device& fDevice;
    wgpu::RenderPipeline fPipeline;
    const bool fSrcIsMSAA;
};
```

Good:

```cpp
class BlitWithDrawEncoder {
public:
    BlitWithDrawEncoder(wgpu::RenderPipeline pipeline,
                        bool srcIsMSAA)
            : fPipeline(std::move(pipeline)), fSrcIsMSAA(srcIsMSAA) {}

    void EncodeBlit(const wgpu::Device& device,
                    const wgpu::RenderPassEncoder& encoder, ...);
private:
    wgpu::RenderPipeline fPipeline;
    const bool fSrcIsMSAA;
};
```
