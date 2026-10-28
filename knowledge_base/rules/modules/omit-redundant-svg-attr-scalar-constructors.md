---
name: skia/modules/omit-redundant-svg-attr-scalar-constructors
description: When declaring SVG DOM node attributes with SVG_ATTR for scalar or numeric types, specify numeric literals directly instead of wrapping them in redundant constructor calls
rubric: readability
severity: nit
---

**Rule.** When declaring SVG DOM node attributes with SVG_ATTR (or related
macros) for scalar or numeric types like SkSVGNumberType or SkSVGIntegerType,
specify the default value directly using a numeric literal (e.g., 0 or 1). Do
not wrap literals in redundant pseudo-constructor calls like SkSVGNumberType(0)
or SkSVGIntegerType(1).

**Why.** In SVG_ATTR macro expansions, default values initialize member
variables directly in-class (attr_type fName = attr_default). For fundamental
and typedef'd scalar types, wrapping numeric literals in explicit
functional-style constructor calls adds unnecessary syntactic boilerplate
without any safety or functional benefit.

## Examples

Bad:

```cpp
// Redundant constructor wrappers for scalar SVG attribute defaults
SVG_ATTR(Azimuth, SkSVGNumberType, SkSVGNumberType(0))
SVG_ATTR(Elevation, SkSVGNumberType, SkSVGNumberType(0))
SVG_ATTR(NumOctaves, SkSVGIntegerType, SkSVGIntegerType(1))
```

Good:

```cpp
// Pass numeric literals directly for scalar SVG attribute defaults
SVG_ATTR(Azimuth, SkSVGNumberType, 0)
SVG_ATTR(Elevation, SkSVGNumberType, 0)
SVG_ATTR(NumOctaves, SkSVGIntegerType, 1)
```
