---
name: skia/module-bazel/omit-unused-crate-features
description: When declaring Rust dependencies in MODULE.bazel crate.spec, omit unused or redundant crate features to avoid pulling unneeded transitive dependencies
rubric: readability
severity: should_fix
---

**Rule.** In MODULE.bazel crate.spec declarations, only enable the minimal set
of Cargo features strictly required by Skia targets, and set default_features =
False when default crate features are not needed. Do not enable redundant
features whose functionality is unused or already supplied by a dedicated direct
crate dependency (such as enabling the 'png' feature on 'image' when Skia
already depends on 'png').

**Why.** Enabling unneeded crate features pulls in extraneous transitive
dependencies, bloats build graphs, increases compilation times, and risks binary
size regressions. Explicitly scoping features to the minimal necessary set
prevents dependency duplication and keeps third-party crate footprints minimal.

## Examples

Bad:

```cpp
crate.spec(
    default_features = False,
    features = [
        "bmp",
        "ico",
        "png",  # Redundant: Skia already depends directly on the 'png' crate
    ],
    git = "https://github.com/image-rs/image.git",
    package = "image",
    rev = "5d0418d0eff747c829d52738c092581312f775c9",
)
```

Good:

```cpp
crate.spec(
    default_features = False,
    features = [
        "bmp",
        "ico",
    ],
    git = "https://github.com/image-rs/image.git",
    package = "image",
    rev = "5d0418d0eff747c829d52738c092581312f775c9",
)
```
