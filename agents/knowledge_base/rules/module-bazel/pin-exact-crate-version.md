---
name: skia/module-bazel/pin-exact-crate-version
description: In MODULE.bazel crate.spec declarations, pin published crate versions using an explicit '=' prefix instead of caret requirements or git revisions
rubric: readability
severity: should_fix
---

**Rule.** In MODULE.bazel crate.spec declarations, pin exact crate versions
using an explicit '=' prefix (e.g., version = "=0.18.0-rc") rather than git
repository URLs and commit revisions when a published crate is available, and do
not omit the '=' prefix.

**Why.** Cargo SemVer requirements in rules_rust crate.spec default to caret
semantics (^), allowing Cargo and Bazel to resolve newer compatible releases or
release candidates. Prefixing the version with '=' enforces an exact version
match and avoids unwanted dependency drift without resorting to git pinning for
published crates.

## Examples

Bad:

```cpp
crate.spec(
    package = "png",
    git = "https://github.com/image-rs/image-png",
    rev = "eb9b5d7f371b88f15aaca6a8d21c58b86c400d76",
)
```

Good:

```cpp
crate.spec(
    package = "png",
    version = "=0.18.0-rc",
)
```
