---
name: skia/api/default-optional-label-parameters
description: When declaring GPU resource, cache, or surface APIs accepting diagnostic labels, default optional std::string_view label parameters to {}
rubric: readability
severity: should_fix
---

**Rule.** In GPU resource, cache, and surface factory APIs, provide a default
empty value (= {}) for trailing optional std::string_view label parameters
(e.g., std::string_view label = {}). Do not force callers that do not require
diagnostic labels to pass boilerplate empty strings or dummy comments like
/*label=*/"".

**Why.** Many GPU resource types (such as samplers, descriptor sets, and scratch
buffers) and unit tests do not require diagnostic debug labels. Providing a
default '= {}' simplifies call sites by eliminating redundant empty string
arguments while keeping diagnostic labels readily available when needed.

## Examples

Bad:

```cpp
// Bad: Callers must pass boilerplate empty strings when labels are not needed
Resource* findAndRefResource(const GraphiteResourceKey& key,
                             Budgeted budgeted,
                             Shareable shareable,
                             std::string_view label,
                             const ScratchResourceSet* unavailable = nullptr);

// Call site:
fResourceCache->findAndRefResource(key, kBudgeted, kShareable, /*label=*/"");
```

Good:

```cpp
// Good: Default '= {}' allows callers to omit optional diagnostic labels
Resource* findAndRefResource(const GraphiteResourceKey& key,
                             Budgeted budgeted,
                             Shareable shareable,
                             std::string_view label = {},
                             const ScratchResourceSet* unavailable = nullptr);

// Call site:
fResourceCache->findAndRefResource(key, kBudgeted, kShareable);
```
