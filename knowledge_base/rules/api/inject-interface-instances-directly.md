---
name: skia/api/inject-interface-instances-directly
description: When designing APIs with dependency injection, inject concrete instances directly rather than accepting deferred factory callbacks, piecewise setters, or manufacturing arguments on the interface
rubric: design
severity: should_fix
---

**Rule.** When designing APIs that accept interface dependencies, require
callers to construct concrete instances directly and pass them via the abstract
interface. Do not accept deferred factory callbacks, expose piecewise setters to
assemble dependencies internally, or add virtual factory methods onto an
interface to instantiate arguments callers pass back into that same interface;
encapsulate fixed dependencies internally or have callers instantiate concrete
implementations directly.

**Why.** Deferring construction to factory callbacks or piecewise setters
introduces stateful lifecycle complexity to consuming components. Similarly,
placing factory methods on an interface to construct its own injected parameters
contradicts dependency injection: fixed dependencies should be encapsulated
internally, while modular dependencies should be instantiated directly by
callers via subsystem factory functions.

## Examples

Bad:

```cpp
// Anti-pattern 1: deferred factory callbacks and piecewise setters
using ClientUnicodeFactory = sk_sp<SkUnicode> (*)(...);
static std::unique_ptr<ParagraphBuilder> Make(..., ClientUnicodeFactory factory);
builder->setWordsUtf8(words);

// Anti-pattern 2: interface factory for its own injected parameter
class SkShaper {
public:
    virtual void shape(..., ScriptRunIterator&, ...) const = 0;
    virtual std::unique_ptr<ScriptRunIterator> makeScriptRunIterator(...) const = 0;
};
```

Good:

```cpp
// Direct dependency injection: caller constructs instance directly
sk_sp<SkUnicode> unicode = SkUnicodes::Client::Make(words, graphemeBreaks, lineBreaks);
auto builder = ParagraphBuilder::Make(style, fonts, std::move(unicode));

// Caller instantiates concrete implementation directly, keeping interface clean
class SkShaper {
public:
    virtual void shape(..., ScriptRunIterator&, ...) const = 0;
};
auto iter = SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes);
shaper->shape(..., *iter, ...);
```
