---
name: skia/api/keep-parameters-consistent-across-data-proc-variants
description: When defining callback or proc variants that differ in data representation or lifetime, keep non-lifetime parameters and metadata consistent across all variants
rubric: correctness
severity: should_fix
---

**Rule.** When defining alternative callback, function pointer, or proc
signatures that differ only in data container or lifetime semantics (such as raw
pointer/length vs. sk_sp<SkData> vs. SkStream), keep all non-lifetime parameters
(such as format tags, options, alpha type, and context pointers) identical
across all variants. Do not omit contextual metadata or configuration parameters
from any representation variant.

**Why.** Omitting contextual parameters from one data container variant deprives
callers using that format of essential metadata, forcing callers into awkward
workarounds or redundant buffer copies. Maintaining parameter symmetry across
variants ensures consistent API capabilities and prevents breaking migration
churn when metadata support is eventually added.

## Examples

Bad:

```cpp
// Asymmetric: only the SkData variant receives optional alpha type metadata.
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data,
                                               size_t length,
                                               void* ctx);
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData> data,
                                                       std::optional<SkAlphaType> alphaType,
                                                       void* ctx);
```

Good:

```cpp
// Symmetric: both variants receive identical metadata and context parameters.
using SkDeserialImageProc = sk_sp<SkImage> (*)(const void* data,
                                               size_t length,
                                               std::optional<SkAlphaType> alphaType,
                                               void* ctx);
using SkDeserialImageFromDataProc = sk_sp<SkImage> (*)(sk_sp<SkData> data,
                                                       std::optional<SkAlphaType> alphaType,
                                                       void* ctx);
```
