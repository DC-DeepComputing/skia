---
name: skia/modules/prefix-canvaskit-wasm-factories-with-make
description: When exposing CanvasKit APIs that construct and return new WebAssembly-backed objects, provide static Make-prefixed factory functions rather than instance methods
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit APIs and TypeScript definitions, expose operations that
construct and return a new WebAssembly-backed object as static factory functions
prefixed with Make (e.g., CanvasKit.Path.MakeFrom...) rather than instance
methods. Signal through the Make prefix that the returned object allocates
WebAssembly linear memory and must be manually released via .delete().

**Why.** CanvasKit objects allocated in WebAssembly linear memory are not
tracked or freed by JavaScript's garbage collector. Using a static Make prefix
conforms to CanvasKit conventions that warn callers the resulting object
requires manual cleanup via .delete(), while avoiding confusing instance methods
that appear to mutate the receiver or create unmanaged leaks.

## Examples

Bad:

```cpp
// On Path instance interface:
path.interpolate(end: Path, weight: number): Path | null;
```

Good:

```cpp
// Static factory on Path constructor/namespace:
CanvasKit.Path.MakeFromPathInterpolation(start: Path, end: Path, weight: number): Path | null;
```
