---
name: skia/api/implement-internal-helpers-on-priv-classes
description: When adding internal helpers, experimental APIs, feature queries, or state for public include/ classes, implement them in src/ companion *Priv types instead of public headers
rubric: design
severity: should_fix
---

**Rule.** When adding internal helpers, feature-flag queries, niche methods,
subsystem operations (such as pathops_*), test accessors, or experimental and
unsettled API methods/constructors for public API classes, do not expose them on
public interfaces under include/; keep internal plumbing and dispatch in src/,
or implement them in src/ on companion *Priv classes (such as RecorderPriv,
SkPathPriv, or SkCanvasPriv). Do not define redundant accessor methods on a
class to expose fields to an already-friended companion *Priv class; the friend
class accesses private fields directly. Correspondingly, internal callers
needing private class state must use existing *Priv helpers (such as
SkCanvasPriv::TopDevice) rather than adding caller-specific friend declarations
to public headers.

**Why.** Public headers under include/ define Skia's public API surface and
external contracts; exposing internal helpers, experimental features, or queries
leaks subsystem details, commits to unstable designs prematurely, and creates
backward-compatibility obligations. Handling internal configuration, dispatch,
and experimental APIs in src/ or routing private access through companion *Priv
types confines private workflows to library internals without indirection or
public header pollution.

## Examples

Bad:

```cpp
// include/core/SkCanvas.h
class SK_API SkCanvas {
    // Bad: Adding caller-specific friend declarations in public header
    friend class SkCaptureCanvas;

    struct SaveLayerRec {
        // Bad: Exposing experimental or unsettled constructor in public header
        SaveLayerRec(const SkRect* bounds, const SkPaint* paint, const SkImageFilter* backdrop,
                     SaveLayerFlags flags, FilterSpan filters = {})
            : SaveLayerRec(bounds, paint, backdrop, 1.f, flags, filters) {}
    };
};

// include/effects/SkRuntimeEffect.h
class SK_API SkRuntimeEffect : public SkRefCnt {
private:
    // Bad: Redundant accessor method for a companion friend class
    uint32_t stableKey() const { return fStableKey; }

    friend class SkRuntimeEffectPriv;
    uint32_t fStableKey = 0;
};

// include/core/SkPathBuilder.h
class SK_API SkPathBuilder {
public:
    // Bad: Niche internal helper, inspection hook, and subsystem method exposed publicly
    bool isZeroLengthSincePoint(int startPtIndex) const;
    int getVerbs(uint8_t verbs[], int max) const;
    SkPathBuilder& pathops_reversePathTo(const SkPath& path);
};

// include/codec/SkAndroidCodec.h
class SK_API SkAndroidCodec : SkNoncopyable {
public:
    // Bad: Exposing internal feature-flag query in public header
    static bool AvifEnabled();
};
```

Good:

```cpp
// src/core/SkCanvasPriv.h
class SkCanvasPriv {
public:
    // Good: Experimental or unsettled layer constructor staged in internal companion header
    static SkCanvas::SaveLayerRec ScaledBackdropLayer(const SkRect* bounds,
                                                      const SkPaint* paint,
                                                      const SkImageFilter* backdrop,
                                                      SkScalar scale,
                                                      SkCanvas::SaveLayerFlags flags,
                                                      SkCanvas::FilterSpan filters = {}) {
        return SkCanvas::SaveLayerRec(bounds, paint, backdrop, scale, flags, filters);
    }
};

// include/effects/SkRuntimeEffect.h
class SK_API SkRuntimeEffect : public SkRefCnt {
private:
    // Good: Friend class accesses private member field directly without redundant accessor
    friend class SkRuntimeEffectPriv;
    uint32_t fStableKey = 0;
};

// src/core/SkRuntimeEffectPriv.h
class SkRuntimeEffectPriv {
public:
    static uint32_t StableKey(const SkRuntimeEffect& effect) { return effect.fStableKey; }
};

// include/core/SkPathBuilder.h
class SK_API SkPathBuilder {
private:
    // Good: Internal helper kept private; accessible via friend *Priv
    bool isZeroLengthSincePoint(int startPtIndex) const;
    friend class SkPathPriv;
};

// src/core/SkPathPriv.h - inspection and subsystem operations handled in src/
class SkPathPriv {
    static SkSpan<const SkPathVerb> GetVerbs(const SkPathBuilder& b) { return b.fVerbs; }
    static void ReversePathTo(SkPathBuilder* b, const SkPath& path) {
        b->privateReversePathTo(path);
    }
};

// Caller: access private canvas state via existing companion *Priv helper
SkCanvasPriv::TopDevice(fBaseCanvas)->recorder()->isCapturing();
```
