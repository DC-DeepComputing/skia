---
name: skia/codec/hoist-loop-invariant-decode-setup
description: In image decoders, hoist loop-invariant accessor calls, row stride calculations, buffer allocations, and swizzler setup outside row-decoding and progressive loops
rubric: safety
severity: should_fix
---

**Rule.** In image decoders, hoist loop-invariant setup operations, assertions,
accessor calls, and scratch buffer allocations outside progressive, interlaced,
or row-decoding loops. Do not declare or resize heap-managed scratch containers
(such as std::vector or scanline buffers) inside the loop body; hoist container
declarations and sizing outside the loop to reuse storage across iterations.
Similarly, hoist invariant accessor calls (such as getEncodedRowBytes()),
swizzler initialization (initializeSwizzler), and storage allocations
(allocateStorage) ahead of decode loops when decoding parameters remain
constant.

**Why.** Declaring or resizing heap-managed containers inside loops causes
repeated memory allocations and deallocations on every iteration. Calling
invariant member accessors or re-initializing swizzlers across row iterations
introduces unnecessary CPU overhead and obscures the loop-invariant nature of
the value.

## Examples

Bad:

```cpp
// 1. Temporary heap buffer declared and resized inside row decode loop
while (true) {
    std::vector<uint8_t> fullWidthRow;
    if (interlaced) {
        fullWidthRow.resize(this->getEncodedRowBytes());
        dstSlice = rust::Slice<uint8_t>(fullWidthRow);
    }
    // ... decode pass ...
}

// 2. Loop-invariant accessor call re-evaluated inside per-row loop
for (int i = 0; i < this->options().fSubset->height(); ++i) {
    const size_t encodedRowBytes = this->getEncodedRowBytes();
    SkSpan<const uint8_t> srcRow = fullImageBuffer.first(encodedRowBytes);
    fullImageBuffer = fullImageBuffer.subspan(encodedRowBytes);
}

// 3. Decoder setup and storage allocation inside multi-pass decode loop
while (!jpeg_input_complete(dinfo)) {
    this->initializeSwizzler(dstInfo, options, true);
    if (!this->allocateStorage(dstInfo)) return kInternalError;
    this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
}
```

Good:

```cpp
// 1. Hoist scratch buffer declaration and sizing outside decode loop
std::vector<uint8_t> fullWidthRow;
if (interlaced) {
    fullWidthRow.resize(this->getEncodedRowBytes());
    dstSlice = rust::Slice<uint8_t>(fullWidthRow);
}
while (true) {
    // ... decode pass reusing fullWidthRow ...
}

// 2. Hoist invariant accessor call outside row loop
const size_t encodedRowBytes = this->getEncodedRowBytes();
for (int i = 0; i < this->options().fSubset->height(); ++i) {
    SkSpan<const uint8_t> srcRow = fullImageBuffer.first(encodedRowBytes);
    fullImageBuffer = fullImageBuffer.subspan(encodedRowBytes);
}

// 3. Hoist invariant swizzler setup and storage allocation outside loop
this->initializeSwizzler(dstInfo, options, true);
if (!this->allocateStorage(dstInfo)) return kInternalError;

while (!jpeg_input_complete(dinfo)) {
    this->readRows(dstInfo, dst, dstRowBytes, count, options, &rows);
}
```
