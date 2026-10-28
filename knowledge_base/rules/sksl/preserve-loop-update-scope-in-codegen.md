---
name: skia/sksl/preserve-loop-update-scope-in-codegen
description: When lowering loops in shader codegen, fall back to dedicated continuing blocks if condition or update expressions emit scratch variables or comma expressions
rubric: correctness
severity: should_fix
---

**Rule.** When lowering loops in shader backend code generators, verify that
loop condition and update expressions can execute cleanly inside the target
language's native for-loop header without hoisting scratch variables or
misplacing updates. If condition or update expressions generate preparatory
scratch variables (such as temporary let bindings), or if update expressions
contain multiple comma-separated statements that cannot be placed in the native
update clause, do not hoist scratch variables before the loop or append updates
to the loop body; fall back to a dynamic loop construct with per-iteration
evaluation and dedicated update scopes (such as WGSL 'loop { continuing { ... }
}').

**Why.** In shader backends like WGSL, emitting preparatory scratch statements
before a native for-loop causes condition or update expressions to be evaluated
only once before loop entry, producing stale loop variable values or infinite
loops. Similarly, appending update expressions to the loop body causes them to
be shadowed by inner body declarations or bypassed whenever 'continue' executes.
Using a loop construct with per-iteration evaluation and an isolated continuing
block guarantees correct scoping and iteration advance.

## Examples

Bad:

```cpp
// 1. Emitting scratch variables for condition or update before a native for-loop:
std::string condStr = this->assembleExpression(*s.test(), Precedence::kExpression);
this->write("for (; " + condStr + "; ) {"); // Bug: scratch 'let' evaluated only once!

// 2. Appending comma-separated updates to the loop body:
for (; cond; first_next) {
    if (shouldSkip) continue; // Bug: remaining_next bypassed; also shadowed by body vars
    remaining_next;
}
```

Good:

```cpp
int scratchBefore = fScratchCount;
AutoOutputStream captureCond(this, &condBuf, &fIndentation);
std::string condStr = this->assembleExpression(*s.test(), Precedence::kExpression);
bool needsLoopForm = hasCommaNext || (fScratchCount != scratchBefore);

if (!needsLoopForm) {
    this->writeLine("for (; " + condStr + "; " + nextStr + ") {");
} else {
    // Fall back to loop with per-iteration condition and continuing block:
    // loop { cond-prep; if (cond) { body; } else { break; } continuing { next; } }
}
```
