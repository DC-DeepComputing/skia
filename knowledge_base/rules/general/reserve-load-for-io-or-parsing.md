---
name: skia/reserve-load-for-io-or-parsing
description: When naming functions or variables that store caller-provided data without I/O or parsing, use Set or Register rather than Load or Loader
rubric: readability
severity: should_fix
---

**Rule.** Do not name functions or variables using 'Load' or 'Loader' when they
merely receive and store caller-provided data or pointers in memory without
performing disk/network I/O, parsing, or decoding. Use 'Set*Data', 'Register*',
or similar descriptive naming instead to reflect passive in-memory storage.

**Why.** The terms 'Load' and 'Loader' imply active retrieval, deserialization,
parsing, or I/O operations with non-trivial execution cost. Using them for
lightweight setter functions or in-memory data pointers misleads callers about
performance characteristics and architectural responsibilities.

## Examples

Bad:

```cpp
// Misleadingly implies an active loader rather than storing in-memory data:
void SetGraphiteModuleLoader(const GraphiteModules& modules);
static const char* gLoad_sksl_graphite_frag = nullptr;
```

Good:

```cpp
// Accurately conveys lightweight setter and static data semantics:
void SetGraphiteModuleData(const GraphiteModules& modules);
static const char* sdata_sksl_graphite_frag = "";
```
