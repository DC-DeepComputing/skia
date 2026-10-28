---
name: skia/modules/keep-build-script-status-echo-accurate
description: When adding or modifying configuration branches in build scripts, ensure status echo messages accurately describe active flags rather than retaining copy-pasted text
rubric: readability
severity: should_fix
---

**Rule.** When adding or modifying build configuration branches in build scripts
(such as compile.sh), ensure console status messages (`echo`) accurately
describe the specific configuration flags being set. Do not retain copy-pasted
or stale descriptions from existing branches that contradict active flags or
enabled options.

**Why.** Copy-pasting configuration branches without updating the accompanying
status messages causes misleading console output during compilation. Developers
and CI logs will report features as active (such as HarfBuzz) when the
underlying GN flags actually disable them.

## Examples

Bad:

```cpp
if [[ $@ == *client_bidi* ]] ; then
  echo "Using the client-provided skunicode data and harfbuz instead of the icu-provided data"
  GN_SHAPER="skia_use_icu=false skia_use_client_icu=false skia_use_bidi=true skia_use_harfbuzz=false"
  ENABLE_BIDI="true"
fi
```

Good:

```cpp
if [[ $@ == *client_bidi* ]] ; then
  echo "Using the bidi skunicode data only"
  GN_SHAPER="skia_use_icu=false skia_use_client_icu=false skia_use_bidi=true skia_use_harfbuzz=false"
  ENABLE_BIDI="true"
fi
```
