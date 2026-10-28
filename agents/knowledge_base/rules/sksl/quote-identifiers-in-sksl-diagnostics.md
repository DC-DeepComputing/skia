---
name: skia/sksl/quote-identifiers-in-sksl-diagnostics
description: When writing SkSL compiler diagnostics via ErrorReporter, enclose identifiers, types, and keywords in single quotes, but leave informal natural-language descriptions unquoted
rubric: readability
severity: nit
---

**Rule.** When reporting SkSL compiler diagnostics and errors (such as via
ErrorReporter or context.fErrors->error), enclose variable names, identifiers,
types, and language keywords in single quotes (''). Conversely, do not enclose
informal natural-language descriptions or conceptual phrases in single quotes;
reserve single quotes strictly for literal program text, tokens, and keywords.

**Why.** Single quotes clearly distinguish literal language symbols and keywords
from compiler diagnostic prose. Quoting informal descriptions or concepts
misleads developers into expecting a literal grammar token or keyword, while
leaving actual identifiers unquoted makes diagnostics ambiguous and breaks error
test expectations.

## Examples

Bad:

```cpp
// Leaving identifiers and types unquoted:
context.fErrors->error(pos, std::string(var->name()) + " not supported");
context.fErrors->error(pos, "type " + type.displayName() + " has no field " + field);

// Enclosing informal natural-language descriptions or concepts in quotes:
context.fErrors->error(pos, "'local size' layout qualifiers are only allowed in compute");
```

Good:

```cpp
// Enclosing identifiers, types, and keywords in single quotes:
context.fErrors->error(pos, "'" + std::string(var->name()) + "' not supported");
context.fErrors->error(pos, "type '" + type.displayName() + "' has no field '" + field + "'");

// Leaving informal natural-language descriptions unquoted:
context.fErrors->error(pos, "local size layout qualifiers are only allowed in compute");
```
