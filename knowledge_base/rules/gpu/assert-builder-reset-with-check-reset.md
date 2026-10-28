---
name: skia/gpu/assert-builder-reset-with-check-reset
description: When managing Graphite draw scratch gatherers and builders, acquire pooled helpers via per-draw RAII guards rather than Device fields, asserting checkReset() and avoiding SkDEBUGCODE mutation
rubric: safety
severity: should_fix
---

**Rule.** Acquire and scope the lifecycle of reusable draw scratch gatherers and
builders (such as PipelineDataGatherer, PaintParamsKeyBuilder, or
KeyAndDataBuilder) to individual draw calls using RAII guards (e.g.,
ScopedDrawBuilder) rather than holding pooled helpers in member fields on
Device, scattering manual resetForDraw() calls across return paths, or deferring
cleanup to flush. Have the RAII constructor acquire the helper and assert clean
state with checkReset() (not at call sites), and the destructor reset and return
it to the pool. In callee helpers, verify clean state with
SkDEBUGCODE(gatherer->checkReset()); never call mutating reset methods within
SkDEBUGCODE.

**Why.** Holding pooled helpers on long-lived objects like Device hoards pool
capacity across idle surfaces and forces unnecessary allocations. Scoping
acquisition and resets to RAII guards reuses a small fixed pool across all
recording devices, prevents resource leaks across early returns, and guarantees
cleanup without retaining texture references until flush. Furthermore, asserting
checkReset() in RAII constructors and callee helpers catches unreset state
without masking caller contract violations or causing release/debug divergence
from mutating SkDEBUGCODE.

## Examples

Bad:

```cpp
// BAD: Holding pooled helper as a long-lived Device member field:
Device::Device(Recorder* recorder, ...) {
    fKeyContextHelpers = fRecorder->priv().popOrCreateKeyContextHelpers();
}

// BAD: Scattering manual resets across early returns or call sites:
void Device::drawGeometry(...) {
    if (!keyResult) {
        fDataGatherer->resetForDraw();
        return;
    }
    fDataGatherer->resetForDraw();
}

// BAD: Mutating shared gatherer inside debug macro or callee:
SkDEBUGCODE(gatherer->resetForDraw());
```

Good:

```cpp
// GOOD: Per-draw RAII guard acquires, asserts checkReset(), resets, and recycles helper:
class ScopedDrawBuilder {
public:
    explicit ScopedDrawBuilder(Recorder* recorder)
            : fRecorder(recorder),
              fKeyAndDataBuilder(fRecorder->priv().popOrCreateKeyAndDataBuilder()) {
        SkASSERT(fKeyAndDataBuilder);
        SkDEBUGCODE(this->gatherer()->checkReset());
        SkDEBUGCODE(this->builder()->checkReset());
    }
    ~ScopedDrawBuilder() {
        this->gatherer()->resetForDraw();
        fRecorder->priv().pushKeyAndDataBuilder(std::move(fKeyAndDataBuilder));
    }
    PipelineDataGatherer* gatherer() { return &fKeyAndDataBuilder->first; }
    PaintParamsKeyBuilder* builder() { return &fKeyAndDataBuilder->second; }
private:
    Recorder* fRecorder;
    std::unique_ptr<KeyAndDataBuilder> fKeyAndDataBuilder;
};

void Device::drawGeometry(...) {
    ScopedDrawBuilder builder(fRecorder);
    if (!keyResult) {
        return;
    }
    // ...
}
```
