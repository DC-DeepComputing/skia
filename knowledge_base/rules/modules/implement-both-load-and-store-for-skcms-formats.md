---
name: skia/modules/implement-both-load-and-store-for-skcms-formats
description: When adding support for a new skcms_PixelFormat, implement both load (Op::load_*) and store (Op::store_*) transform operations rather than supporting only one direction
rubric: correctness
severity: should_fix
---

**Rule.** When adding support for a new pixel format (skcms_PixelFormat) in
skcms, implement both the load stage (Op::load_*) and the store stage
(Op::store_*) in the transform pipeline. Do not introduce a pixel format that
only supports one-way conversion without corresponding source loading support.

**Why.** In skcms, pixel formats configured in skcms_Transform are expected to
function symmetrically as both source decode buffers and destination render
targets. Supporting only a store stage causes transform pipeline creation to
fail or fall back unexpectedly when converting from that format.

## Examples

Bad:

```cpp
// In skcms: only implementing store without corresponding load stage
case skcms_PixelFormat_RGB_101010x_XR >> 1:
    add_op(Op::store_101010x_XR);
    break;
```

Good:

```cpp
// In skcms: implementing both load and store transform ops
case skcms_PixelFormat_RGB_101010x_XR >> 1:
    add_op(Op::load_101010x_XR);
    break;
...
case skcms_PixelFormat_RGB_101010x_XR >> 1:
    add_op(Op::store_101010x_XR);
    break;
```
