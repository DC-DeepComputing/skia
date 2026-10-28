---
name: skia/prefer-skstring-appendf
description: When appending formatted text to an existing SkString, format directly in-place using appendf instead of constructing and appending a temporary string
rubric: readability
severity: nit
---

**Rule.** When formatting and appending text into an existing SkString, call
appendf directly on the destination string. Do not construct an intermediate
SkString, format it via printf or SkStringPrintf, and append it using append.

**Why.** Calling appendf formats directly into the destination string's buffer,
eliminating the boilerplate and memory allocation overhead of constructing and
copying intermediate SkString temporaries.

## Examples

Bad:

```cpp
SkString dups;
dups.printf("*%d", instruction.duplicates + 1);
label.append(dups);

// Or using SkStringPrintf:
label.append(SkStringPrintf(" [%s]", status));
```

Good:

```cpp
label.appendf("*%d", instruction.duplicates + 1);
label.appendf(" [%s]", status);
```
