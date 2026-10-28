---
name: skia/api/update-doc-comments-for-span-parameters
description: When migrating API functions to SkSpan, update doc comments, @param tags, and code examples to describe empty spans ({}) and remove obsolete count parameters
rubric: readability
severity: should_fix
---

**Rule.** When migrating API functions from pointer-and-count arrays to span
types (such as SkSpan), update accompanying documentation comments, @param tags,
and usage examples to reflect span value semantics. Document optional or omitted
span arguments as 'may be empty' (or passing {}), update example calls to pass
empty braces ({}) rather than NULL, 0, or nullptr, remove obsolete @param tags
for eliminated count/capacity parameters, and replace stale count preconditions
with span size expressions (e.g., glyphs.size()).

**Why.** Span types like SkSpan are passed by value and represent optional,
omitted, or probe buffers via empty spans ({}), not nullptr or 0. Retaining
descriptions or code examples referencing NULL, 0, nullptr, obsolete count
parameters in @param tags, or stale count preconditions directly contradicts the
modernized function signature and misleads API callers about valid argument
values and sizing contracts.

## Examples

Bad:

```cpp
/** Retrieves the advance and bounds for each glyph in glyphs.
    Both widths and bounds may be nullptr.
    If widths is not nullptr, widths must be an array of count entries.
 */
void getWidthsBounds(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths,
                     SkSpan<SkRect> bounds) const;

/** Calling with all zeros (e.g. getKerningPairAdjustments(NULL, 0, NULL)) checks support.
    @param glyphs        storage for glyph indices; may be nullptr
    @param maxGlyphCount storage capacity
*/
bool getKerningPairAdjustments(SkSpan<const SkGlyphID> glyphs,
                               SkSpan<int32_t> adjustments) const;
```

Good:

```cpp
/** Retrieves the advance and bounds for each glyph in glyphs.
    widths receives min(widths.size(), glyphs.size()) values; may be empty.
    bounds receives min(bounds.size(), glyphs.size()) values; may be empty.
 */
void getWidthsBounds(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths,
                     SkSpan<SkRect> bounds) const;

/** Calling with empty spans (e.g. getKerningPairAdjustments({}, {})) checks support.
    @param glyphs        storage for glyph indices; may be empty
*/
bool getKerningPairAdjustments(SkSpan<const SkGlyphID> glyphs,
                               SkSpan<int32_t> adjustments) const;
```
