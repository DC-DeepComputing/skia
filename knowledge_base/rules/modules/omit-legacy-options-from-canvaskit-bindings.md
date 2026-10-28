---
name: skia/modules/omit-legacy-options-from-canvaskit-bindings
description: When binding C++ enums or options in CanvasKit Emscripten bindings or TypeScript definitions, omit legacy or backwards-compatibility-only values
rubric: design
severity: should_fix
---

**Rule.** When exposing C++ enums, flags, or configuration options in CanvasKit
Emscripten bindings or TypeScript definitions, do not bind legacy or
backwards-compatibility-only values. Restrict public JavaScript bindings
strictly to modern, active options, and handle legacy C++ variants internally or
in deserialization logic.

**Why.** C++ backends often retain deprecated enum values and obsolete modes
solely for backwards compatibility with legacy serialized assets or historical
file formats. Exposing these compatibility options in CanvasKit expands the
public JavaScript API surface with deprecated modes that new web applications
should never use and increases long-term API maintenance burdens.

## Examples

Bad:

```cpp
// modules/canvaskit/npm_build/types/index.d.ts
export interface VerticalTextAlignEnumValues extends EmbindEnum {
  Top: VerticalAlign;
  TopBaseline: VerticalAlign;
  HybridTop: VerticalAlign;     // Deprecated Bodymovin compatibility mode
  HybridCenter: VerticalAlign;  // Deprecated Bodymovin compatibility mode
  VisualTop: VerticalAlign;
  VisualCenter: VerticalAlign;
}
```

Good:

```cpp
// modules/canvaskit/npm_build/types/index.d.ts
export interface VerticalTextAlignEnumValues extends EmbindEnum {
  Top: VerticalAlign;
  TopBaseline: VerticalAlign;
  VisualTop: VerticalAlign;
  VisualCenter: VerticalAlign;
}
```
