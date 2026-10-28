---
name: skia/tests/concise-reporter-context-labels
description: When formatting labels for skiatest::ReporterContext in unit tests, use concise yet descriptive names, avoiding cryptic single-letter abbreviations and verbose C++ qualifiers
rubric: readability
severity: nit
---

**Rule.** When formatting labels, subtest descriptions, or stringified
parameters for skiatest::ReporterContext in unit tests, use descriptive yet
concise tokens. Avoid ambiguous single-letter or cryptic abbreviations (such as
spelling out "CIE Y (luminance)" rather than "Y"), while omitting redundant C++
type qualifications (such as using "clamp" instead of "SkTileMode::kClamp").

**Why.** skiatest::ReporterContext appends labels onto a context stack printed
during assertion failures. Cryptic single-letter labels impair failure
diagnostics by obscuring what domain concept failed, while verbose fully
qualified C++ identifiers bloat failure outputs and make stacked messages hard
to read.

## Examples

Bad:

```cpp
// Bad: cryptic single-letter subtest label
test("Y", mixingFn, inputColor, expectedColor);

// Bad: verbose fully qualified enum identifier in ReporterContext
static std::string to_string(SkTileMode tm) {
    switch (tm) {
        case SkTileMode::kClamp:  return "SkTileMode::kClamp";
        case SkTileMode::kRepeat: return "SkTileMode::kRepeat";
        default: return "invalid";
    }
}
skiatest::ReporterContext ctx(r, to_string(tm));
```

Good:

```cpp
// Good: descriptive subtest label clarifying test intent
test("CIE Y (luminance)", mixingFn, inputColor, expectedColor);

// Good: concise, unqualified string token for failure context
static std::string to_string(SkTileMode tm) {
    switch (tm) {
        case SkTileMode::kClamp:  return "clamp";
        case SkTileMode::kRepeat: return "repeat";
        default: return "invalid";
    }
}
skiatest::ReporterContext ctx(r, to_string(tm));
```
