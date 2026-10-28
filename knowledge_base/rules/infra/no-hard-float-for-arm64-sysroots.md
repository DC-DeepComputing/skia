---
name: skia/infra/no-hard-float-for-arm64-sysroots
description: When documenting ARM64 or AArch64 sysroots and toolchains, do not describe them as hard float since hardware floating-point is standard in ARM64
rubric: readability
severity: nit
---

**Rule.** Do not describe ARM64 (AArch64) sysroots, toolchain assets, or build
environments as 'hard float'. Hardware floating-point support is standard and
mandatory in the ARM64 ABI, unlike 32-bit ARM (ARMv7) which distinguishes
between soft-float (armel) and hard-float (armhf).

**Why.** In 32-bit ARM architectures, sysroots and toolchain configurations
distinguish between soft-float (armel) and hard-float (armhf) ABIs. In ARM64,
hardware floating-point support is an inherent requirement of the standard ABI,
making '(hard float)' annotations redundant and misleading when adapting
documentation from 32-bit assets.

## Examples

Bad:

```cpp
ARM64 (hard float) sysroot for cross-compiling c++ code on an x86_64 Linux machine.
```

Good:

```cpp
ARM64 sysroot for cross-compiling c++ code on an x86_64 Linux machine.
```
