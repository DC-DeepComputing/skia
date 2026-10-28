---
name: skia/rust/avoid-copying-input-buffers-in-ffi-results
description: When returning parsed metadata or results from Rust FFI functions, avoid allocating and returning duplicate copies of input byte buffers
rubric: safety
severity: should_fix
---

**Rule.** Do not allocate and return an owned copy of the input byte buffer
(such as Vec<u8>) in Rust FFI result structs or out-parameters. Expose only
parsed metadata, transformed structures, or views, as callers already own the
underlying buffer (such as sk_sp<SkData>) and can retain or duplicate it if
needed without forced heap allocations.

**Why.** Returning an owned Vec<u8> copy of an input byte slice forces an
unnecessary heap allocation and redundant memory copy on every invocation. C++
callers typically already retain the input data in an owned buffer (such as
sk_sp<SkData>) and only require parsed metadata or conversion results, making
duplicate buffer copies wasteful.

## Examples

Bad:

```cpp
#[cxx::bridge]
mod ffi {
    struct IccProfileResult {
        data_color_space: ColorSpace,
        rendering_intent: RenderingIntent,
        profile_bytes: Vec<u8>,
    }

    extern "Rust" {
        fn parse_icc_profile(data: &[u8]) -> IccProfileResult;
    }
}

// Redundantly copies caller-owned input buffer onto the heap
fn parse_icc_profile(data: &[u8]) -> IccProfileResult {
    IccProfileResult {
        data_color_space: parse_cs(data),
        rendering_intent: parse_intent(data),
        profile_bytes: data.to_vec(),
    }
}
```

Good:

```cpp
#[cxx::bridge]
mod ffi {
    struct IccProfile {
        data_color_space: ColorSpace,
        rendering_intent: RenderingIntent,
    }

    extern "Rust" {
        fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool;
    }
}

// Returns parsed metadata only; caller already owns the input data
fn parse_icc_profile(data: &[u8], out: &mut IccProfile) -> bool {
    out.data_color_space = parse_cs(data);
    out.rendering_intent = parse_intent(data);
    true
}
```
