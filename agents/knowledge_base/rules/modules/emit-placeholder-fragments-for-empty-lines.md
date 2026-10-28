---
name: skia/modules/emit-placeholder-fragments-for-empty-lines
description: When shaping text into line fragments in text builders, emit empty placeholder fragments for empty lines rather than requiring consumers to backfill missing lines
rubric: design
severity: should_fix
---

**Rule.** In text shapers and fragment builders, emit empty placeholder
fragments for empty lines rather than omitting them and forcing downstream
consumers to reconstruct missing lines. Ensure contiguous line indexing so
consumers can rely on sequential line iteration invariants without manual gap
detection or backfilling.

**Why.** Omitting empty lines breaks line-index continuity invariants and pushes
the burden of detecting gaps and reconstructing intermediate or trailing empty
lines onto downstream consumers. Generating placeholder fragments in the builder
preserves contiguous line mapping and simplifies consumer traversal logic.

## Examples

Bad:

```cpp
// Builder omits empty lines, forcing consumers to backfill missing entries:
if (frag.fLineIndex != line) {
    fMaps.fLinesMap.insert(fMaps.fLinesMap.end(),
                           frag.fLineIndex - line,
                           {line_start, 0, 0, 0});
    line = frag.fLineIndex;
}
```

Good:

```cpp
// Builder directly emits an empty placeholder fragment for empty lines:
if (start == end) {
    this->beginLine();
    this->commitLine();
    if (fDesc.fFlags & Shaper::Flags::kFragmentGlyphs) {
        fResult.fFragments.push_back({Shaper::ShapedGlyphs(), {fBox.x(), fBox.y()}, 0, 0, fLineCount - 1, false});
    }
    return;
}
```
