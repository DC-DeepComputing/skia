---
name: skia/gpu/specify-alpha-as-unorm-in-srgb-formats
description: When defining sRGB texture format channels or test expectation tables, specify alpha channel encoding as linear UNorm rather than sRGB
rubric: correctness
severity: should_fix
---

**Rule.** In sRGB texture format definitions, channel descriptor structs, and
format test expectation tables (such as sRGBA or BC1_sRGB), declare the alpha
channel encoding as linear UNorm rather than sRGB. Ensure only the color
channels (red, green, blue) specify non-linear sRGB transfer characteristics.

**Why.** Standard graphics format specifications apply non-linear sRGB transfer
functions exclusively to RGB color components; the alpha channel represents
linear opacity or coverage and is universally sampled and encoded as linear
UNorm. Specifying sRGB encoding for alpha causes incorrect gamma decoding or
invalid test expectations.

## Examples

Bad:

```cpp
// Incorrectly assigning sRGB transfer encoding to the alpha channel
{.fFormat=TextureFormat::kRGBA8_BC1_sRGB,
 .fChannels={{'r', 10, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}, {'a', 8, sRGB}}},
```

Good:

```cpp
// Alpha channel is linear UNorm even in sRGB texture formats
{.fFormat=TextureFormat::kRGBA8_BC1_sRGB,
 .fChannels={{'r', 10, sRGB}, {'g', 8, sRGB}, {'b', 8, sRGB}, {'a', 8, UNorm}}},
```
