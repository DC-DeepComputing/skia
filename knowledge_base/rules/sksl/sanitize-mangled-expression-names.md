---
name: skia/sksl/sanitize-mangled-expression-names
description: When deriving or mangling identifier names from expression descriptions in SkSL code generators, sanitize all non-alphanumeric characters rather than select punctuation
rubric: correctness
severity: nit
---

**Rule.** When mangling, synthesizing, or deriving identifier and symbol names
(such as specialized function names or debug OpNames) from expression
descriptions in SkSL backend code generators, sanitize all non-alphanumeric
characters (e.g., replacing them with underscores via std::replace_if and
!isalnum). Do not replace only select characters (like dots), as expression
descriptions frequently contain brackets, arrows, and operator symbols.

**Why.** Expression descriptions for complex IR nodes (such as array indexing or
field accesses) contain diverse punctuation characters. Real-world downstream
shader compilers and GPU drivers can crash, reject shaders, or produce naming
collisions if mangled function or symbol names contain non-identifier
characters.

## Examples

Bad:

```cpp
std::string name = expr->description();
std::replace(name.begin(), name.end(), '.', '_');
mangledName += "_" + name;
```

Good:

```cpp
std::string name = expr->description();
std::replace_if(name.begin(), name.end(), [](char c) { return !isalnum(c); }, '_');
mangledName += "_" + name;
```
