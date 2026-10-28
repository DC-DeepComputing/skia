---
name: skia/prefer-config-defines-for-third-party-transitions
description: When handling differing third-party library API signatures across version migrations, use explicit compile-time defines in SkUserConfig.h or build flags instead of SFINAE or template introspection
rubric: design
severity: should_fix
---

**Rule.** Do not use SFINAE, std::is_invocable, or template introspection to
detect differing third-party library API signatures across version transitions.
Use explicit compile-time preprocessor configuration defines (documented in
include/config/SkUserConfig.h or build flags) so downstream clients can control
their migration schedule.

**Why.** Template introspection bloats compilation, triggers compiler and linter
warnings, complicates error handling, and obscures which API overload is being
invoked. Compile-time defines in SkUserConfig.h cleanly decouple library
versions and allow embedders (such as Chromium and Android) to transition on
their own schedule before obsolete paths are removed.

## Examples

Bad:

```cpp
// Avoid SFINAE or template introspection to detect third-party API differences:
template <typename F, typename... Args>
auto invoke_if_invocable(F&& f, Args&&... args)
    -> std::enable_if_t<std::is_invocable_v<F, Args...>, decltype(std::invoke(std::forward<F>(f), std::forward<Args>(args)...))> {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

if constexpr (std::is_invocable_v<decltype(&dng_area_task::Start), dng_area_task&, uint32_t,
        const dng_rect&, const dng_point&, dng_memory_allocator*, dng_abort_sniffer*>) {
    invoke_if_invocable(&dng_area_task::Start, task, numTasks, area, tileSize, &Allocator(), Sniffer());
} else {
    invoke_if_invocable(&dng_area_task::Start, task, numTasks, tileSize, &Allocator(), Sniffer());
}
```

Good:

```cpp
// In include/config/SkUserConfig.h:
// #define SK_DNG_VERSION 0x01040000

// In src/codec/SkRawCodec.cpp:
#if SK_DNG_VERSION <= 0x01040000
    task.Start(numTasks, tileSize, &Allocator(), Sniffer());
#else
    task.Start(numTasks, area, tileSize, &Allocator(), Sniffer());
#endif
```
