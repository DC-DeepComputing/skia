---
name: skia/modules/delete-wasm-canvaskit-objects
description: When creating C++ or WASM-backed CanvasKit objects in JavaScript or TypeScript, explicitly call .delete() before reassigning references or leaving scope
rubric: safety
severity: should_fix
---

**Rule.** In CanvasKit JavaScript and TypeScript code and tests, explicitly call
`.delete()` on all C++/WASM-backed CanvasKit objects (such as `Path`, `Paint`,
`Surface`, and intermediate objects created via `new`, `Make*`, or factory
methods) before reassigning references or letting variables fall out of scope.

**Why.** CanvasKit objects reside in WebAssembly linear memory. Because
JavaScript garbage collection cannot track or reclaim WASM heap allocations,
failing to call `.delete()` before reassigning reference variables or exiting
scope causes permanent linear memory leaks.

## Examples

Bad:

```cpp
const p1 = new CanvasKit.Path();
const p2 = new CanvasKit.Path();
let p3 = p1.interpolate(p2, 0);
p3 = p1.interpolate(p2, 1);
```

Good:

```cpp
const p1 = new CanvasKit.Path();
const p2 = new CanvasKit.Path();
let p3 = p1.interpolate(p2, 0);
p3.delete();
p3 = p1.interpolate(p2, 1);
p3.delete();
p1.delete();
p2.delete();
```
