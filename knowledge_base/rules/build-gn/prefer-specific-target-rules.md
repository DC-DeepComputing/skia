---
name: skia/build-gn/prefer-specific-target-rules
description: When declaring GN build targets with a fixed type, invoke specific target rules directly instead of calling generic target(type, name)
rubric: readability
severity: should_fix
---

**Rule.** In GN build files, do not use target("type", "name") when declaring a
fixed target type; call the specific target rule (such as static_library,
source_set, or executable) directly. Reserve target(target_type, target_name)
exclusively for templates where the target type is parameterized dynamically.

**Why.** In GN, target(target_type, target_name) is designed for meta-rules and
templates where target types are resolved at evaluation time. Calling specific
target rules directly for statically known targets is standard GN idiom, makes
build definitions clearer, and enables static tooling inspection.

## Examples

Bad:

```cpp
target("static_library", "skcms") {
  public = skcms_public_headers
  sources = skcms_sources
}
```

Good:

```cpp
static_library("skcms") {
  public = skcms_public_headers
  sources = skcms_sources
}
```
