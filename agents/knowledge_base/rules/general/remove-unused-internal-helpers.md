---
name: skia/remove-unused-internal-helpers
description: When modifying internal code, classes, or tests, remove unreferenced helpers, obsolete declarations, leftover accessors, and speculative scaffolding, omitting unused macros or functions
rubric: readability
severity: should_fix
---

**Rule.** When modifying or refactoring internal classes, translation units,
modules, or tests, delete unreferenced helper functions, unused accessors or
member methods left over from testing iterations, obsolete overloads, and empty
struct stubs or unused type declarations rather than retaining dead code. When
deleting or refactoring function definitions, eliminating a helper's sole
caller, deleting obsolete tests, or consolidating implementations into shared
logic, remove all associated declarations, accompanying type aliases (such as
visitor callbacks), redundant differential test comparisons or mock overrides,
orphaned test-only helper classes, and companion friend class declarations from
headers, source files, and test suites. Do not retain obsolete helpers under #if
defined(SK_DEBUG) solely to re-verify caller invariants, nor introduce
speculative utility methods (such as shrink_to_fit on transient builders),
unused macros, vestigial scaffolding, or uncalled helper functions 'just in
case' without active callers or production use cases.

**Why.** Internal headers, classes, and test files should not accumulate dead
code, uncalled leftover methods or accessors, empty type stubs, speculative
APIs, or redundant test scaffolding. Speculative or unused code added 'just in
case' increases binary size and maintenance burden without test coverage, risks
bit-rot, and misleads readers into believing the code is actively required.
Retaining unreferenced helpers or declarations also clutters interfaces, risks
linker errors if dead declarations are invoked, and leaves internals
unnecessarily exposed.

## Examples

Bad:

```cpp
// In modules/skparagraph/src/ParagraphCache.cpp, speculative helper and macros added without callers:
// Not in use but keeping it just in case
#define NOCACHE_RUN_MIN 128
#define NOCACHE_RUN_RATIO 0.4
bool ParagraphCache::isTooMuchMemoryWasted(skia::textlayout::ParagraphImpl* paragraph) {
    if (paragraph->fRuns.size() < NOCACHE_RUN_MIN) {
        return false;
    }
    return true;
}

// In modules/skparagraph/src/ParagraphImpl.h, dead declarations and alias left after deleting implementations:
using CodeUnitRangeVisitor = std::function<bool(TextRange textRange)>;
void forEachCodeUnitPropertyRange(CodeUnitFlags property, CodeUnitRangeVisitor visitor);

// In modules/skparagraph/src/Run.h, accessor added during drafting but uncalled anywhere in the change:
uint8_t bidiLevel() const { return fBidiLevel; }

// In SkPathPriv.h, leaving an obsolete overload after callers pass bounds directly:
static bool TooBigForMath(const SkPath& path) { return TooBigForMath(path.getBounds()); }

// Retaining a helper under SK_DEBUG solely to assert an invariant guaranteed by its caller:
#if defined(SK_DEBUG)
bool paint_depends_on_dst(const PaintParams& paintParams) { return false; }
#endif
```

Good:

```cpp
// Omit speculative helper functions, macros, and unused declarations until an active use case exists;
// rely on version control history if needed later.

// Remove obsolete header declarations and associated type aliases alongside deleted definitions:
// (CodeUnitRangeVisitor and forEachCodeUnitPropertyRange removed from ParagraphImpl.h)

// Delete unused accessors and testing scaffolding from class definitions:
// (Run::bidiLevel() removed)

// Remove the unused overload once callers pass bounds directly:
static bool TooBigForMath(const SkRect& bounds) { return true; }

// Remove the obsolete helper and redundant assertion when the caller establishes the contract:
std::optional<SkColor4f> extract_paint_color(const PaintParams& paint, const SkColorInfo& info) {
    return std::nullopt;
}
```
