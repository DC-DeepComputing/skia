---
name: skia/modules/defer-svg-css-variables-to-render-time
description: When parsing SVG attributes and colors with CSS variables, preserve variable references for render-time resolution rather than eagerly evaluating them during parsing
rubric: design
severity: should_fix
---

**Rule.** When parsing SVG attributes, properties, and colors that support CSS
variables (var(...)), store unresolved variable identifiers in the DOM
representation for deferred resolution at render time rather than eagerly
evaluating them at parse time.

**Why.** Resolving CSS variables eagerly during SVG DOM parsing bakes active
palette values into the parsed tree, requiring expensive re-parsing whenever
dynamic palettes or variables change. Deferring resolution to render time
enables parsed SVG DOM trees and font glyphs (such as OT-SVG) to be cached and
reused across multiple dynamic palettes.

## Examples

Bad:

```cpp
// Eagerly resolving CSS variables to concrete SkColor values at parse time
bool SkSVGAttributeParser::parseSVGColor(SkColor* color) {
    if (this->parseParenthesized("var", [&](SkColor* res) {
        *res = this->resolveCurrentPaletteColor(varName);
        return true;
    })) {
        return true;
    }
    return this->parseSVGColorType(color);
}
```

Good:

```cpp
// Preserving variable identifiers in SkSVGColor for render-time evaluation
bool SkSVGAttributeParser::parseSVGColor(SkSVGColor* color, SkSVGColor::Vars&& vars) {
    if (this->parseParenthesized("var", [&](SkSVGColor* colorResult) {
        vars.push_back(std::move(ident));
        return this->parseSVGColor(colorResult, std::move(vars));
    })) {
        return true;
    }
    if (SkSVGColorType c; this->parseSVGColorType(&c)) {
        *color = SkSVGColor(c, std::move(vars));
        return true;
    }
    return false;
}
```
