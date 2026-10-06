---
name: skia/api/avoid-transitional-methods-for-legacy-macros
rubric: design
severity: should_fix
---

**Rule.** Do not introduce newly named transitional or parallel member functions
(such as fooConst() or getDataView()) or deprecate existing methods solely to
migrate return types or tighten constness. Instead, update the signature of the
existing method in place, conditionally compiling the declaration in headers and
the return expression in source files under a legacy compatibility macro (such
as SK_DISABLE_LEGACY_* or SK_SUPPORT_LEGACY_*) when staged migration is
required.

**Why.** Renaming methods or adding parallel variants clutters class interfaces,
causes unnecessary churn across downstream consumers (like Chromium and Android)
that only read the data, and requires extra cleanup passes later. Updating the
existing method in place under a build flag minimizes downstream call-site
changes and keeps interfaces single-sourced and clean.
