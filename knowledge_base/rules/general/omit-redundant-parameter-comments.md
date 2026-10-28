---
name: skia/omit-redundant-parameter-comments
description: When annotating call-site arguments, omit redundant parameter comments on self-describing expressions and format required comments as /*param=*/arg
rubric: readability
severity: nit
---

**Rule.** Avoid redundant inline parameter name comments (e.g., /*param=*/) when
an argument's semantic role is already self-evident from variable names, member
field accesses, descriptive helper calls, or adjacent test case description
strings. When an in-scope variable corresponds to a parameter, pass that
variable directly rather than passing a commented literal like /*param=*/true.
When inline parameter comments are needed for ambiguous literals (such as
booleans, numeric constants, nullptr, or empty braces), format them as
'/*param=*/arg' including the equals sign rather than bare '/*param*/'.

**Why.** Inline parameter comments clarify ambiguous call-site literals and
enable tooling (such as clang-tidy bugprone-argument-comment or IDE inspections)
to validate argument names against declarations when formatted as /*param=*/.
Conversely, annotating self-describing arguments or passing commented literals
instead of in-scope variables adds visual clutter without conveying new
information.

## Examples

Bad:

```cpp
// Redundant comments on self-describing arguments or test cases
VULKAN_CALL(interface,
            CmdSetStencilCompareMask(commandBuffer,
                                     VK_STENCIL_FACE_FRONT_BIT,
                                     /*compareMask=*/frontStencil.fReadMask));
test("base-1, target-0", /*targetedHdrHeadroom=*/0.f, params);

// Passing commented literal when matching in-scope variable exists
const bool strict = SkToBool(analysis & BoundsAnalysis::kRequiresShaderTiling);
if (strict && decalClampToTransparent) {
    imageShader = fImage->makePixelOutset()->asShader(tileMode, sampling, preDecal,
                                                      /*strict=*/true);
}

// Missing '=' in inline parameter comment
paint.setColor4f(SkColors::kTransparent, /*colorSpace*/nullptr);
```

Good:

```cpp
// Omit comments when argument expression or test label is self-evident
VULKAN_CALL(interface,
            CmdSetStencilCompareMask(commandBuffer,
                                     VK_STENCIL_FACE_FRONT_BIT,
                                     frontStencil.fReadMask));
test("base-1, target-0", 0.f, params);

// Pass in-scope variable directly
const bool strict = SkToBool(analysis & BoundsAnalysis::kRequiresShaderTiling);
if (strict && decalClampToTransparent) {
    imageShader = fImage->makePixelOutset()->asShader(tileMode, sampling, preDecal,
                                                      strict);
}

// Format inline parameter comments with '='
paint.setColor4f(SkColors::kTransparent, /*colorSpace=*/nullptr);
```
