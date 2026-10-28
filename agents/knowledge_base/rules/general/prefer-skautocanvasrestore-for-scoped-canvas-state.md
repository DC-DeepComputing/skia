---
name: skia/prefer-skautocanvasrestore-for-scoped-canvas-state
description: When temporarily or conditionally modifying canvas or clip state, use SkAutoCanvasRestore or SkClipStack::AutoRestore (with doSave=false) rather than manual save/restore, and omit redundant SkAutoDeviceTransformRestore
rubric: readability
severity: nit
---

**Rule.** When temporarily modifying canvas or clip stack state within a local
scope, manage the state using RAII helpers such as `SkAutoCanvasRestore` or
`SkClipStack::AutoRestore` rather than manual `save()` and `restore()` pairs.
When canvas or clip modifications are conditional, construct the auto-restore
helper with `/*doSave=*/false` to capture the initial save count and
automatically restore on exit if nested `save()` calls occurred, rather than
maintaining manual tracking flags or `SK_AT_SCOPE_EXIT` guards. Additionally,
when drawing through `SkCanvas` with `canvas->setMatrix()`, do not retain
`SkAutoDeviceTransformRestore` on the underlying `SkDevice`, as `SkCanvas`
already coordinates and restores the transform.

**Why.** Manual `save()` and `restore()` calls or ad-hoc boolean tracking flags
and `SK_AT_SCOPE_EXIT` macros are verbose and risk leaving canvas or clip stack
state unbalanced across early returns and error branches. Initializing
`SkAutoCanvasRestore` or `SkClipStack::AutoRestore` with `doSave = false`
cleanly captures the baseline save count and guarantees balanced restoration
only when saves occurred. Furthermore, `SkCanvas` coordinates device transform
state, making `SkAutoDeviceTransformRestore` redundant.

## Examples

Bad:

```cpp
// Bad: Manual save/restore risks leaking canvas state on early return
canvas->save();
canvas->translate(dx, dy);
if (earlyExit) {
    return;
}
canvas->drawPath(path, paint);
canvas->restore();

// Bad: Manual boolean flag and scope-exit guard for conditional saves
bool needToRestore = false;
if (condition) {
    this->cs().save();
    this->cs().clipRect(dst, ctm, SkClipOp::kIntersect, true);
    needToRestore = true;
}
SK_AT_SCOPE_EXIT(if (needToRestore) { this->cs().restore(); });

// Bad: Redundant device transform restore when drawing via SkCanvas
SkAutoDeviceTransformRestore adr2(this, SkM44(shadowMatrix));
SkAutoCanvasRestore autoRestore(canvas, /*doSave=*/true);
canvas->setMatrix(shadowMatrix);
canvas->drawPath(path, paint);
```

Good:

```cpp
// Good: RAII safely restores canvas state on any exit path
SkAutoCanvasRestore autoRestore(canvas, /*doSave=*/true);
canvas->translate(dx, dy);
if (earlyExit) {
    return;
}
canvas->drawPath(path, paint);

// Good: AutoRestore with doSave=false cleanly handles conditional save/clip
SkClipStack::AutoRestore autoRestore(&this->cs(), /*doSave=*/false);
if (condition) {
    this->cs().save();
    this->cs().clipRect(dst, ctm, SkClipOp::kIntersect, true);
}

// Good: SkAutoCanvasRestore and setMatrix coordinate device transforms
SkAutoCanvasRestore autoRestore(canvas, /*doSave=*/true);
canvas->setMatrix(shadowMatrix);
canvas->drawPath(path, paint);
```
