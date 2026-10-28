---
name: skia/avoid-overloaded-domain-terms-in-comments
description: When documenting expressions, types, or algorithms in code comments, avoid domain-overloaded terms like 'regular expression'; use unambiguous descriptors like 'standard' or omit modifiers
rubric: readability
severity: nit
---

**Rule.** Do not use domain-overloaded technical terminology (such as 'regular
expression' when referring to an ordinary or standard expression) as informal
descriptors in code comments. Use unambiguous modifiers such as 'standard' or
'normal', or omit the modifier entirely when documenting AST nodes, expressions,
and data structures.

**Why.** In compiler and computer science codebases, terms like 'regular
expression' carry specific formal semantics (such as finite automata for regex
parsing). Using technical terms colloquially to mean 'ordinary' creates
ambiguity about whether pattern matching or standard expression evaluation is
intended.

## Examples

Bad:

```cpp
// Pass parameters to the helper so that regular expression-tree processing
// knows to correctly dereference them.
```

Good:

```cpp
// Pass parameters to the helper so that expression-tree processing
// knows to correctly dereference them.
```
