---
name: skia/modules/document-fallback-asset-id-todo
description: When invoking skresources::ResourceProvider::loadImageAsset, pass the element or asset identifier for resource_id or document fallback IDs with a TODO
rubric: readability
severity: nit
---

**Rule.** When invoking skresources::ResourceProvider::loadImageAsset, pass the
DOM element or asset identifier as the resource_id parameter to enable
client-side asset substitution. If element IDs are not yet tracked or available
and a fallback (such as the asset filename) is temporarily used as the
identifier, document the limitation with an attributed TODO explaining the
planned element-based substitution.

**Why.** Embedding clients often configure skresources::ResourceProvider
callbacks to intercept and dynamically substitute image assets by element or
asset ID. Passing a filename or dummy identifier instead of the element's ID
prevents clients from performing targeted substitutions, so any temporary
fallback must be documented with a TODO to ensure proper identifier plumbing is
completed.

## Examples

Bad:

```cpp
const auto name = SkOSPath::Basename(href.iri().c_str());
imageAsset = rp->loadImageAsset(path.c_str(), name.c_str(), /* id */ name.c_str());
```

Good:

```cpp
// TODO: It may be better to use the SVG 'id' attribute as the asset id, to allow
// clients to perform asset substitution based on element id.
const auto name = SkOSPath::Basename(href.iri().c_str());
imageAsset = rp->loadImageAsset(path.c_str(), name.c_str(), /* id */ name.c_str());
```
