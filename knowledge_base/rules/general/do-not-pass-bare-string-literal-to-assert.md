---
name: skia/do-not-pass-bare-string-literal-to-assert
description: When asserting conditions or signaling failures with SkASSERT, do not pass bare string literals; use SkDEBUGFAIL, SK_ABORT, or SkASSERTF instead
rubric: correctness
severity: should_fix
---

**Rule.** Do not pass a bare string literal as the condition to assertion macros
like SkASSERT() or SkASSERT_RELEASE(). Because string literals decay to non-null
pointers in C++, the condition evaluates to true and the assertion will never
fail. To unconditionally signal failure, use SkDEBUGFAIL("message") or
SK_ABORT("message"), and use SkASSERTF(condition, "message", ...) when asserting
a condition with an explanatory formatted message.

**Why.** In C++, a string literal decays to const char*, which contextually
evaluates to true in a boolean expression. An assertion macro like
SkASSERT("message") is therefore a tautological pass that compiles away to a
no-op even in debug builds, masking incomplete functionality or unreachable
paths instead of failing fast.

## Examples

Bad:

```cpp
// Bad: bare string literal decays to non-null pointer and always evaluates to true
SkString toUpper(const SkString& str, const char* locale) override {
    SkASSERT("toUpper is not implemented");
    return SkString(fData->fText8.data(), fData->fText8.size());
}

// Bad: assertion condition is a tautological string literal
void processInput(const void* data, size_t size) {
    if (!data) {
        SkASSERT("data pointer must not be null");
    }
}
```

Good:

```cpp
// Good: fail fast with dedicated debug failure or fatal abort macro
SkString toUpper(const SkString& str, const char* locale) override {
    SkDEBUGFAIL("toUpper is not implemented"); // or SK_ABORT("Not implemented.");
    return SkString(fData->fText8.data(), fData->fText8.size());
}

// Good: use SkASSERT with boolean condition or SkASSERTF with formatted message
void processInput(const void* data, size_t size) {
    SkASSERTF(data != nullptr, "data pointer must not be null (size: %zu)", size);
}
```
