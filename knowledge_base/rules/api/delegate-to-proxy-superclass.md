---
name: skia/api/delegate-to-proxy-superclass
description: When implementing or subclassing proxy classes like SkTypeface_proxy, forward virtuals, invoke proxy superclass methods when customizing, but delegate configured style attributes to SkTypeface
rubric: correctness
severity: should_fix
---

**Rule.** In proxy base classes (such as SkTypeface_proxy or
SkScalerContext_proxy), forward all virtual methods to the delegate—especially
auditing non-pure virtuals with default stubs (like onGetResourceName)—and
declare wrapped members private to prevent derived classes from bypassing proxy
logic. When subclassing proxy base classes, omit redundant forwarding overrides
that duplicate base behavior, and when customizing virtual methods (such as
onGetFontDescriptor), call the proxy superclass method first. However, when
wrapping with system-configured font style or pitch attributes, override
onGetFontStyle() and onGetFixedPitch() to delegate to SkTypeface rather than
SkTypeface_proxy.

**Why.** Omitting non-pure virtual overrides in proxy base classes causes calls
to silently fall back to default stubs instead of delegating to the wrapped
object. Keeping wrapped members private enforces proxy delegation, while
omitting trivial overrides removes boilerplate. Calling superclass methods first
ensures baseline properties are populated before customizations, whereas
delegating style and pitch to SkTypeface preserves wrapper-configured attributes
instead of querying the wrapped font's intrinsic properties.

## Examples

Bad:

```cpp
// In proxy base class header (e.g. SkTypeface_proxy.h)
class SkTypeface_proxy : public SkTypeface {
protected:
    sk_sp<SkTypeface> fProxy; // Bypasses encapsulation
    // Missing onGetResourceName: silently falls back to base stub returning 0
};

// In derived subclass (e.g. SkFontMgr_android.cpp)
class SkTypeface_Android : public SkTypeface_proxy {
    int onGetUPEM() const override { return SkTypeface_proxy::onGetUPEM(); } // Redundant override

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        // Misses calling superclass: baseline factory ID and properties are lost
        desc->setFamilyName(fFamilyName.c_str());
        *serialize = false;
    }

    // Delegating to proxy returns intrinsic font style instead of configured style
    SkFontStyle onGetFontStyle() const override { return SkTypeface_proxy::onGetFontStyle(); }
};
```

Good:

```cpp
// In proxy base class header (e.g. SkTypeface_proxy.h)
class SkTypeface_proxy : public SkTypeface {
public:
    int onGetResourceName(SkString* name) const override { return fProxy->getResourceName(name); }
private:
    sk_sp<SkTypeface> fProxy; // Enforces delegation through base methods
};

// In derived subclass (e.g. SkFontMgr_android.cpp)
class SkTypeface_Android : public SkTypeface_proxy {
    // Redundant forwarding overrides omitted

    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override {
        SkTypeface_proxy::onGetFontDescriptor(desc, serialize); // Populates baseline properties
        desc->setFamilyName(fFamilyName.c_str());
        *serialize = false;
    }

    // Delegates to SkTypeface to preserve wrapper-configured style and pitch attributes
    SkFontStyle onGetFontStyle() const override { return SkTypeface::onGetFontStyle(); }
    bool onGetFixedPitch() const override { return SkTypeface::onGetFixedPitch(); }
};
```
