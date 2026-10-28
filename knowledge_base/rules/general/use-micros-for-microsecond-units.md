---
name: skia/use-micros-for-microsecond-units
description: When naming microsecond duration variables, parameters, or helper functions, use 'micros' or 'microseconds' instead of 'us' to avoid confusion with the English word
rubric: readability
severity: nit
---

**Rule.** Do not use 'us' or 'us_' as an identifier, prefix, or suffix for
microsecond time units; use 'micros' or 'microseconds' instead (e.g.,
micros_since or timeoutMicros).

**Why.** The ASCII abbreviation 'us' (used for microseconds, µs) is visually
identical to the English pronoun 'us', making identifiers like us_since or
duration_us ambiguous and prone to misreading. Using 'micros' or 'microseconds'
eliminates ambiguity and reads naturally.

## Examples

Bad:

```cpp
// Ambiguous: 'us_since' can easily be misread as the English pronoun 'us'.
int64_t us_since(skgpu::StdSteadyClock::time_point start);
uint32_t timeout_us = 5000;
```

Good:

```cpp
// Unambiguous: 'micros' clearly denotes microsecond time units.
int64_t micros_since(skgpu::StdSteadyClock::time_point start);
uint32_t timeoutMicros = 5000;
```
