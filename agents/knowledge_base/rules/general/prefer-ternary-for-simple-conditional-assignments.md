---
name: skia/prefer-ternary-for-simple-conditional-assignments
description: When assigning or returning values across two branches, prefer concise ternary expressions for simple conditions, but use standard if-else for awkward multi-line assignments
rubric: readability
severity: should_fix
---

**Rule.** Prefer a concise ternary expression (`cond ? a : b`) over an
`if`/`else` or `if`/`return` construct when initializing a variable, assigning a
value, or selecting between two return values based on a simple condition.
Declare local variables as `const` at initialization whenever ternary assignment
makes immediate definition possible. Conversely, prefer standard `if`/`else`
statements over multi-line ternary assignments when conditional assignments
become awkward or difficult to parse across multiple lines. Additionally, do not
re-evaluate a condition using a ternary operator when an `if` block checking
that same condition already exists nearby; initialize the variable with its
default value upfront and update it inside the existing `if` block alongside
related state updates.

**Why.** Concise ternary expressions eliminate repetitive assignment targets,
if/return boilerplate, and unnecessary vertical branching for simple selections,
while enabling const initialization. However, splitting awkward or long
conditional assignments across multiple lines obscures control flow and makes
code harder to visually scan than straightforward if/else blocks; similarly,
re-evaluating conditions in a subsequent ternary when an enclosing if block
already exists scatters related state mutations.

## Examples

Bad:

```cpp
// Standalone two-branch assignment using verbose if/else:
if (vendorID == kAMD_VkVendor) {
    fMaxVertexAttributes = 32;
} else {
    fMaxVertexAttributes = deviceLimits.maxVertexInputAttributes;
}

// Verbose two-branch return selection:
if (kernelSize <= kMediumKernelSize) {
    return kMediumKernelSize;
}
return kMaxKernelSize;

// Redundant ternary re-checking condition from preceding if block:
if (rpMayReadDstAsInput) {
    stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    access |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
}
VkImageLayout layout = rpMayReadDstAsInput ? VK_IMAGE_LAYOUT_GENERAL
                                           : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

// Awkward multi-line ternary assignment:
glyphInfo['dir'] = glyphInfo['dir'] == 0
  ? CanvasKit.TextDirection.RTL
  : CanvasKit.TextDirection.LTR;
```

Good:

```cpp
// Concise ternary for standalone two-branch assignment:
fMaxVertexAttributes = vendorID == kAMD_VkVendor ? 32 : deviceLimits.maxVertexInputAttributes;

// Concise ternary for simple two-value return selection:
return kernelSize <= kMediumKernelSize ? kMediumKernelSize : kMaxKernelSize;

// Consolidate mutation into existing conditional block with default initialization:
VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
if (rpMayReadDstAsInput) {
    stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    access |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    layout = VK_IMAGE_LAYOUT_GENERAL;
}

// Straightforward if/else for multi-line conditional assignment:
if (glyphInfo['dir'] === 0) {
    glyphInfo['dir'] = CanvasKit.TextDirection.RTL;
} else {
    glyphInfo['dir'] = CanvasKit.TextDirection.LTR;
}
```
