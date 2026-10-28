---
name: skia/build-bazel/declare-companion-config-settings-for-select
description: When referencing package-local labels or :<name>_true/:<name>_false flags in Bazel select() statements, declare the companion config_setting, config_setting_group, or bool_flag target locally
rubric: correctness
severity: blocking
---

**Rule.** When referencing package-local target labels (e.g., `:debug_build`) or
boolean flag settings (`:<name>_true` / `:<name>_false`) in Bazel `select()`
statements or target attributes, ensure the corresponding `config_setting`,
`selects.config_setting_group`, or `bool_flag(name = "<name>", ...)` target is
declared in the same `BUILD.bazel` file. Alternatively, reference the
configuration setting using its fully qualified target label (such as
`"//bazel/common_config_settings:..."`).

**Why.** In Bazel, labels with a leading colon refer to targets defined within
the current package, and Skia's `bool_flag` macro generates package-local
`:<name>_true` and `:<name>_false` `config_setting` targets. Referencing an
undeclared package-local target or missing flag in a `select()` branch causes
build evaluation to fail immediately with an unresolved target error.

## Examples

Bad:

```cpp
# Missing bool_flag declaration for :enable_bidi_true / :enable_bidi_false
select({
    ":enable_bidi_true": ["bidi_bindings.cpp"],
    ":enable_bidi_false": [],
})

# Missing config_setting declaration for package-local label
cc_library(
    name = "user_config",
    defines = select({
        ":debug_build": ["SK_DEBUG"],
        "//conditions:default": ["SK_RELEASE"],
    }),
)
```

Good:

```cpp
# Declaring the companion bool_flag in the same BUILD.bazel file
bool_flag(
    name = "enable_bidi",
    default = False,
)

select({
    ":enable_bidi_true": ["bidi_bindings.cpp"],
    ":enable_bidi_false": [],
})

# Declaring the companion config_setting in the same BUILD.bazel file
config_setting(
    name = "debug_build",
    values = {"compilation_mode": "dbg"},
)

cc_library(
    name = "user_config",
    defines = select({
        ":debug_build": ["SK_DEBUG"],
        "//conditions:default": ["SK_RELEASE"],
    }),
)
```
