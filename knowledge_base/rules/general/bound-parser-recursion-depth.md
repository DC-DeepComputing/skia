---
name: skia/bound-parser-recursion-depth
description: When recursively parsing nested syntax like expression trees or fallback chains, enforce an explicit recursion depth limit to prevent stack exhaustion
rubric: safety
severity: should_fix
---

**Rule.** When implementing recursive descent parsers or processing nested
syntax (such as expression trees, CSS variable fallbacks, or nested markup),
enforce an explicit recursion depth limit using a counter or depth-tracking
guard (such as kRecursionLimit or kMaxParseDepth). Fail or bail out gracefully
when nesting depth exceeds the limit rather than allowing unbounded recursion.

**Why.** Untrusted inputs with maliciously crafted or deeply nested syntax can
cause unbounded recursive descent, leading to call-stack exhaustion,
denial-of-service crashes, and potential security vulnerabilities. Bounding
recursion depth ensures parsing terminates safely even on pathological inputs.

## Examples

Bad:

```cpp
bool SkSVGAttributeParser::parseSVGColor(SkSVGColor* colorResult, int numVars) {
    if (this->parseExpectedStringToken(",")) {
        this->parseWSToken();
        // Unbounded recursive descent on nested var() fallbacks:
        if (!this->parseSVGColor(colorResult, numVars + 1)) {
            return false;
        }
    }
    return true;
}
```

Good:

```cpp
static constexpr int kVarsLimit = 32;

bool SkSVGAttributeParser::parseSVGColor(SkSVGColor* colorResult, SkSVGColor::Vars&& vars) {
    if (this->parseExpectedStringToken(",")) {
        this->parseWSToken();
        // Explicit depth check prevents stack exhaustion:
        if (vars.size() >= kVarsLimit ||
            !this->parseSVGColor(colorResult, std::move(vars))) {
            return false;
        }
    }
    return true;
}
```
