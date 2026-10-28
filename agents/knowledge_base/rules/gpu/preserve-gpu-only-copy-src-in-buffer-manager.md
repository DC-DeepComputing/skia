---
name: skia/gpu/preserve-gpu-only-copy-src-in-buffer-manager
description: When determining the AccessPattern for GPU-only buffers in Graphite buffer managers, do not hardcode AccessPattern::kGpuOnly; preserve AccessPattern::kGpuOnlyCopySrc for tests
rubric: correctness
severity: should_fix
---

**Rule.** When determining the AccessPattern for GPU-only buffers in Graphite
buffer managers, do not unconditionally hardcode AccessPattern::kGpuOnly. Route
pattern selection through access helpers or DrawBufferManager::Options that
return AccessPattern::kGpuOnlyCopySrc when GPU_TEST_UTILS is enabled.

**Why.** Graphite testing infrastructure relies on
AccessPattern::kGpuOnlyCopySrc to add copy-source usage flags to otherwise
private GPU buffers, allowing test fixtures to copy back and inspect buffer
contents during validation. Hardcoding AccessPattern::kGpuOnly bypasses test
configuration and prevents test readbacks from succeeding.

## Examples

Bad:

```cpp
info->fBuffer = fResourceProvider->findOrCreateBuffer(
    bufferSize,
    info->fType,
    (useTransferBuffer || !supportCpuUpload) ? AccessPattern::kGpuOnly :
                                               AccessPattern::kHostVisible,
    std::move(label));
```

Good:

```cpp
info->fBuffer = fResourceProvider->findOrCreateBuffer(
    bufferSize,
    info->fType,
    get_gpu_access_pattern(useTransferBuffer || !supportCpuUpload, fOptions),
    std::move(label));
```
