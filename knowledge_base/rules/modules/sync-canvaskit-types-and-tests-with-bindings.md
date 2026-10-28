---
name: skia/modules/sync-canvaskit-types-and-tests-with-bindings
description: When exposing or modifying CanvasKit APIs across C++ bindings, TypeScript declarations, or JavaScript modules, keep bindings, types, JS implementations, externs, and tests synchronized
rubric: readability
severity: should_fix
---

**Rule.** When exposing or modifying CanvasKit APIs across C++ Emscripten
bindings, TypeScript definitions, or JavaScript modules, keep all layers
synchronized:
- When binding C++ methods in canvaskit_bindings.cpp, add TypeScript declarations in npm_build/types/index.d.ts and type tests in npm_build/types/canvaskit-wasm-tests.ts.
- When declaring types, enums, properties, or constructors in npm_build/types/index.d.ts, ensure they match exposed runtime APIs: implement required JS definitions and omit unexposed or internal types.
- When adding public JS methods or private C++ Embind methods, declare them in modules/canvaskit/externs.js to prevent Google Closure Compiler from minifying them in release builds.
- When introducing or modifying APIs, add executable Jasmine tests in modules/canvaskit/tests/ (e.g., core_test.js); do not rely solely on canvaskit-wasm-tests.ts, which is only statically checked by dtslint.

**Why.** CanvasKit exposes public APIs through a combination of
WebAssembly/Emscripten bindings, JavaScript wrapper helpers, TypeScript
declarations, and Closure Compiler externs. Omitting TypeScript declarations
leaves consumers without type definitions, declaring unexposed types or omitting
JS implementations causes runtime undefined property access errors, omitting
methods from externs.js causes Closure Compiler to minify or mangle property
names in release builds, and relying solely on dtslint type tests without
runtime Jasmine tests in modules/canvaskit/tests/ leaves Embind bindings and
WebGL/Wasm execution unverified in CI.

## Examples

Bad:

```cpp
// 1. C++ method bound in canvaskit_bindings.cpp but omitted from index.d.ts:
// builder.close();
// builder.snapshot(); // Missing from index.d.ts

// 2. TypeScript constructor declared without JavaScript implementation,
// or unexposed enum declared on CanvasKit interface:
// modules/canvaskit/npm_build/types/index.d.ts
export interface CanvasKit {
  readonly LineBreakType: LineBreakTypeEnumValues; // Not exposed in JS API!
}
export interface SlottableTextPropertyConstructor {
  new(text: SlottableTextProperty): SlottableTextProperty;
}
// CanvasKit.SlottableTextProperty omitted from modules/canvaskit/skottie.js

// 3. New JS methods or Embind bindings omitted from externs.js:
// modules/canvaskit/externs.js
ManagedAnimation: {
  prototype: {
    setColor: function() {},
    // Missing: getColorSlot
  },
  // Missing: _getColorSlot
}

// 4. API tested only in TypeScript type tests without runtime unit tests:
// modules/canvaskit/npm_build/types/canvaskit-wasm-tests.ts
const surface = CK.MakeOnScreenGLSurface(grCtx!, 100, 400, // $ExpectType Surface
    CK.ColorSpace.ADOBE_RGB, sample, stencil)!;
// modules/canvaskit/tests/core_test.js: no executable Jasmine test added
```

Good:

```cpp
// 1. Keep index.d.ts and canvaskit-wasm-tests.ts in sync with C++ bindings:
// modules/canvaskit/npm_build/types/index.d.ts
export interface PathBuilder extends EmbindObject<"SkPathBuilder"> {
  snapshot(): Path;
}
// modules/canvaskit/npm_build/types/canvaskit-wasm-tests.ts
const p8 = builder.snapshot(); // $ExpectType Path

// 2. Omit unexposed types and implement constructor on CanvasKit in JS:
// modules/canvaskit/npm_build/types/index.d.ts: omit LineBreakType if unexposed in JS
// modules/canvaskit/skottie.js
CanvasKit.SlottableTextProperty = function(t) {
  t['text'] = t['text'] || "";
  return t;
};

// 3. Declare new public and private Embind methods in externs.js:
// modules/canvaskit/externs.js
ManagedAnimation: {
  prototype: {
    getColorSlot: function() {},
  },
  _getColorSlot: function() {},
}

// 4. Add executable runtime tests in modules/canvaskit/tests/:
// modules/canvaskit/tests/core_test.js
it('can provide sample count and stencil parameters', () => {
  const surface = CanvasKit.MakeOnScreenGLSurface(grContext, 100, 100, CanvasKit.ColorSpace.SRGB, sample, stencil);
  expect(surface).toBeTruthy();
});
```
