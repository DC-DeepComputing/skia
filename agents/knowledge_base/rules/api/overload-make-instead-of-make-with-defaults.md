---
name: skia/api/overload-make-instead-of-make-with-defaults
description: When defining factory methods for constructing objects with default options, overload Make() rather than creating separate suffixed functions like MakeWithDefaults()
rubric: readability
severity: should_fix
---

**Rule.** Prefer providing a parameterless Make() factory overload (or
defaulting the options parameter) rather than introducing a separate factory
function with a descriptive suffix like MakeWithDefaults() or MakeDefault() for
constructing an object with default options.

**Why.** In Skia C++ APIs, static factory methods canonically use function
overloading under the uniform name Make(...). Providing a no-argument Make()
overload that forwards to Make({}) keeps the API surface clean, concise, and
consistent with project-wide idioms while avoiding redundant and verbose variant
names.

## Examples

Bad:

```cpp
class Context {
public:
    static std::unique_ptr<const Context> Make(const Options&);
    static std::unique_ptr<const Context> MakeWithDefaults();
};
```

Good:

```cpp
class Context {
public:
    static std::unique_ptr<const Context> Make(const Options&);
    static std::unique_ptr<const Context> Make();
};
```
