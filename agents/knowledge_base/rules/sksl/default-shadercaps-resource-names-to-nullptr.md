---
name: skia/sksl/default-shadercaps-resource-names-to-nullptr
description: When declaring optional resource, buffer, or feature identifiers in ShaderCaps, initialize them to nullptr rather than hardcoding default names
rubric: readability
severity: should_fix
---

**Rule.** In ShaderCaps, initialize optional resource names, buffer binding
identifiers, and extension strings to nullptr rather than hardcoding default
string literals. Ensure capabilities that depend on external pipeline setup are
strictly opt-in and explicitly configured by backends or test harnesses.

**Why.** If an optional shader intrinsic or capability depends on external
resources (such as uniform buffers or interface blocks bound by the host
pipeline), providing a non-null default string causes shader code generators to
assume the resource is available. Defaulting identifiers to nullptr prevents
code generators from emitting references to unbound pipeline resources unless
the backend or test harness explicitly opts in and supplies the resource name.

## Examples

Bad:

```cpp
struct ShaderCaps {
    // Bad: Defaulting to a non-null name enables the capability implicitly
    // even when the host pipeline has not bound the required buffer.
    const char* fFloatBufferArrayName = "fsGradientBuffer";
    const char* fFBFetchColorName = "gl_LastFragColor";
};
```

Good:

```cpp
struct ShaderCaps {
    // Good: Initializing to nullptr ensures the feature is strictly opt-in.
    const char* fFloatBufferArrayName = nullptr;
    const char* fFBFetchColorName = nullptr;
};
```
