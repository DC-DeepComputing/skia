---
name: skia/modules/match-run-iterator-factory-names
description: When declaring or implementing factory methods creating SkShaper run iterators, include the full RunIterator type name like makeBidiRunIterator rather than truncating to makeBidiIterator
rubric: readability
severity: should_fix
---

**Rule.** When declaring or implementing factory methods that create SkShaper
run iterators (such as BiDiRunIterator, ScriptRunIterator, FontRunIterator, or
LanguageRunIterator), name the method after the full iterator subtype (e.g.,
makeBidiRunIterator, makeScriptRunIterator). Do not truncate the name by
omitting 'Run' (e.g., avoid makeBidiIterator) or use generic names like
makeRunIterator.

**Why.** In SkShaper, run iterators form an explicit family derived from
SkShaper::RunIterator. Truncating 'Run' from factory names creates inconsistency
across sibling iterator methods and causes naming ambiguity with general Unicode
iterators (such as SkUnicode::makeBidiIterator).

## Examples

Bad:

```cpp
// Bad: omits 'Run', creating inconsistency with SkShaper iterator naming
virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiIterator(
        const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) const = 0;

virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptIterator(
        const char* utf8, size_t utf8Bytes) const = 0;
```

Good:

```cpp
// Good: mirrors the concrete RunIterator type name
virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(
        const char* utf8, size_t utf8Bytes, uint8_t bidiLevel) const = 0;

virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(
        const char* utf8, size_t utf8Bytes) const = 0;
```
