---
name: skia/modules/use-ts-names-in-canvaskit-doc-comments
description: When writing or adapting doc comments in TypeScript declarations, use TypeScript binding types and constants instead of underlying C++ identifiers
rubric: readability
severity: nit
---

**Rule.** When writing or adapting API doc comments in TypeScript/JavaScript
bindings (such as CanvasKit's `.d.ts` files), refer to the target language and
binding identifiers rather than copying underlying C++ symbols. Replace C++
class prefixes and enum names (e.g., SkPath, SkRect, kMove_Verb) with their
exposed TypeScript equivalents (e.g., Path, Rect, MOVE_VERB).

**Why.** CanvasKit TypeScript definitions serve as public API documentation and
editor tooltip references for web developers. Using internal C++ type names and
enum conventions is confusing and inaccurate because those C++ symbols do not
exist in the JavaScript or TypeScript runtime environment.

## Examples

Bad:

```cpp
/**
 * Adds circle centered at (x, y) of size radius to SkPath, appending kMove_Verb,
 * four kConic_Verb, and kClose_Verb.
 */
addCircle(x: number, y: number, r: number): Path;
```

Good:

```cpp
/**
 * Adds circle centered at (x, y) of size radius to Path, appending MOVE_VERB,
 * four CONIC_VERB, and CLOSE_VERB.
 */
addCircle(x: number, y: number, r: number): Path;
```
