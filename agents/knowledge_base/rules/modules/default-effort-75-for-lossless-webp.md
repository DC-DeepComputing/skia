---
name: skia/modules/default-effort-75-for-lossless-webp
description: When configuring SkWebpEncoder with lossless compression (kLossless), set fQuality to 75 for default compression effort to match libwebp and Blink conventions
rubric: readability
severity: nit
---

**Rule.** When configuring lossless WebP encoding
(SkWebpEncoder::Compression::kLossless) with a default or fixed compression
effort, set SkWebpEncoder::Options::fQuality to 75 rather than an arbitrary
value. This maintains consistency with libwebp's WebPConfigInit defaults and
Blink conventions.

**Why.** In lossless WebP compression, fQuality controls compression effort
(speed vs. compression ratio) rather than image fidelity. Setting fQuality to 75
provides a balanced trade-off between encoding time and output size consistent
with standard libwebp and Chromium defaults.

## Examples

Bad:

```cpp
opts.fCompression = SkWebpEncoder::Compression::kLossless;
opts.fQuality = 70; // Non-standard compression effort
```

Good:

```cpp
opts.fCompression = SkWebpEncoder::Compression::kLossless;
opts.fQuality = 75; // Matches libwebp and Blink default effort
```
