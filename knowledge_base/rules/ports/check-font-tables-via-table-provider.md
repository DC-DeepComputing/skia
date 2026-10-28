---
name: skia/ports/check-font-tables-via-table-provider
description: In Fontations font bridge code, check table presence or capabilities using typed TableProvider accessors rather than scanning raw SFNT table_directory records
rubric: design
severity: should_fix
---

**Rule.** In Fontations font bridge code, query font table presence or
capabilities using typed TableProvider accessors (such as f.colr().is_ok(),
f.sbix().is_ok(), or f.cbdt().is_ok()) instead of iterating over raw table tags
in table_directory.table_records().

**Why.** Querying tables through TableProvider accessors operates at the
appropriate abstraction layer in the font stack, verifying that the table is
valid and readable rather than merely declared in the SFNT header. It also
ensures table availability checks stay aligned with how font and glyph data are
actually parsed and retrieved.

## Examples

Bad:

```cpp
let has_any_color = font
    .as_ref()
    .map(|f| {
        f.table_directory
            .table_records()
            .iter()
            .any(|record| match record.tag.get() {
                SBIX_TAG | CBDT_TAG | COLR_TAG => true,
                _ => false,
            })
    })
    .unwrap_or_default();
```

Good:

```cpp
let has_any_color = font
    .as_ref()
    .map(|f| {
        f.cbdt().is_ok() ||
        f.sbix().is_ok() ||
        f.colr().is_ok()
    })
    .unwrap_or_default();
```
