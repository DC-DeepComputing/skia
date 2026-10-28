---
name: skia/gpu/prefer-snippet-requirement-flags-over-id-checks
description: When inspecting Graphite shader snippets for capabilities or payload requirements, query SnippetRequirementFlags or Entry accessors rather than checking BuiltInCodeSnippetID lists
rubric: design
severity: nit
---

**Rule.** When inspecting Graphite shader snippets for capabilities, required
features, or data payloads (such as sampler descriptor data or coordinate
requirements), query SnippetRequirementFlags or dedicated
ShaderCodeDictionary::Entry predicate methods rather than branching on an
explicit list of BuiltInCodeSnippetID enum values. If a new snippet capability
is needed across snippet types, define a new SnippetRequirementFlags bit instead
of coupling callers to concrete snippet IDs.

**Why.** Hardcoding lists of BuiltInCodeSnippetID values to determine
capabilities or payload handling couples callers to specific snippet variants
and is error-prone when new snippet types or specializations are introduced.
Querying centralized requirement flags or entry accessors keeps snippet
consumers modular and ensures new snippets automatically inherit correct
processing when registered with the appropriate flags.

## Examples

Bad:

```cpp
int32_t snippetId = node->codeSnippetId();
if (!node->data().empty() &&
    (snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kImageShader) ||
     snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kCubicImageShader) ||
     snippetId == static_cast<int32_t>(BuiltInCodeSnippetID::kHWImageShader))) {
    append_sampler_descs(node->data(), *outDescs);
}
```

Good:

```cpp
// Query capability flags on the snippet entry rather than listing snippet IDs:
if (entry->storesSamplerDescData()) {
    append_sampler_descs(node->data(), *outDescs);
}
```
