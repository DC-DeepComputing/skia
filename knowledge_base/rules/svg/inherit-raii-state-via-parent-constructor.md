---
name: skia/svg/inherit-raii-state-via-parent-constructor
description: When propagating shared context to nested child RAII helper scopes, inherit state via a parent-referencing constructor rather than bundling fields into an options struct
rubric: design
severity: nit
---

**Rule.** When propagating shared context and writers to nested child RAII scope
helpers, provide a constructor that takes a reference to the parent scope (e.g.,
const AutoHelper& parent) and initializes members directly. Do not wrap
parameters into an intermediate options struct solely to pass state down to
nested helper instances.

**Why.** Creating an intermediate struct solely to bundle parameters for nested
RAII scopes adds unnecessary abstraction and forces existing direct member
accesses to be rewritten through the struct. Initializing directly from the
parent helper avoids indirection and call-site churn while keeping member access
direct.

## Examples

Bad:

```cpp
struct Options {
    SkSVGCanvas::EncodePngCallback pngEncoder = nullptr;
    SkXMLWriter* xmlWriter = nullptr;
};
AutoElement(const char name[], Options opts) : fOpts(opts) {
    fOpts.xmlWriter->startElement(name);
}
Options fOpts;

// Inside helper method:
AutoElement defs("defs", fOpts);
```

Good:

```cpp
AutoElement(const char name[], const AutoElement& parent)
    : fWriter(parent.fWriter)
    , fPngEncoder(parent.fPngEncoder) {
    fWriter->startElement(name);
}
SkXMLWriter*                   fWriter = nullptr;
SkSVGCanvas::EncodePngCallback fPngEncoder = nullptr;

// Inside helper method:
AutoElement defs("defs", *this);
```
