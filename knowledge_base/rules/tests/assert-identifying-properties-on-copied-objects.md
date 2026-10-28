---
name: skia/tests/assert-identifying-properties-on-copied-objects
description: When unit testing copy constructors, assignment operators, or backend wrappers, assert specific identifying properties match rather than relying solely on coarse validity checks
rubric: testing
severity: should_fix
---

**Rule.** When unit testing copy constructors, assignment operators, or backend
wrapper objects, assert that discriminating properties (such as the backend API
or specific type attributes) match between the source and target objects rather
than relying solely on coarse boolean validity checks like isInitialized() or
isValid().

**Why.** Coarse validity checks like isInitialized() or isValid() only verify
that an object holds non-empty state. They do not ensure that discriminating
fields, backend enums, or type attributes were copied correctly and not left
uninitialized, defaulted, or mismatched.

## Examples

Bad:

```cpp
GrBackendSemaphore copy(sema);
REPORTER_ASSERT(reporter, copy.isInitialized());
```

Good:

```cpp
GrBackendSemaphore copy(sema);
REPORTER_ASSERT(reporter, copy.isInitialized());
REPORTER_ASSERT(reporter, copy.backend() == GrBackendApi::kVulkan);
```
