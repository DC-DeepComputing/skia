---
name: skia/modules/declare-extern-methods-on-prototype-with-function-syntax
description: 'In CanvasKit Closure Compiler externs, define instance methods under prototype using name: function() {} syntax instead of ES6 method shorthand or top-level class declarations'
rubric: correctness
severity: should_fix
---

**Rule.** In CanvasKit Closure Compiler externs (such as
modules/canvaskit/externs.js), define instance methods on the class prototype
using standard property function syntax (name: function() {}). Do not use ES6
method shorthand (name() {}), and do not declare instance methods directly on
the class or namespace object.

**Why.** Closure Compiler externs require ES5 function property syntax to parse
correctly without minification syntax errors. Furthermore, declaring instance
methods directly on the class object registers them as static namespace
properties, failing to protect instance invocations from being mangled or
stripped during release minification.

## Examples

Bad:

```cpp
ImageFilter: {
  // Anti-pattern: ES6 method shorthand and instance method declared at class level
  filterBounds() {},
  MakeBlend: function() {},
  prototype: {
    // Anti-pattern: ES6 shorthand inside prototype
    filterBounds() {},
  },
}
```

Good:

```cpp
ImageFilter: {
  // Static factory function on the class
  MakeBlend: function() {},
  prototype: {
    // Instance method correctly placed on prototype with function syntax
    filterBounds: function() {},
  },
}
```
