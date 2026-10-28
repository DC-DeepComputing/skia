---
name: skia/modules/name-unpacked-canvaskit-buffer-pointers-by-consumed-count
description: In CanvasKit bindings, unpack WASM buffer inputs with helpers or exact-count pointers, and write output buffers via a single memcpy
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit WebAssembly bindings, use existing pointer conversion
helpers (such as ptrToSkColor4f in WasmCommon.h) rather than manually casting
WASMPointer types and unpacking individual elements. When manual buffer
unpacking is necessary, name local buffer pointers (e.g., via
reinterpret_cast<float*>) after the number of elements consumed by the function
(e.g., twoFloats for an SkV2) rather than the buffer's allocated capacity.
Conversely, when writing POD types (such as SkColor4f or SkV3) into an output
WASMPointer across conditional branches, populate a local struct across branches
and copy to the destination pointer with a single memcpy rather than repeating
element-by-element buffer assignments in each branch.

**Why.** Shared WASM conversion helpers and accurate pointer names clarify data
contracts, avoid redundant pointer casting, and prevent misleading assumptions
about buffer sizes. Consolidating output writes into a local struct followed by
a single memcpy eliminates duplicated indexing across branches, centralizes raw
pointer writes, and avoids element transposition mistakes.

## Examples

Bad:

```cpp
// Bad: Manually casting and unpacking when a helper exists
float* fourFloats = reinterpret_cast<float*>(cPtr);
SkColor4f color = { fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3] };
return self.setColor(key, color.toSkColor());

// Bad: Naming unpacked pointer after buffer capacity rather than consumed elements
float* threeFloats = reinterpret_cast<float*>(vPtr);
SkV2 vec2 = { threeFloats[0], threeFloats[1] };

// Bad: Duplicating element-by-element output buffer writes across branches
float* fourFloats = reinterpret_cast<float*>(outPtr);
if (auto c = fSlotMgr->getColorSlot(SkString(slotID))) {
    auto c4f = SkColor4f::FromColor(*c);
    fourFloats[0] = c4f.fR; fourFloats[1] = c4f.fG; fourFloats[2] = c4f.fB; fourFloats[3] = c4f.fA;
} else {
    fourFloats[0] = -1; fourFloats[1] = -1; fourFloats[2] = -1; fourFloats[3] = -1;
}
```

Good:

```cpp
// Good: Using existing ptrToSkColor4f helper from WasmCommon.h
SkColor4f color = ptrToSkColor4f(cPtr);
return self.setColorSlot(key, color.toSkColor());

// Good: Naming manually unpacked pointer by the number of elements consumed
float* twoFloats = reinterpret_cast<float*>(vPtr);
SkV2 vec2 = { twoFloats[0], twoFloats[1] };

// Good: Populating local struct across branches and copying once with memcpy
SkColor4f c4f;
if (auto c = fSlotMgr->getColorSlot(SkString(slotID))) {
    c4f = SkColor4f::FromColor(*c);
} else {
    c4f = {-1, -1, -1, -1};
}
memcpy(reinterpret_cast<float*>(outPtr), &c4f, sizeof(c4f));
```
