---
name: skia/omit-url-scheme-in-skbug-references
description: When referencing Skia bugs in code comments, omit the 'http://' or 'https://' URL scheme prefix and use the bare 'skbug.com/<id>' format instead
rubric: readability
severity: nit
---

**Rule.** When referencing Skia bugs in code comments, omit the 'http://' or
'https://' scheme prefix and write the bare 'skbug.com/<id>' domain format. Do
not include protocol schemes such as 'http://skbug.com/<id>' or
'https://skbug.com/<id>'.

**Why.** Skia standardizes bug tracker references in comments on concise domain
shortlinks like 'skbug.com/<id>'. Modern code review tools, IDEs, and terminals
automatically linkify bare domain URLs, so including protocol schemes adds
unnecessary visual noise without functional benefit.

## Examples

Bad:

```cpp
// http://skbug.com/40038559
DEF_SIMPLE_GM(text_scale_skew, canvas, 256, 128) {
    // See https://skbug.com/40040947 for workaround details.
}
```

Good:

```cpp
// skbug.com/40038559
DEF_SIMPLE_GM(text_scale_skew, canvas, 256, 128) {
    // See skbug.com/40040947 for workaround details.
}
```
