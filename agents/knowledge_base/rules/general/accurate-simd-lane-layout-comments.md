---
name: skia/accurate-simd-lane-layout-comments
description: When commenting SIMD lane layouts or vector loads, verify intrinsic element ordering and explicitly describe zero-padding rather than ambiguous lane expansion
rubric: readability
severity: should_fix
---

**Rule.** When annotating SIMD shuffle, interleave, or repack routines with
inline comments showing vector register and lane layouts, verify that the
annotated element and byte order accurately match intrinsic behavior, accounting
for argument ordering, bitwise shifts, and endianness. Additionally, when
documenting integer widening or zero-extension into SIMD vector lanes,
explicitly describe that unused lanes are padded with zeros (noting the exact
lane layout) rather than using ambiguous phrases like 'expand to N lanes' that
can be misread as broadcasting or splatting.

**Why.** Inaccurate lane layout comments obscure channel-inversion or
element-ordering bugs and mislead maintainers tracing vector register states.
Furthermore, casting a narrower scalar or packed value to a wider integer before
vector loading leaves high lanes zero-extended rather than replicated; ambiguous
comments can lead readers to mistakenly assume values are broadcast across all
lanes.

## Examples

Bad:

```cpp
// 1. Shuffling/interleaving halfwords incorrectly commented as producing rgba:
rg = __lsx_vor_v(r, __lsx_vslli_h(g, 8));            // rgrgrgrg RGRGRGRG
ba = __lsx_vor_v(b, __lsx_vslli_h(a, 8));            // babababa BABABABA
lo = __lsx_vilvl_h(ba, rg);                          // rgbargba rgbargba

// 2. Ambiguous comment implying broadcast/splatting instead of zero-extension:
uint64_t x4 = (uint64_t)x;                           // expand out to four lanes
return from_half(skvx::half4::Load(&x4));
```

Good:

```cpp
// 1. Accurately document byte/halfword ordering from shifts and interleaves:
bg = __lsx_vor_v(b, __lsx_vslli_h(g, 8));            // bgbg bgbg BGBG BGBG
ra = __lsx_vor_v(r, __lsx_vslli_h(a, 8));            // rara rara RARA RARA
lo = __lsx_vilvl_h(ra, bg);                          // bgra bgra bgra bgra

// 2. Explicitly note zero-padding and specific lane layout:
uint64_t x4 = (uint64_t)x;                           // add 0s out to four lanes (0,0,x,x)
return from_half(skvx::half4::Load(&x4));
```
