---
name: skia/encapsulate-wrapper-initialization-in-constructors
description: In wrapper and adapter constructors, accept target objects directly and register them internally instead of requiring callers to unpack dependencies or make separate setup calls
rubric: design
severity: should_fix
---

**Rule.** When designing constructors for wrapper, adapter, or decorator
classes, accept the target object directly rather than requiring callers to
unpack and pass internal dependencies (such as backing devices or sub-objects).
Fully initialize the wrapper by extracting internal dependencies and registering
or attaching the target internally, and assert against conflicting multi-target
additions when the wrapper is single-target. Do not leave the instance partially
initialized by requiring callers or derived classes to perform separate setup or
registration calls post-construction.

**Why.** Requiring callers to unpack internal dependencies or make separate
setup calls (such as addCanvas() or attach()) leaks implementation details,
creates error-prone two-step initialization, and risks invalid or partially
initialized wrapper states. Encapsulating dependency extraction and registration
inside the constructor guarantees that wrapper invariants and target consistency
are established uniformly upon creation.

## Examples

Bad:

```cpp
// Bad: Callers must unpack internal device and separately register the target canvas
SkNWayCanvas::SkNWayCanvas(sk_sp<SkBaseDevice> device) : INHERITED(std::move(device)) {}

SkTestCanvas::SkTestCanvas(SkCanvas* canvas) : SkNWayCanvas(sk_ref_sp(canvas->baseDevice())) {
    this->addCanvas(canvas); // Error-prone separate setup post-construction
}
```

Good:

```cpp
// Good: Accept target directly, extract internal dependencies, and register internally
SkNWayCanvas::SkNWayCanvas(SkCanvas* canvas) : INHERITED(sk_ref_sp(canvas->baseDevice())) {
    SkASSERT(canvas);
    this->addCanvas(canvas); // Fully encapsulated initialization
}

SkTestCanvas::SkTestCanvas(SkCanvas* canvas) : SkNWayCanvas(canvas) {}
```
