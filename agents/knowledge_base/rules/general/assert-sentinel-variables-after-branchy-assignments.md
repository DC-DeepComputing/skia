---
name: skia/assert-sentinel-variables-after-branchy-assignments
description: When initializing variables to sentinel or default values before multi-branch assignment blocks, assert with SkASSERT that non-failing paths populated a valid value
rubric: readability
severity: should_fix
---

**Rule.** When initializing a variable to a default or sentinel value (such as 0
or nullptr) prior to multi-branch conditional blocks (such as if/else or switch
statements), assert with SkASSERT immediately after the branching structure that
the variable was assigned a valid value along all non-failing paths. Do not
allow sentinel-initialized variables to propagate downstream without verifying
that at least one branch populated them.

**Why.** Default or sentinel initializations prevent uninitialized-variable
compiler warnings, but they risk masking unhandled or newly added branches that
omit assignment. Verifying the populated state with SkASSERT immediately
following the branch structure ensures incomplete coverage fails fast in debug
builds rather than silently passing invalid sentinel values downstream.

## Examples

Bad:

```cpp
int numComponents = 0;
if (srcCT == kRGB_888x_SkColorType) {
    numComponents = 4;
} else {
    switch (SkColorTypeNumChannels(srcCT)) {
        case 1: numComponents = 1; break;
        case 3: numComponents = 4; break;
        case 4: numComponents = 4; break;
        default: return false;
    }
}
fCInfo.input_components = numComponents;
```

Good:

```cpp
int numComponents = 0;
if (srcCT == kRGB_888x_SkColorType) {
    numComponents = 4;
} else {
    switch (SkColorTypeNumChannels(srcCT)) {
        case 1: numComponents = 1; break;
        case 3: numComponents = 4; break;
        case 4: numComponents = 4; break;
        default: return false;
    }
}
SkASSERT(numComponents != 0);
fCInfo.input_components = numComponents;
```
