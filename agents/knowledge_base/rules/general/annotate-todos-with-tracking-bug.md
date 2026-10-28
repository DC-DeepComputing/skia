---
name: skia/annotate-todos-with-tracking-bug
description: When adding or modifying TODO comments, ensure they include an active tracking issue reference and explain the planned task or condition for removal
rubric: readability
severity: nit
---

**Rule.** Format all TODO comments with an attributed tracking bug reference
(such as `TODO(b/123456789): ...`, `TODO(skbug.com/1234): ...`, or
`TODO(https://crbug.com/...): ...`) and a concise description of the remaining
task or condition for resolution. Never leave bare or untracked `TODO` or
`FIXME` comments without an issue reference. This rule does not apply to
temporary local debugging lines not intended for commit, or self-contained
explanatory comments that document intentional architectural decisions rather
than deferred work.

**Why.** Untracked TODO comments accumulate technical debt and frequently become
stale or forgotten because there is no assigned owner or tracking issue to
ensure follow-up. Citing an issue tracker ID provides searchable context,
enables automated tooling to notify owners when related dependencies change, and
ensures future maintainers understand when and how the deferred work should be
completed.

## Examples

Bad:

```cpp
// Variant 1 (API Header):
// TODO: implement capture recording for CPU canvas
sk_sp<SkCanvas> makeCaptureCanvas();

// Variant 2 (GPU Pipeline):
// TODO: add support for sample counts > 1
SkASSERT(fTargetTexture && fTargetTexture->numSamples() == 1);

// Variant 3 (Core):
// TODO: handle CICP profile conversions
sk_sp<SkColorSpace> MakeCICP();
```

Good:

```cpp
// Variant 1 (API Header):
// TODO(b/294543210): Add recording canvas support for CPU-backed capture.
sk_sp<SkCanvas> makeCaptureCanvas();

// Variant 2 (GPU Pipeline):
// TODO(b/302126809): Remove sample count restriction once shader sample loading supports multisampling.
SkASSERT(fTargetTexture && fTargetTexture->numSamples() == 1);

// Variant 3 (Core):
// TODO(https://crbug.com/1339019): Parse and validate CICP color profiles per ISO/IEC 23000-19.
sk_sp<SkColorSpace> MakeCICP();
```
