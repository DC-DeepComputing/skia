---
name: skia/core/preserve-matrixrec-validity-for-runtime-effect-callbacks
description: When passing a MatrixRec to RuntimeEffectRPCallbacks or child shader evaluators, do not unconditionally call markTotalMatrixInvalid(); let callbacks invalidate selectively for non-passthrough samples
rubric: correctness
severity: should_fix
---

**Rule.** Do not unconditionally call MatrixRec::markTotalMatrixInvalid() before
passing a MatrixRec into RuntimeEffectRPCallbacks or child shader evaluation
helpers. Allow the callbacks to selectively invalidate the total matrix for
non-passthrough child samples while preserving matrix validity for passthrough
samples.

**Why.** RuntimeEffectRPCallbacks::appendShader selectively invalidates the
MatrixRec for non-passthrough child shader samples while preserving matrix
validity when child coordinates are passed through unmodified. Unconditionally
invalidating the MatrixRec prior to invoking callbacks forces all child shaders
(such as SkImageShader) to treat the matrix as unknown or invalid, defeating
sampling and MIP-mapping optimizations.

## Examples

Bad:

```cpp
SkShaderBase::MatrixRec matrix(SkMatrix::I());
matrix.markTotalMatrixInvalid();
RuntimeEffectRPCallbacks callbacks(rec, matrix, fChildren, fEffect->fSampleUsages);
```

Good:

```cpp
SkShaderBase::MatrixRec matrix(SkMatrix::I());
RuntimeEffectRPCallbacks callbacks(rec, matrix, fChildren, fEffect->fSampleUsages);
```
