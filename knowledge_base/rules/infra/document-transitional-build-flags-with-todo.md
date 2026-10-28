---
name: skia/infra/document-transitional-build-flags-with-todo
description: When configuring transitional or asymmetric build flags during feature migrations in bot recipes, document the planned cutover with a TODO(bug) comment
rubric: readability
severity: nit
---

**Rule.** When configuring transitional or asymmetric build flags during
incremental feature migrations in bot recipes (infra/bots/), document the
intended follow-up with an attributed TODO comment citing the tracking bug
(e.g., '# TODO(b/<id>): set <flag> to <val> also'). Explicitly clarify the
remaining cutover steps whenever a new library or feature is enabled before its
legacy counterpart is fully disabled.

**Why.** Incremental library or feature migrations often cannot switch all
complementary components simultaneously (such as enabling a new library for
decode and encode while only disabling legacy decode). Without an explanatory
comment, leaving the legacy counterpart partially enabled appears as an
accidental omission rather than an intentional transitional state; an attributed
TODO documents design intent and tracks remaining cutover work.

## Examples

Bad:

```cpp
if 'RustPNG' in extra_tokens:
    args['skia_use_rust_png_decode'] = 'true'
    args['skia_use_rust_png_encode'] = 'true'
    args['skia_use_libpng_decode'] = 'false'
```

Good:

```cpp
if 'RustPNG' in extra_tokens:
    args['skia_use_rust_png_decode'] = 'true'
    args['skia_use_rust_png_encode'] = 'true'
    args['skia_use_libpng_decode'] = 'false'
    # TODO(b/356875275): set skia_use_libpng_encode to false also
```
