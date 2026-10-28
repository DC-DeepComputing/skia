---
name: skia/cite-published-spec-clause-and-title
description: When implementing external specifications or syntax parsers, cite published clause titles, deep-link section URLs, and document grammar rules or multi-standard discrepancies
rubric: readability
severity: should_fix
---

**Rule.** When documenting data structures, constants, parser logic, or
algorithms implementing external specifications or standards (such as W3C, CSS,
WGSL, SMPTE, ISO, or OpenType), cite published standards or grammar productions,
include descriptive clause titles, and provide direct deep links with section
anchor fragments rather than linking only to top-level URLs. Document governing
clauses in blanket comments at enclosing scopes rather than repeating fragmented
citations across nested members. When unifying metadata across multiple
standards, document all relevant specifications and highlight semantic
discrepancies.

**Why.** Draft citations shift before ratification, while undocumented parser
syntax and generic top-level URLs force maintainers to guess which clause or
grammar rule justifies code behavior. Citing deep links with section anchors,
grammar productions, and descriptive clause titles ensures maintainers can
verify normative requirements, constraints, and edge-case syntax compliance.

## Examples

Bad:

```cpp
// Draft clause citation without descriptive title and fragmented across nested structs:
struct ComponentMixingFunction { /* Clause 5.2 */ };

// Parser rule matching standards-based syntax without grammar comment or spec link:
if (this->parseParenthesized("var", [this, numVars](SkSVGColor* colorResult) -> bool {
    SkString ident;
    if (!this->parseIdentToken(&ident) || ident.size() < 2 || !ident.startsWith("--")) {
        return false;
    }
    ...
});

// Common spec keyword referenced without a link to the specification section:
// TODO (b/340278447): Dawn doesn't support the new 'either' option yet.
this->write("@interpolate(flat) ");
```

Good:

```cpp
/**
 * Adaptive global tone mapping metadata defined in SMPTE ST 2094-50.
 * Names correspond to the ColorVolumeTransform group in Clause 7.1: Metadata set.
 */
class AgtmImpl final : public Agtm { ... };

// Document standards-based parser logic with deep link to specification section:
// https://drafts.csswg.org/css-variables/#using-variables
if (this->parseParenthesized("var", [this, &vars](SkSVGColor* colorResult) -> bool {
    SkString ident;
    if (!this->parseIdentToken(&ident) || ident.size() < 2 || !ident.startsWith("--")) {
        return false;
    }
    ...
});

// Common spec keyword documented with direct link to the specification section:
// See https://www.w3.org/TR/WGSL/#interpolation-sampling-either
this->write("@interpolate(flat) ");
```
