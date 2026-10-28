---
name: skia/deduplicate-fallback-else-across-preprocessor-guards
description: 'When conditionally compiling code, keep shared enclosing control flow (like else blocks), trailing returns, or fallback blocks outside #if/#else/#endif rather than duplicating them'
rubric: readability
severity: should_fix
---

**Rule.** When conditionalizing code with preprocessor guards (#if / #ifdef /
#else), narrow directives to only divergent statements rather than duplicating
enclosing control-flow constructs like 'else' blocks. Do not rely on
preprocessor #else as the fallback path for runtime capability checks inside an
#if block; unsupported runtime states must execute explicit fallback logic. When
conditionally prepending an 'if' check before a shared fallback path, place '}
else' before #endif and keep the shared fallback block '{ ... }' after #endif.
Similarly, place #endif before trailing fallback returns rather than duplicating
returns across #else branches.

**Why.** Wrapping duplicated control-flow structures (such as 'else' blocks or
duplicate braces) inside preprocessor branches fragments block syntax and
obscures what actually differs between configurations. Furthermore, relying on
compile-time #else leaves runtime capability checks without fallback handling
when unsupported at runtime, while duplicating trailing returns or fallback
blocks across #else branches introduces redundant boilerplate and risks
maintenance divergence.

## Examples

Bad:

```cpp
// Bad: relying on #else omits fallback when capability is false at runtime
#if !defined(__EMSCRIPTEN__)
    if (fSharedContext->dawnCaps()->supportsPartialLoadResolve()) {
        wgpuRenderPass.nextInChain = &wgpuPartialRect;
    }
#else
    SkASSERT(resolveOffset.isZero());
#endif

// Bad: duplicating enclosing else blocks across preprocessor branches
#if !defined(SK_DISABLE_LEGACY_FONTMGR_REFDEFAULT)
else {
    tf = SkTypeface::MakeDeserialize(stream, SkFontMgr::RefDefault());
}
#else
else {
    tf = SkTypeface::MakeDeserialize(stream, nullptr);
}
#endif

// Bad: duplicating fallback return statements across #ifdef and #else branches
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    if (foundGainmap) {
        *info = gainmapInfo;
        return true;
    }
    return false;
#else
    return false;
#endif
```

Good:

```cpp
// Good: place '} else' before #endif so the shared fallback executes in all unsupported cases
#if !defined(__EMSCRIPTEN__)
    if (fSharedContext->dawnCaps()->supportsPartialLoadResolve()) {
        wgpuRenderPass.nextInChain = &wgpuPartialRect;
    } else
#endif
    {
        SkASSERT(resolveOffset.isZero());
    }

// Good: keep enclosing else block outside preprocessor directives
else {
#if !defined(SK_DISABLE_LEGACY_FONTMGR_REFDEFAULT)
    tf = SkTypeface::MakeDeserialize(stream, SkFontMgr::RefDefault());
#else
    tf = SkTypeface::MakeDeserialize(stream, nullptr);
#endif
}

// Good: share the trailing fallback return after #endif
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    if (foundGainmap) {
        *info = gainmapInfo;
        return true;
    }
#endif
    return false;
```
