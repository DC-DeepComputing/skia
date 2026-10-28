---
name: skia/const-variable-internal-linkage
description: When declaring namespace- or file-scope const variables in C++ files, do not require adding 'static' as non-extern const objects have internal linkage by default
rubric: readability
severity: nit
---

**Rule.** Do not require adding 'static' to namespace- or file-scope 'const' (or
'constexpr') variables to achieve internal linkage. In C++, non-extern and
non-inline const variables declared at namespace scope already have internal
linkage by default, making 'static' redundant for symbol isolation.

**Why.** In C++, namespace-scope const variables that are not explicitly
declared 'extern' or 'inline' automatically receive internal linkage according
to the language standard ([basic.link]). Requesting or requiring 'static' on
file-scope const data is unnecessary and reflects C rather than C++ linkage
semantics.

## Examples

Bad:

```cpp
// Reviewer comment: "Please add 'static' so this doesn't pollute the global symbol table"
static const SkPathVerb gTriangle_Verbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kClose,
};
```

Good:

```cpp
// Correct: const variables at namespace scope already have internal linkage by default in C++.
const SkPathVerb gTriangle_Verbs[] = {
    SkPathVerb::kMove,
    SkPathVerb::kLine,
    SkPathVerb::kLine,
    SkPathVerb::kClose,
};
```
