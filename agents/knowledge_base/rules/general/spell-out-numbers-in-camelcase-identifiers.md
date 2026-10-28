---
name: skia/spell-out-numbers-in-camelcase-identifiers
description: When naming methods, variables, or types in camelCase, spell out numbers as words or use descriptive adjectives instead of embedding raw digits
rubric: readability
severity: nit
---

**Rule.** When naming methods, variables, parameters, or types in camelCase,
spell out numeric quantities as words (e.g., ZeroSized or OneByte) or use
descriptive adjectives (e.g., Sizeless) instead of embedding raw numeric digits
(e.g., 0Size). Standard dimensional suffixes and fixed format designations (such
as 2D, 3D, or RGBA8888) are acceptable exceptions.

**Why.** Embedding numeric digits directly in camelCase identifiers (such as
0Size) disrupts natural word boundaries and casing conventions, reducing
readability. Spelling out numbers as words (such as ZeroSized) or using
descriptive adjectives (such as Sizeless) maintains clean, idiomatic naming
consistent with Skia conventions.

## Examples

Bad:

```cpp
// Avoid embedding raw numeric digits in camelCase identifiers
virtual bool shouldDump0SizeObjects() const { return false; }
bool is0ByteAllocation = (size == 0);
```

Good:

```cpp
// Spell out numbers as words or use descriptive adjectives
virtual bool shouldDumpSizelessObjects() const { return false; }
bool isZeroByteAllocation = (size == 0);
```
