---
name: skia/gpu/document-raii-scope-exit-callback-execution
description: When documenting immediate callback execution that relies on RAII wrappers like RefCntedCallback or AutoCallback, state that execution occurs upon scope exit
rubric: readability
severity: nit
---

**Rule.** When relying on RAII wrappers (such as RefCntedCallback or
AutoCallback) to trigger callbacks or cleanup actions implicitly on destruction,
explicitly document in explanatory comments that invocation occurs when the
wrapper goes out of scope. Avoid phrasing comments as though an immediate
function call is performed when no explicit call is made in the branch.

**Why.** In C++, RAII wrappers invoke callbacks or release procs inside their
destructors rather than through explicit call sites. Stating in comments that a
callback 'fires immediately' without noting that it triggers upon scope exit
confuses readers and maintainers who expect an explicit call or suspect missing
logic.

## Examples

Bad:

```cpp
// If a finish proc was provided, attach it to the most recent outstanding submission,
// or let it fire immediately if the GPU is idle.
if (callback) {
    if (!fOutstandingSubmissions.empty()) {
        fOutstandingSubmissions.back()->addFinishInfo(std::move(callback));
    }
    return true;
}
```

Good:

```cpp
// If a finish proc was provided, attach it to the most recent outstanding submission,
// or let it fire immediately if the GPU is idle (when callback goes out of scope).
if (callback) {
    if (!fOutstandingSubmissions.empty()) {
        fOutstandingSubmissions.back()->addFinishInfo(std::move(callback));
    }
    return true;
}
```
