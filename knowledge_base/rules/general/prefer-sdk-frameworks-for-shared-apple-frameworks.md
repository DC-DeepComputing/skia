---
name: skia/prefer-sdk-frameworks-for-shared-apple-frameworks
description: In Bazel skia_objc_library targets, use sdk_frameworks for shared Apple frameworks instead of duplicating them across ios_frameworks and mac_frameworks
rubric: readability
severity: should_fix
---

**Rule.** In Bazel `skia_objc_library` (and `skia_objc_library_with_testutil`)
targets, specify Apple SDK frameworks under `sdk_frameworks` when they apply
across Apple platforms. Do not duplicate identical framework entries across both
`ios_frameworks` and `mac_frameworks`; reserve `ios_frameworks` and
`mac_frameworks` strictly for platform-specific frameworks.

**Why.** `skia_objc_library` forwards `sdk_frameworks` directly to the
underlying Apple library rule, whereas `ios_frameworks` and `mac_frameworks`
generate platform-conditional `select()` branches. Using `sdk_frameworks` for
shared frameworks eliminates duplicate declarations and unnecessary `select()`
resolution during target analysis.

## Examples

Bad:

```cpp
skia_objc_library(
    name = "audioplayer_apple",
    srcs = ["SkAudioPlayer_mac.mm"],
    hdrs = ["SkAudioPlayer.h"],
    ios_frameworks = ["AVFoundation"],
    mac_frameworks = ["AVFoundation"],
    deps = ["//:core"],
)
```

Good:

```cpp
skia_objc_library(
    name = "audioplayer_apple",
    srcs = ["SkAudioPlayer_mac.mm"],
    hdrs = ["SkAudioPlayer.h"],
    sdk_frameworks = ["AVFoundation"],
    deps = ["//:core"],
)
```
