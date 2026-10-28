---
name: skia/gpu/use-skgpu-std-steady-clock
description: When measuring elapsed time, recording timestamps, or calculating durations in GPU code, use skgpu::StdSteadyClock instead of std::chrono::steady_clock
rubric: readability
severity: should_fix
---

**Rule.** In GPU code under src/gpu/, use skgpu::StdSteadyClock (along with its
member types and methods like time_point, duration, and now()) instead of
referencing std::chrono::steady_clock directly.

**Why.** skgpu::StdSteadyClock (defined in src/gpu/GpuTypesPriv.h) provides
portability across standard library implementations (handling older libstdc++
releases where steady_clock was monotonic_clock) and ensures consistency across
Ganesh and Graphite cache management and performance timing.

## Examples

Bad:

```cpp
#include <chrono>

struct PipelineCreation {
    std::chrono::steady_clock::time_point fStartTime;

    void record() {
        auto elapsed = std::chrono::steady_clock::now() - fStartTime;
    }
};
```

Good:

```cpp
#include "src/gpu/GpuTypesPriv.h"

struct PipelineCreation {
    skgpu::StdSteadyClock::time_point fStartTime;

    void record() {
        skgpu::StdSteadyClock::duration elapsed = skgpu::StdSteadyClock::now() - fStartTime;
    }
};
```
