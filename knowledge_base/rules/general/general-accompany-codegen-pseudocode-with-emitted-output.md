---
name: skia/general-accompany-codegen-pseudocode-with-emitted-output
description: When documenting code-generation routines in comments using pseudocode, accompany high-level control flow with concrete examples of actual emitted target code or instructions
rubric: readability
severity: nit
---

**Rule.** When documenting code-generation routines, AST lowerings, or
instruction emitters using high-level pseudocode in code comments, accompany the
conceptual logic with a concrete, realistic example of the actual emitted
output. For low-level or stack-based targets (such as PostScript, bytecode, or
assembly), show how abstract variable assignments and branches translate into
concrete stack operations or instruction sequences.

**Why.** High-level pseudocode explains conceptual control flow and formulas
using named variables, but obscures how that logic maps onto low-level target
instructions and stack manipulation (such as dup, exch, or pop). Providing a
concrete example of the emitted output allows reviewers and maintainers to
verify instruction sequences and stack balances without manually
mental-compiling the emitter.

## Examples

Bad:

```cpp
/* Generate Type 4 function code to map t to the passed gradient.
   The generated code will be of the following form:
  if (t <= 0) {
    ret = color[0];
    t = 0;
  }
  ...
*/
static void gradient_function_code(const SkShader::GradientInfo& info,
                                   SkDynamicMemoryWStream* result);
```

Good:

```cpp
/* Generate Type 4 function code to map t to the passed gradient.
   The generated code will be of the following form:
  if (t <= 0) {
    ret = color[0];
    t = 0;
  }
  ...
   which in PDF will be represented like
  dup 0 le {pop 0 0 0 0} if
  dup dup 0 gt exch 1 le and {
    dup .5 le {
      ...
    } ifelse
  0} if
*/
static void gradient_function_code(const SkShader::GradientInfo& info,
                                   SkDynamicMemoryWStream* result);
```
