---
name: skia/prefer-contextual-diff-in-commit-messages
description: When illustrating code, shader, or assembly output transformations in commit messages, provide a unified diff with surrounding context lines rather than an isolated or redundant context-free diff summary
rubric: readability
severity: should_fix
---

**Rule.** When illustrating code, shader, or assembly transformations in commit
messages, provide a unified diff with surrounding context lines rather than an
isolated, context-free diff summary (such as ed-style output). Do not duplicate
both formats in the description; use a single unified diff with context lines to
make the semantic impact clear.

**Why.** A unified diff with surrounding context lines allows reviewers and
future maintainers to immediately understand the semantic location and impact of
a transformation. Context-free diff summaries force readers to mentally
reconstruct where edits occurred, and providing redundant diff formats adds
unnecessary noise without additional clarity.

## Examples

Bad:

```cpp
An example shader shows the following diff of SPIR-V before and after the transformation:

10c11
<                OpEntryPoint Fragment %6 "main" %15
---
>                OpEntryPoint Fragment %6 "main" %15 %37

In context:

                OpMemoryModel Logical GLSL450
-               OpEntryPoint Fragment %6 "main" %15
+               OpEntryPoint Fragment %6 "main" %15 %37
                OpExecutionMode %6 OriginUpperLeft
```

Good:

```cpp
An example shader shows the following diff of SPIR-V before and after the transformation:

                OpMemoryModel Logical GLSL450
-               OpEntryPoint Fragment %6 "main" %15
+               OpEntryPoint Fragment %6 "main" %15 %37
                OpExecutionMode %6 OriginUpperLeft
```
