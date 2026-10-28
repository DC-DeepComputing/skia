---
name: skia/modules/advance-utf16-index-to-skip-surrogate-pairs
description: When skipping a multi-byte codepoint or trailing surrogate in UTF-16-to-UTF-8 mappings, advance the UTF-16 index rather than incrementing the UTF-8 byte offset
rubric: testing
severity: should_fix
---

**Rule.** When skipping past a trailing surrogate or multi-byte codepoint
translated from UTF-16 to UTF-8, advance the UTF-16 code unit index (e.g., start
+ 1) and query the mapping table rather than incrementing the UTF-8 byte offset
directly (++utf8).

**Why.** In translation tables between UTF-16 and UTF-8, surrogate pairs map
both lead and trail code units to the same 4-byte UTF-8 sequence start.
Incrementing the UTF-8 byte offset directly lands on an unaligned continuation
byte within the multi-byte sequence, whereas advancing the UTF-16 index directly
resolves to the aligned start byte of the next code point.

## Examples

Bad:

```cpp
auto utf8 = fUTF8IndexForUTF16Index[start];
// If start points to a trailing surrogate, incrementing the byte offset lands on a continuation byte
if (start > 0 && fUTF8IndexForUTF16Index[start - 1] == utf8) {
    ++utf8;
}
text.start = this->findNextGraphemeBoundary(utf8);
```

Good:

```cpp
auto utf8 = fUTF8IndexForUTF16Index[start];
// If start points to a trailing surrogate, advance the UTF-16 index to reach the next code point
if (start > 0 && fUTF8IndexForUTF16Index[start - 1] == utf8) {
    utf8 = fUTF8IndexForUTF16Index[start + 1];
}
text.start = this->findNextGraphemeBoundary(utf8);
```
