---
name: skia/build-bazel/restrict-bazel-visibility-outside-root
description: When configuring Bazel visibility or exposing public targets, restrict internal targets to :__pkg__, omit visibility on local helpers, and alias public APIs in root BUILD.bazel
rubric: design
severity: should_fix
---

**Rule.** Do not set `visibility = ["//visibility:public"]` on Bazel targets
outside the root `//BUILD.bazel`, and do not grant repo-wide visibility
(`["//:__subpackages__"]`) to internal targets. To expose an internal target in
Skia's public Bazel API, set its visibility to `["//:__pkg__"]` and define a
public `alias` in root `//BUILD.bazel`. For targets consumed only within their
defining package, omit `visibility` entirely, and never list a target's own
package. When exposing targets across non-root packages, scope visibility
strictly to `//<pkg>:__pkg__` instead of `//<pkg>:__subpackages__`, and depend
on root public aliases (e.g., `//:core`).

**Why.** In Bazel, targets without an explicit `visibility` attribute default to
package-private. Exposing public targets via aliases in the root `//BUILD.bazel`
while restricting implementation targets to `//:__pkg__` keeps Skia's public
build surface centralized and prevents external dependencies on internal
targets. Setting public or repo-wide (`//:__subpackages__`) visibility on
internal targets leaks implementation details and violates least privilege.

## Examples

Bad:

```cpp
# src/encode/BUILD.bazel
# Bad: public visibility outside root //BUILD.bazel or omitting root alias
skia_cc_library(
    name = "rust_png_encoder",
    visibility = ["//visibility:public"],
)

# Bad: repo-wide visibility on internal filegroup
skia_filegroup(
    name = "rs_srcs",
    srcs = ["FFI.rs"],
    visibility = ["//:__subpackages__"],
)

# Bad: using :__subpackages__ or listing target's own package
skia_cc_library(
    name = "utils",
    visibility = ["//src/codec:__subpackages__", "//src/encode:__pkg__"],
)
```

Good:

```cpp
# src/encode/BUILD.bazel
# Good: scope public target to root package, then alias in root BUILD.bazel
skia_cc_library(
    name = "rust_png_encoder",
    visibility = ["//:__pkg__"],
)

# BUILD.bazel
alias(
    name = "png_encode_rust",
    actual = "//src/encode:rust_png_encoder",
    visibility = ["//visibility:public"],
)

# Good: omit visibility on internal helpers; scope cross-package targets to :__pkg__
skia_filegroup(
    name = "rs_srcs",
    srcs = ["FFI.rs"],
)
skia_cc_library(
    name = "utils",
    visibility = ["//src/codec:__pkg__"],
)
```
