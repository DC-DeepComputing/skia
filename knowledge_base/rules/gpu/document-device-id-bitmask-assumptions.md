---
name: skia/gpu/document-device-id-bitmask-assumptions
description: When using bitmasks or prefix matching on hardware or PCI device IDs to classify GPU models, document bit-field layout assumptions, driver sources, and known exceptions
rubric: readability
severity: nit
---

**Rule.** When using bitmasks, bit shifts, or prefix ranges on hardware device
IDs (such as PCI device IDs in GPU Caps or utility helpers) to classify GPU
models or architectures, document the underlying bit-field layout assumptions in
an accompanying comment. Cite authoritative driver sources or hardware
references (such as Mesa PCI ID tables), and explicitly note any known
architecture exceptions or ID collisions where the heuristic deviates.

**Why.** Hardware device IDs pack architecture family, tier, market segment, and
stepping across disparate bit fields, and different product lines may deviate
from general masking patterns. Documenting bit-field heuristics and known
exceptions ensures reviewers can verify masking logic against upstream sources,
prevents false assumptions about future hardware IDs, and simplifies maintaining
hardware workarounds.

## Examples

Bad:

```cpp
// Check Intel GPU model for driver workarounds
switch (deviceID & 0xFF00) {
    case 0x1900:
        return IntelGPUType::kSkyLake;
    case 0x7D00:
        return IntelGPUType::kMeteorLake;
}
```

Good:

```cpp
// Referenced from Mesa iris driver PCI IDs (pci_ids/iris_pci_ids.h):
// High-order byte generally defines the GPU generation.
// Note: MTL/ARL share 0x7D00 across differing markets; inspect low bytes for exceptions.
switch (deviceID & 0xFF00) {
    case 0x1900:
        return IntelGPUType::kSkyLake;
    case 0x7D00:
        return IntelGPUType::kMeteorLake;
}
```
