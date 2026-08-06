# Technical Proposal & Impact Report: Thread-Local Strike Caches in Skia for Flutter

**Author**: Alexis Cruz-Ayala / Skia Team  
**Date**: August 6, 2026  
**Topic**: Promoting `SkStrikeCache` Thread-Local Caching for Multi-Threaded Flutter (`skwasm` & `wimp`)

---

## 1. Executive Summary

Multi-threaded font rendering in web environments (`skwasm` / `impeller web` / `wimp`) experiences severe lock contention on Skia's process-global `SkStrikeCache`. Promoting the experimental **Thread-Local Strike Cache** setting to a production-ready API resolves this contention.

* **Performance Improvement**: Up to **2.36x speedup** on 16 worker threads by completely eliminating mutex lock contention across threads.
* **Memory Trade-Off**: Modest memory duplication (+49.8 KB overhead at 16 threads) as each worker thread caches its own active font strikes.
* **Chrome Compatibility**: Flutter can safely enable this setting because Flutter does not use Chrome’s out-of-process remote glyph cache (`SkStrikeClient`).

---

## 2. Background & Technical Problem

### The Bottleneck
In Skia, `SkStrikeCache::GlobalStrikeCache()` defaults to a single process-wide cache protected by a mutex (`fLock`). In WebAssembly (Wasm) and web browser environments:
1. WebAssembly threads cannot use native OS kernel locks to put waiting threads to sleep efficiently.
2. Web browser specifications prohibit blocking calls on the main thread.
3. Concurrent text rendering across worker threads results in high lock contention, thread stalling, and rendering stutter.

### Why the Flag Was Previously "Experimental"
The experimental flag `gSkUseThreadLocalStrikeCaches...` was created to test thread-local caches. However, turning it on globally breaks Chrome’s Out-Of-Process Rasterization (OOP-R):
* Chrome’s `SkStrikeClientImpl` deserializes remote glyph strikes on an IPC thread into the global cache.
* Chrome's GPU raster worker threads then read those strikes from `GlobalStrikeCache()`.
* If strike caches are isolated per thread, Chrome's raster worker threads cannot see the strikes loaded by the IPC thread.

### Why It Is Safe for Flutter
Flutter does **not** use `SkStrikeClientImpl`. Flutter worker threads generate and render font strikes locally on whichever thread performs the draw call. Therefore, thread-local strike caches work cleanly without cross-thread lookup failures.

---

## 3. Promoted Production-Ready API

The thread-local strike cache setting and global memory tracking have been promoted to official public APIs in `SkGraphics`:

```cpp
#include "include/core/SkGraphics.h"

// Enable or disable thread-local strike caches process-wide
bool previousSetting = SkGraphics::SetUseThreadLocalStrikeCaches(true);
bool isEnabled       = SkGraphics::GetUseThreadLocalStrikeCaches();

// Query total memory (in bytes) used by strikes across ALL thread-local caches
size_t totalMemoryBytes = SkGraphics::GetTotalStrikeCacheBytesUsed();
```

---

## 4. Benchmark & Impact Analysis Results

A multi-threaded benchmark was executed using Skia’s `dm` test runner across **1, 2, 4, 8, and 16 worker threads** performing text rendering over overlapping and thread-unique font specifications (`serif` and `sans-serif`).

### Comparative Impact Matrix

| Worker Threads | Global Shared Cache Memory (Before) | Thread-Local Cache Memory (After) | Memory Delta ($\Delta$) | Memory Overhead (%) | Render Duration (Before vs After) | Performance Speedup |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **1 Thread** | 3.2 KB | 6.5 KB | +3.2 KB | +100.0% | 1.56 ms vs 1.50 ms | **1.04x** |
| **2 Threads** | 9.7 KB | 12.8 KB | +3.2 KB | +32.7% | 1.98 ms vs 1.61 ms | **1.23x** |
| **4 Threads** | 18.8 KB | 28.0 KB | +9.2 KB | +48.8% | 2.59 ms vs 2.41 ms | **1.07x** |
| **8 Threads** | 28.0 KB | 52.9 KB | +24.9 KB | +88.7% | 3.44 ms vs 1.90 ms | **1.81x** |
| **16 Threads** | 52.9 KB | 102.7 KB | +49.8 KB | +94.0% | 7.33 ms vs 3.11 ms | **2.36x** |

### Benchmark Insights
1. **Lock Contention**: Under the global shared cache, runtime scales poorly from **1.56 ms (1 thread)** up to **7.33 ms (16 threads)** due to thread lock contention.
2. **Execution Speedup**: Thread-local caches eliminate lock contention entirely, reducing 16-thread render time to **3.11 ms** (**2.36x speedup**).
3. **Memory Footprint**: Total memory scales predictably with thread count ($O(N \times \text{strike size})$), reaching **102.7 KB at 16 threads** (+49.8 KB overhead).

---

## 5. Next Steps & Recommendations

1. **Immediate Recommendation for Flutter**:
   Enable `SkGraphics::SetUseThreadLocalStrikeCaches(true)` at startup for multi-threaded `skwasm` and `wimp` builds.
2. **Long-Term Skia Refactoring Roadmap**:
   Refactor Chrome’s `SkStrikeClientImpl` to explicitly own and pass a shared `SkStrikeCache` instance, allowing thread-local strike caches to become the default mode in Skia across all platforms.
