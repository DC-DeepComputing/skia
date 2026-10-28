---
name: skia/pdf/include-stm-in-non-page-marked-content-references
description: When emitting tagged PDF marked-content references for non-page streams like FormXObjects, include an /Stm stream reference in the /MCR dictionary alongside /Pg and /MCID
rubric: correctness
severity: should_fix
---

**Rule.** When emitting marked-content references (/Type /MCR) in tagged PDF
structure trees, include an /Stm entry referencing the containing stream object
whenever the marked content sequence resides in a non-page stream (such as a
FormXObject). Do not emit a plain integer MCID or omit /Stm when referencing
content outside a page's primary content stream.

**Why.** Per ISO 32000-2 (section 14.7.5.2), marked content identifiers (MCIDs)
are scoped strictly to their containing stream. When marked content resides
inside a non-page stream like a FormXObject, PDF readers cannot resolve the
target marks without an explicit /Stm indirect reference in the /MCR dictionary.

## Examples

Bad:

```cpp
// Bad: Emitting raw MCID integer or omitting /Stm for non-page stream marks
kids->appendInt(info.fMcid);
// or:
std::unique_ptr<SkPDFDict> mcr = SkPDFMakeDict("MCR");
mcr->insertRef("Pg", doc->getPage(info.fLocation.fPageIndex));
mcr->insertInt("MCID", info.fMcid);
kids->appendObject(std::move(mcr));
```

Good:

```cpp
// Good: Emitting /Stm referencing the stream object for non-page content
std::unique_ptr<SkPDFDict> mcr = SkPDFMakeDict("MCR");
mcr->insertRef("Pg", doc->getPage(info.fLocation.fPageIndex));
if (SkPDFIndirectReference streamRef =
            structTree.getContentStreamRefForStructParentsKey(info.fStructParentsKey)) {
    mcr->insertRef("Stm", streamRef);
}
mcr->insertInt("MCID", info.fMcid);
kids->appendObject(std::move(mcr));
```
