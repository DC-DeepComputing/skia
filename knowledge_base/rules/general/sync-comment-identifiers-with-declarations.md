---
name: skia/sync-comment-identifiers-with-declarations
description: When referencing types, members, or parameters in comments, TODOs, or call-site labels, use exact declared names and remove deleted or renamed identifiers
rubric: readability
severity: should_fix
---

**Rule.** When referencing types, classes, variables, functions, member fields,
or parameters in comments, TODOs, or call-site labels (e.g., `/*param=*/`),
ensure identifiers and capitalization exactly match active code declarations
(e.g., 'RenderStep' rather than 'renderstep'). Spell out type and concept names
in full rather than using ad-hoc acronyms or informal shorthand (e.g.,
'RuntimeEffect' instead of 'RE', and exact 'startList' instead of colloquial
'start'). When writing, duplicating, or refactoring code, keep referenced
identifiers synchronized with declarations in scope (avoiding copy-pasted
comments referencing adjacent fields) and remove references to deleted or
renamed members.

**Why.** Stale references to deleted or adjacent fields, mismatched
capitalization, and informal acronyms in comments confuse readers, break
codebase symbol search, and misrepresent active invariants and inputs. Keeping
comment identifiers synchronized with declarations prevents confusion during
maintenance.

## Examples

Bad:

```cpp
// Ad-hoc acronym abbreviation for type in comment:
// Scope of a SkRuntimeEffect based on how the RE's SkSL invokes each child.

// Lowercased type name in documentation comments:
// * Each renderstep may have "Static" data and/or "Append" data.

// Stale snake_case variable names in condition comment:
// If y_top_tiles == y_bottom_tiles, then the line is either completely above or below
if (yTopTiles >= yBottomTiles) {
    return;
}

// Copy-pasted comment referencing adjacent field instead of annotated member:
// All Symbol* objects in fStructCounts must be FunctionDeclaration*.
skia_private::THashMap<const Symbol*, int> fCallCounts;

// Mismatched parameter name from refactoring or drafting:
// Copy `decodedRow` into `decodedInterlacedFullWidthRow`.
void expandRow(SkSpan<uint8_t> dst, SkSpan<const uint8_t> srcRow);

// Call-site label mismatch with formal parameter:
auto pattern = get_gpu_access_pattern(/*useTransferBuffer=*/true);

// Stale reference to deleted struct field in doc comment:
/** Expands fDeviceGamma, fPaintGamma, fContrast, and fLumBits into a mask pre-blend. */
SkMaskGamma::PreBlend GetMaskPreBlend(const SkScalerContextRec& rec);
```

Good:

```cpp
// Spelled-out class name matching concept:
// Scope of a SkRuntimeEffect based on how the RuntimeEffect's SkSL invokes each child.

// Exact capitalized class name matching declaration:
// * Each RenderStep may have "Static" data and/or "Append" data.

// Synchronized variable names and casing in condition comment:
// If yTopTiles == yBottomTiles, then the line is either completely above or below
if (yTopTiles >= yBottomTiles) {
    return;
}

// Synchronized member name in invariant comment:
// All Symbol* objects in fCallCounts must be FunctionDeclaration*.
skia_private::THashMap<const Symbol*, int> fCallCounts;

// Synchronized parameter name matching active declaration:
// Copy `srcRow` into `decodedInterlacedFullWidthRow`.
void expandRow(SkSpan<uint8_t> dst, SkSpan<const uint8_t> srcRow);

// Call-site label matching formal parameter:
auto pattern = get_gpu_access_pattern(/*isGpuOnlyAccess=*/true);

// Removed reference to deleted field 'fPaintGamma':
/** Expands fDeviceGamma, fContrast, and fLumBits into a mask pre-blend. */
SkMaskGamma::PreBlend GetMaskPreBlend(const SkScalerContextRec& rec);
```
