---
name: skia/sksl/check-makelvalue-for-null-in-codegen
description: In SkSL backend code generators, when creating an lvalue via makeLValue(), check the returned std::unique_ptr<LValue> for null and bail out before dereferencing
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL backend code generators, always null-check the
std::unique_ptr<LValue> returned by makeLValue() before dereferencing it. If
makeLValue() returns nullptr, bail out immediately (e.g., return an empty string
or error status) rather than calling store() or load().

**Why.** In SkSL code generators, makeLValue() emits a compilation error and
returns nullptr when an expression is not a supported assignable lvalue.
Dereferencing the returned pointer without checking for null leads to a null
pointer dereference crash during code generation.

## Examples

Bad:

```cpp
auto outLValue = this->makeLValue(*call.arguments()[1]);
this->writeLine(outLValue->store(outValue));
```

Good:

```cpp
std::unique_ptr<LValue> lvalue = this->makeLValue(*call.arguments()[1]);
if (!lvalue) {
    return "";
}
this->writeLine(lvalue->store(outValue));
```
