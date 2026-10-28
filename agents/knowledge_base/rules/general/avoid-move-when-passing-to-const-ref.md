---
name: skia/avoid-move-when-passing-to-const-ref
description: When passing arguments to functions taking const reference parameters, omit std::move to avoid misleading no-op moves and apparent use-after-move bugs
rubric: readability
severity: should_fix
---

**Rule.** Do not wrap arguments in std::move() when passing to functions or
helpers that take parameters by const reference (const T&) or const value,
especially when the argument continues to be used later in the caller. Pass
arguments directly without std::move().

**Why.** std::move() produces an rvalue reference that binds to const T& without
moving any resources, making it an ineffective no-op. Using std::move() here
misleadingly implies transfer of ownership, creates false use-after-move
concerns when the variable is accessed afterward, and risks silent bugs if the
callee is later refactored to take by value.

## Examples

Bad:

```cpp
// Bad: extract_cff_data takes `const HBFace&`; std::move is a no-op that looks like a use-after-move
if (sk_sp<SkData> cffData = extract_cff_data(std::move(subset))) {
    return cffData;
}
HBBlob result(hb_face_reference_blob(subset.get()));
```

Good:

```cpp
// Good: Pass directly without std::move since the callee only borrows the object
if (sk_sp<SkData> cffData = extract_cff_data(subset)) {
    return cffData;
}
HBBlob result(hb_face_reference_blob(subset.get()));
```
