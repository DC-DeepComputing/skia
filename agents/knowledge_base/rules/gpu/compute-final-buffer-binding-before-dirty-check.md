---
name: skia/gpu/compute-final-buffer-binding-before-dirty-check
description: When tracking bound buffer state to skip redundant GPU bindings, compute the final normalized binding configuration before comparing against the currently bound state
rubric: correctness
severity: should_fix
---

**Rule.** When caching or tracking bound GPU buffer state to skip redundant
bindings, compute the final binding configuration (including any conditional
normalizations such as zeroing offsets for stride-aligned base offsets) before
comparing against the currently bound state. Do not compare the bound state
against an unconditionally normalized candidate before verifying that the
normalization conditions are met.

**Why.** Comparing against a normalized candidate binding before checking
alignment conditions causes false-positive rebinds or missed dirty updates when
the buffer offset is not aligned. Finalizing the binding representation before
comparison ensures dirty tracking accurately reflects whether the GPU state
actually needs to be rebound.

## Examples

Bad:

```cpp
// Bad: Compares against a normalized binding before verifying stride alignment
BindBufferInfo newBinding = {buffer.fBuffer, 0, SkTo<uint32_t>(buffer.fBuffer->size())};
rebindDrawBuffers |= boundBuffer != newBinding;
if (buffer.fOffset % stride == 0) {
    pendingBase = buffer.fOffset / stride;
    boundBuffer = newBinding;
} else {
    boundBuffer = buffer;
}
```

Good:

```cpp
// Good: Compute normalized binding first, then evaluate dirty comparison
BindBufferInfo newBinding = buffer;
if (buffer.fOffset % stride == 0) {
    pendingBase = buffer.fOffset / stride;
    newBinding = {buffer.fBuffer, 0, SkTo<uint32_t>(buffer.fBuffer->size())};
}
rebindDrawBuffers |= boundBuffer != newBinding;
boundBuffer = newBinding;
```
