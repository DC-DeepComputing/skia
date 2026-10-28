---
name: skia/codec/assert-platform-type-size-assumptions
description: When code relies on platform type-size assumptions like sizeof(int) >= sizeof(int32_t) during integer conversions, guard the invariant with static_assert
rubric: correctness
severity: should_fix
---

**Rule.** When code relies on platform-dependent type size assumptions (such as
assuming sizeof(int) >= sizeof(int32_t) when assigning or returning int32_t as
int), guard the assumption with a static_assert. Phrase the diagnostic message
as the required invariant or failure condition.

**Why.** In C++, the language standard only guarantees that int is at least 16
bits. Implicitly converting or returning fixed-width integers like int32_t as
int relies on platform-specific size guarantees, and static_assert verifies this
assumption at compile time across unconventional target architectures.

## Examples

Bad:

```cpp
int SkPngRustCodec::getRawFrameCount() const {
    if (!fReader->has_actl_chunk()) {
        return 1;
    }
    uint32_t num_frames = fReader->get_actl_num_frames();
    // Bad: Assumes sizeof(int) >= sizeof(int32_t) without compile-time verification
    return Sk64_pin_to_s32(num_frames);
}
```

Good:

```cpp
int SkPngRustCodec::getRawFrameCount() const {
    if (!fReader->has_actl_chunk()) {
        return 1;
    }
    // Good: Explicit compile-time assertion guarding platform type-size assumption
    static_assert(sizeof(int) >= sizeof(int32_t), "int must be at least 32 bits for Sk64_pin_to_s32");
    uint32_t num_frames = fReader->get_actl_num_frames();
    return Sk64_pin_to_s32(num_frames);
}
```
