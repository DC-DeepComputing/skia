---
name: skia/name-query-helpers-after-specific-property
description: When defining query or helper functions that calculate properties of an object or construct, name them after the specific attribute evaluated rather than generic terms
rubric: readability
severity: nit
---

**Rule.** Name query, inspection, and analysis helper functions after the
specific attribute, metric, or property they evaluate (such as returnComplexity
or astNodeCount) rather than using overly broad or generic names (such as bare
complexity or size). When a construct or entity can exhibit multiple distinct
dimensions or properties of that category, explicitly qualify the identifier to
reflect the concrete concept evaluated.

**Why.** Generic names like complexity or count are ambiguous because a
construct (such as an AST node, function definition, or expression) can possess
multiple independent metrics, such as cyclomatic complexity, return complexity,
or nesting depth. Naming the function after the specific property or attribute
being evaluated clarifies caller intent, avoids misinterpreting what metric is
measured, and self-documents call sites.

## Examples

Bad:

```cpp
// Ambiguous: does not specify what form of complexity is evaluated
Analysis::ReturnComplexity complexity(const FunctionDefinition* func) {
    Analysis::ReturnComplexity* cached = fReturnComplexityMap.find(func);
    if (!cached) {
        cached = fReturnComplexityMap.set(func, Analysis::GetReturnComplexity(*func));
    }
    return *cached;
}
```

Good:

```cpp
// Descriptive: explicitly names the specific property and return type evaluated
Analysis::ReturnComplexity returnComplexity(const FunctionDefinition* func) {
    Analysis::ReturnComplexity* cached = fReturnComplexityMap.find(func);
    if (!cached) {
        cached = fReturnComplexityMap.set(func, Analysis::GetReturnComplexity(*func));
    }
    return *cached;
}
```
