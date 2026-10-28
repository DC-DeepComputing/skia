---
name: skia/gpu/group-ycbcr-model-and-range-in-conversion-strings
description: When formatting YCbCr conversion or sampler info for debug labels, group coupled color model and range properties using '+' rather than whitespace
rubric: readability
severity: nit
---

**Rule.** When formatting YCbCr conversion or sampler descriptor properties into
human-readable strings (such as debug dumps or pipeline labels), group closely
coupled color properties like model and range using '+' (e.g.,
'<model>+<range>') rather than plain whitespace delimiters.

**Why.** YCbCr conversion descriptors contain many individual properties
(format, external format, model, range, chroma offsets, chroma filters,
swizzles). Separating all fields with plain spaces impairs visual parsing;
binding coupled color attributes like model and range with '+' makes the
composite descriptor easier to scan in logs and pipeline keys.

## Examples

Bad:

```cpp
// Separating tightly coupled color model and range with spaces
result += VkModelToStr(info.model());
result += " ";
result += VkRangeToStr(info.range());
```

Good:

```cpp
// Group tightly coupled color model and range with '+'
result += VkModelToStr(info.model());
result += "+";
result += VkRangeToStr(info.range());
```
