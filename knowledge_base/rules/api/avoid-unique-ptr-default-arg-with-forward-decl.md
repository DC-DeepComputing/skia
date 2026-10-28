---
name: skia/api/avoid-unique-ptr-default-arg-with-forward-decl
description: When declaring functions with std::unique_ptr<T> parameters in headers where T is forward-declared, avoid default arguments like nullptr; provide overloaded functions instead
rubric: correctness
severity: blocking
---

**Rule.** Do not use default arguments for std::unique_ptr<T> parameters (such
as = nullptr or = {}) in header declarations where T is only forward-declared.
Either #include the header defining T or provide separate function overloads
with and without the parameter.

**Why.** Evaluating and destructing a default std::unique_ptr<T> argument at
caller call sites instantiates std::default_delete<T>, requiring T to be a
complete type. If T is only forward-declared, callers invoking the function with
the default argument will fail to compile. Providing overloaded declarations
avoids forcing an #include of T into the header while preserving caller
convenience and respecting Skia's minimal includes policy.

## Examples

Bad:

```cpp
class SkFontScanner;
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider,
                                              std::unique_ptr<SkFontScanner> scanner = nullptr);
```

Good:

```cpp
class SkFontScanner;
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider,
                                              std::unique_ptr<SkFontScanner> scanner);
SK_API sk_sp<SkFontMgr> SkFontMgr_New_Fuchsia(fuchsia::fonts::ProviderSyncPtr provider);
```
