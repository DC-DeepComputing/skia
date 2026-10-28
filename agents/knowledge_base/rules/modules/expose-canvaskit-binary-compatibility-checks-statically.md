---
name: skia/modules/expose-canvaskit-binary-compatibility-checks-statically
description: When exposing binary operations between two instances in CanvasKit as static factory functions, expose companion prerequisite compatibility checks as static class functions
rubric: design
severity: should_fix
---

**Rule.** When exposing binary operations between two instances as static
factory functions on a CanvasKit class (such as
CanvasKit.Path.MakeFromPathInterpolation), expose companion prerequisite
compatibility checks (such as CanInterpolate) as static class functions on the
constructor/factory object taking both operands. Do not expose compatibility
checks as asymmetric instance methods on only one operand.

**Why.** Exposing a prerequisite check as an instance method treats one operand
as the receiver and the other as an argument, creating an asymmetric API for an
inherently symmetric operation. Placing the query as a static method alongside
its companion static factory keeps the API uniform and aligns with CanvasKit
conventions for multi-instance operations.

## Examples

Bad:

```cpp
// In index.d.ts - asymmetric instance method:
export interface Path extends EmbindObject<Path> {
    isInterpolatable(compare: Path): boolean;
}
```

Good:

```cpp
// In index.d.ts - static check alongside companion static factory:
export interface PathConstructorAndFactory extends DefaultConstructor<Path> {
    CanInterpolate(path1: Path, path2: Path): boolean;
    MakeFromPathInterpolation(start: Path, end: Path, weight: number): Path | null;
}
```
