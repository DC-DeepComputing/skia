---
name: skia/gpu/account-for-alignment-padding-when-sizing-buffers
description: When calculating total capacity to pack multiple sub-allocations into a buffer, align the running total before adding each reservation size
rubric: correctness
severity: should_fix
---

**Rule.** When calculating the total size required to pack multiple
sub-allocations into a single buffer, advance the running total by aligning it
to the reservation's required alignment before adding its size (e.g.,
align(total, req_alignment) + size) rather than accumulating raw sizes. Ensure
the alignment calculation accounts for both element-specific alignment
constraints and any minimum buffer binding alignments.

**Why.** When packing multiple sub-allocations with alignment requirements into
a single target buffer, inter-element padding is required so each allocation
begins at a properly aligned offset. Merely summing raw sizes underestimates
required buffer capacity, leading to buffer overruns, failed copy operations, or
assertion failures when data is placed at aligned offsets.

## Examples

Bad:

```cpp
// Flawed: accumulating raw sizes ignores required inter-element alignment padding
for (const auto& req : reservations) {
    fData.push_back({req.fBinding, req.fTarget});
    fTotalRequiredBytes += req.fSize;
}
```

Good:

```cpp
// Correct: align running total to element and buffer requirements before adding size
for (const auto& req : reservations) {
    fData.push_back({req.fBinding, req.fTarget, req.fAlignment});
    const uint32_t alignment = BufferAligner::LcmAlignment(fMinimumAlignment, req.fAlignment);
    fTotalRequiredBytes = SkAlignNonPow2(fTotalRequiredBytes, alignment) + req.fSize;
}
```
