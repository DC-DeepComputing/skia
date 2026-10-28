---
name: skia/tools/when-writing-or-extending-if-else-if-dispatch-chains-such-as
description: When writing or extending if-else if dispatch chains such as string-to-enum parsers, verify that each branch condition is unique to prevent unreachable dead code
rubric: correctness
severity: should_fix
---

**Rule.** Do not duplicate condition checks across branches in `if`-`else if`
chains (such as string-to-enum dispatchers or CLI argument parsers). Ensure
every branch tests a distinct condition so no branch is rendered unreachable
dead code by preceding checks.

**Why.** A duplicate condition in a subsequent `else if` branch can never
evaluate to true because the earlier matching branch always executes first. This
creates unreachable dead code and usually indicates a copy-paste error where
another intended variant was left unhandled.

## Examples

Bad:

```cpp
if (0 == strcmp(str, "grdawn_vk")) {
    return sk_app::Window::BackendType::kGraphiteDawnVulkan;
} else if (0 == strcmp(str, "grdawn_metal")) {
    return sk_app::Window::BackendType::kGraphiteDawnMetal;
} else if (0 == strcmp(str, "grdawn_vk")) {
    return sk_app::Window::BackendType::kGraphiteDawnVulkan;
}
```

Good:

```cpp
if (0 == strcmp(str, "grdawn_d3d12")) {
    return sk_app::Window::BackendType::kGraphiteDawnD3D12;
} else if (0 == strcmp(str, "grdawn_metal")) {
    return sk_app::Window::BackendType::kGraphiteDawnMetal;
} else if (0 == strcmp(str, "grdawn_vk")) {
    return sk_app::Window::BackendType::kGraphiteDawnVulkan;
}
```
