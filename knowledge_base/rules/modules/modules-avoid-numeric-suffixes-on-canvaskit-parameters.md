---
name: skia/modules/modules-avoid-numeric-suffixes-on-canvaskit-parameters
description: In CanvasKit JavaScript bindings, do not append numeric suffixes to public parameters to resolve name collisions; suffix internal WASM heap pointers with Ptr instead
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit JavaScript bindings, do not append artificial numeric
suffixes (such as words0 or rect0) to public function or method parameter names
to avoid name collisions with internal variables. Keep public parameter names
clean and descriptive, and distinguish internal WASM heap memory pointer
variables by suffixing them with Ptr (e.g., wordsPtr or wPtr).

**Why.** Public API parameter names are client-facing and surface directly in
public documentation, TypeScript declarations, and IDE autocomplete. Mangling
public parameters with numeric digits simply to reuse the base identifier
internally confuses consumers and degrades API readability, whereas suffixing
internal pointers with Ptr clearly communicates their representation as WASM
heap allocations.

## Examples

Bad:

```cpp
CanvasKit.ParagraphBuilder.prototype.buildWithClientInfo =
    function(bidiRegions, words0, graphemeBreaks, softLineBreaks, hardLineBreaks) {
  // Bad: Public parameter 'words0' has an artificial digit to avoid collision with 'words'
  var words = copy1dArray(words0, 'HEAPU32');
  this._buildWithClientInfo(bidiRegions, words, graphemeBreaks, softLineBreaks, hardLineBreaks);
};
```

Good:

```cpp
CanvasKit.ParagraphBuilder.prototype.buildWithClientInfo =
    function(bidiRegions, words, graphemeBreaks, softLineBreaks, hardLineBreaks) {
  // Good: Parameter 'words' remains clean; internal WASM pointer is suffixed with 'Ptr'
  var wordsPtr = copy1dArray(words, 'HEAPU32');
  this._buildWithClientInfo(bidiRegions, wordsPtr, graphemeBreaks, softLineBreaks, hardLineBreaks);
};
```
