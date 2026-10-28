---
name: skia/module-bazel/document-third-party-patch-removal-todo
description: When applying temporary downstream patches or workarounds to third-party dependencies in build files, include a TODO specifying the removal condition
rubric: readability
severity: nit
---

**Rule.** When applying temporary downstream patches or workarounds to
third-party dependencies in build configuration files (such as MODULE.bazel,
WORKSPACE.bazel, or under bazel/), accompany the patch declaration with an
attributed TODO comment specifying the condition for removal (such as an
upstream PR merge or a new release).

**Why.** Downstream patches against third-party dependencies introduce ongoing
maintenance debt and risk diverging from upstream. Documenting temporary patches
with an attributed TODO comment ensures the rationale is tracked and prevents
orphaned patches from lingering after an upstream release incorporates the fix.

## Examples

Bad:

```cpp
crate.annotation(
    crate = "png",
    patch_args = ["-p1"],
    # Patches from https://crrev.com/c/5922033:
    # New public API for `png` crate: `fn next_frame_info`.
    patches = ["//bazel/external/image-png:captured-chunks.patch"],
)
```

Good:

```cpp
crate.annotation(
    crate = "png",
    patch_args = ["-p1"],
    # Patches from https://crrev.com/c/5922033:
    # New public API for `png` crate: `fn next_frame_info`.
    #
    # TODO(username): Remove this patch once a new version of `png`
    # (>= 0.18.2) is released incorporating upstream PR #123.
    patches = ["//bazel/external/image-png:captured-chunks.patch"],
)
```
