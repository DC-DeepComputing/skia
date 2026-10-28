---
name: skia/document-workarounds-and-compiler-quirks
description: When implementing non-obvious workarounds, compiler-quirk mitigations, or selective transformations to bypass toolchain limitations, document the rationale and tracking bug in an inline comment
rubric: readability
severity: nit
---

**Rule.** When implementing non-obvious workarounds or compiler- and
driver-quirk mitigations—such as selectively transforming only a single operand,
argument, or expression to force runtime evaluation and evade compile-time
validation errors—document the rationale with an explanatory inline comment and
cite the tracking bug. Explicitly explain why the workaround is necessary, why
specific operands are targeted, and what failure mode or crash it prevents so
future maintainers do not revert or refactor the workaround as arbitrary code.

**Why.** Selective transformations and compiler mitigations (such as wrapping
only the first operand in a scratch let-variable to prevent compile-time
overflow errors) appear arbitrary, incomplete, or redundant in isolation.
Documenting the specific quirk, failure mode, and tracking bug prevents
maintainers from simplifying or removing the workaround during refactoring.

## Examples

Bad:

```cpp
// Selective transformation without context or tracking bug
std::string argument = this->assembleExpression(*args[index], Precedence::kSequence);
expr += (allConstant && index == 0) ? this->writeScratchLet(argument)
                                    : argument;
```

Good:

```cpp
// We can use a scratch-let for argument 0 to dodge WGSL overflow errors. (skia:14385)
std::string argument = this->assembleExpression(*args[index], Precedence::kSequence);
expr += (allConstant && index == 0) ? this->writeScratchLet(argument)
                                    : argument;
```
