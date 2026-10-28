---
name: skia/name-predicate-callback-aliases-by-entity-and-condition
description: When defining type aliases for predicate callbacks or filter functions, name them after the evaluated entity and condition rather than vague operations
rubric: readability
severity: nit
---

**Rule.** When defining type aliases for predicate callbacks (callables
returning bool), name the alias to specify the target entity being evaluated and
the match condition (e.g., ParameterMatchesFn). Avoid vague noun phrases or
operational action names (e.g., SpecializationFn) that sound like procedures
that execute a transformation or mutate state.

**Why.** Predicate callbacks return a boolean to indicate whether an argument
satisfies a condition or filter criteria. Naming a predicate alias after an
action or generic noun obscures what argument is being inspected and
misleadingly implies the callback performs an operation rather than evaluating a
match condition.

## Examples

Bad:

```cpp
// Vague noun/action alias sounds like a function that performs specialization
using SpecializationFn = std::function<bool(const Variable&)>;

void FindFunctionsToSpecialize(const Program& program,
                               const SpecializationFn& filter,
                               SpecializationInfo* info);
```

Good:

```cpp
// Explicitly identifies the evaluated entity and matching condition
using ParameterMatchesFn = std::function<bool(const Variable&)>;

void FindFunctionsToSpecialize(const Program& program,
                               const ParameterMatchesFn& matches,
                               SpecializationInfo* info);
```
