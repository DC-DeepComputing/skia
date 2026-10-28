---
name: skia/sksl/save-and-restore-state-in-recursive-codegen
description: When emitting subexpressions in recursive code generators, save and restore generator member state rather than unconditionally clearing or resetting it
rubric: correctness
severity: should_fix
---

**Rule.** When generating code for child expressions or statements in recursive
code generators, save and restore generator member state (such as tracking sets,
temporary buffers, or context flags) across recursive calls—either via RAII
guards or by moving state into local variables and restoring upon return—rather
than unconditionally resetting or clearing member state. Ensure nested or
reentrant subexpression evaluations do not clobber or prematurely wipe out outer
caller context.

**Why.** Evaluating nested subexpressions (such as nested function calls in
arguments or index expressions) can reenter code generation methods that mutate
or clear generator member state. If member state is unconditionally cleared upon
exit instead of saved and restored, an inner recursive call prematurely wipes
out outer tracking state, causing corrupted code emission or assertion failures.

## Examples

Bad:

```cpp
// In writeOutParamHelper:
// fOutParamArgVars populated for current helper...
for (const auto& arg : args) {
    this->writeExpression(*arg, Precedence::kAssignment);
}
// Evaluating arg can recursively reenter writeOutParamHelper (e.g. nested calls);
// unconditionally resetting wipes out the outer caller's state:
fOutParamArgVars.reset();
```

Good:

```cpp
// In writeOutParamHelper:
// Save outer caller state before populating for current helper scope:
auto parentOutParamArgVars = std::move(fOutParamArgVars);
SkASSERT(fOutParamArgVars.empty());

for (const auto& arg : args) {
    this->writeExpression(*arg, Precedence::kAssignment);
}
// Restore outer state upon exiting scope:
fOutParamArgVars = std::move(parentOutParamArgVars);
```
