---
name: skia/omit-parameter-names-in-defaulted-copy-move
description: When declaring defaulted (= default) or deleted (= delete) copy and move constructors and assignment operators in headers, omit parameter names to reduce syntactic noise
rubric: readability
severity: nit
---

**Rule.** Do not include parameter names in defaulted (= default) or deleted (=
delete) copy or move constructor and assignment operator declarations. Leave the
parameter unnamed (such as Foo(const Foo&) = default; or Foo& operator=(const
Foo&) = default;) rather than specifying redundant parameter identifiers.

**Why.** Defaulted and deleted special member functions have no user-written
body that references parameters. Specifying parameter names in these
declarations adds unnecessary visual clutter without conveying semantic meaning,
whereas omitting them keeps class definitions concise and clean.

## Examples

Bad:

```cpp
class FontArguments {
public:
    FontArguments(const FontArguments& args) = default;
    FontArguments& operator=(const FontArguments& rhs) = default;

    FontArguments(FontArguments&& other) = delete;
    FontArguments& operator=(FontArguments&& other) = delete;
};
```

Good:

```cpp
class FontArguments {
public:
    FontArguments(const FontArguments&) = default;
    FontArguments& operator=(const FontArguments&) = default;

    FontArguments(FontArguments&&) = delete;
    FontArguments& operator=(FontArguments&&) = delete;
};
```
