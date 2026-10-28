---
name: skia/prefer-sk-invalid-unique-id
description: When initializing, defaulting, or checking invalid unique or generation IDs, use the SK_InvalidUniqueID or SK_InvalidGenID sentinel instead of literal 0
rubric: readability
severity: nit
---

**Rule.** When initializing, assigning a default value to, or checking for an
invalid or absent unique or generation ID (such as results from uniqueID() or
getGenerationID()), use the canonical SK_InvalidUniqueID or SK_InvalidGenID
sentinel constant. Do not use raw literal 0 or 0u.

**Why.** Using the named sentinel constants SK_InvalidUniqueID and
SK_InvalidGenID clarifies intent, avoids magic numbers, and ensures consistency
across Skia subsystems that track and validate object identifiers.

## Examples

Bad:

```cpp
uint32_t srcGenID = srcInKey ? context.source().image()->uniqueID() : 0;
if (imageUniqueID == 0) {
    return;
}
```

Good:

```cpp
uint32_t srcGenID = srcInKey ? context.source().image()->uniqueID() : SK_InvalidUniqueID;
if (imageUniqueID == SK_InvalidUniqueID) {
    return;
}
```
