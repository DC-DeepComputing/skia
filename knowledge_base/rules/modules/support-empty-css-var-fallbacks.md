---
name: skia/modules/support-empty-css-var-fallbacks
description: When parsing CSS var() custom property references in SVG attributes, accept empty fallback values after a comma as valid syntax
rubric: correctness
severity: should_fix
---

**Rule.** When parsing CSS var() references in SVG attributes, support an empty
fallback value following a comma (such as var(--foo,)) as valid syntax. Do not
treat a missing fallback value or empty declaration after the comma separator as
a parse error.

**Why.** Per the W3C CSS Custom Properties specification, the fallback value
after a comma in var() is optional and may be empty. Rejecting var(--foo,) as a
syntax error violates the specification and prevents valid custom properties
from resolving.

## Examples

Bad:

```cpp
if (this->parseExpectedStringToken(",")) {
    this->parseWSToken();
    // Rejects empty fallback: var(--foo,) fails parse
    if (!this->parseSVGColor(colorResult, numVars + 1)) {
        return false;
    }
}
```

Good:

```cpp
if (this->parseExpectedStringToken(",")) {
    this->parseWSToken();
    // Allow empty fallback when closing parenthesis immediately follows comma
    if (this->matchStringToken(")")) {
        *colorResult = SkSVGColor(SK_ColorBLACK, std::move(vars));
        return true;
    }
    return vars.size() < kVarsLimit && this->parseSVGColor(colorResult, std::move(vars));
}
```
