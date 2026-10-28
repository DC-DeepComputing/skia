---
name: skia/api/document-dump-filter-predicate-exceptions
description: When documenting memory dump or diagnostic filter predicates in public headers, explicitly document any unconditional inclusions or exclusions that bypass filtering
rubric: readability
severity: should_fix
---

**Rule.** In doc comments for memory dump or diagnostic filter predicates (such
as SkTraceMemoryDump::shouldDump*), explicitly document any unconditional
exclusions or inclusions that bypass the filter—such as memoryless textures
being dumped regardless of zero-size filtering—rather than stating an
unqualified filtering rule.

**Why.** Filter predicates that claim to suppress certain categories of
resources (such as zero-sized objects) confuse callers and implementers if
specific resources bypass the check and are dumped regardless. Explicitly noting
exceptions in header documentation prevents misunderstandings of dump statistics
and keeps filtering contracts accurate.

## Examples

Bad:

```cpp
/**
 * Returns true if we should dump zero-sized objects (e.g. Samplers, pipelines, etc).
 * This call is only used when dumping Graphite memory statistics.
 */
virtual bool shouldDump0SizeObjects() const { return false; }
```

Good:

```cpp
/**
 * Returns true if we should dump sizeless non-texture objects (e.g. Samplers, pipelines, etc).
 * Memoryless textures are always dumped. This call is only used when dumping Graphite memory
 * statistics.
 */
virtual bool shouldDumpSizelessObjects() const { return false; }
```
