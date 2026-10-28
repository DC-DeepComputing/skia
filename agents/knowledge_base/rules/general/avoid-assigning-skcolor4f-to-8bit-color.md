---
name: skia/avoid-assigning-skcolor4f-to-8bit-color
description: When converting between SkColor4f and 8-bit color, use FromColor or toSkColor instead of unscaled casts or manual SkColorGet channel division
rubric: readability
severity: should_fix
---

**Rule.** When converting between SkColor4f and 8-bit color representations, use
conversion helpers like SkColor4f::FromColor() and color.toSkColor() rather than
direct assignment or manual boilerplate. When converting packed SkColor to
normalized float components, use SkColor4f::FromColor(color) rather than
manually extracting channels with SkColorGet{R,G,B,A} and dividing by 255.0f.
When initializing SkColor4f from raw 8-bit integer channels, normalize to [0.0f,
1.0f]; when assigning SkColor4f to 8-bit integer structures, scale normalized
floats to 255 or select a float-compatible format.

**Why.** SkColor4f stores unpremultiplied color components as floats normalized
to [0.0f, 1.0f], whereas 8-bit color types store integer values in [0, 255].
SkColor4f::FromColor and toSkColor encapsulate normalization, channel
extraction, and packing; manual channel extraction and arithmetic introduces
verbose boilerplate, while assigning unscaled values results in out-of-range
floats (> 1.0f) or truncated integer zeros.

## Examples

Bad:

```cpp
// 1. Manually extracting channels and scaling SkColor instead of FromColor:
CGFloat components[4];
components[0] = (CGFloat)SkColorGetR(bgra) * (1/255.0f);
components[1] = (CGFloat)SkColorGetG(bgra) * (1/255.0f);
components[2] = (CGFloat)SkColorGetB(bgra) * (1/255.0f);
components[3] = (CGFloat)SkColorGetA(bgra) * (1/255.0f);

// 2. Initializing SkColor4f from 8-bit integer channels without normalizing:
float ambientf = static_cast<float>(fLight.fAmbient); // uint8_t [0..255]
SkColor4f ambientColor = {ambientf, ambientf, ambientf, 1.0f};

// 3. Assigning normalized SkColor4f float components directly to 8-bit fields:
XPS_COLOR xpsColor;
xpsColor.colorType = XPS_COLOR_TYPE_SRGB;
xpsColor.value.sRGB.red = skColor.fR; // Truncates float in [0.0f, 1.0f] to 0
```

Good:

```cpp
// 1. Use SkColor4f::FromColor to unpack normalized float components:
SkColor4f color4f = SkColor4f::FromColor(bgra);
CGFloat components[4] = {(CGFloat)color4f.fR, (CGFloat)color4f.fG,
                         (CGFloat)color4f.fB, (CGFloat)color4f.fA};

// 2. Normalize 8-bit channels to [0.0f, 1.0f]:
float ambientf = static_cast<float>(fLight.fAmbient) / 255.0f;
SkColor4f ambientColor = {ambientf, ambientf, ambientf, 1.0f};

// 3. Use float-compatible format, toSkColor(), or explicit scaling:
XPS_COLOR xpsColor;
xpsColor.colorType = XPS_COLOR_TYPE_SCRGB;
xpsColor.value.scRGB.red = skColor.fR;
```
