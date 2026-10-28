---
name: skia/tools/name-viewer-slides-descriptively
description: When defining Viewer slides via DEF_SLIDE or Slide::fName, name slides descriptively after the scenario or feature tested rather than positional labels
rubric: readability
severity: nit
---

**Rule.** When defining Viewer slides (via DEF_SLIDE, Slide subclasses, or
setting Slide::fName), name each slide descriptively after the specific feature,
edge case, or scenario being exercised rather than using positional,
sequence-based, or generic names like SlideLast, SlideNext, or Slide1. Ensure
the slide's registered identifier matches this descriptive scenario.

**Why.** Positional or sequential slide names obscure what scenario is being
tested and cause confusing review diffs when slides are reordered or inserted.
Descriptive scenario names ensure slides can be searched and selected via the
Viewer UI filter and CLI flags (--slide, --match).

## Examples

Bad:

```cpp
// Bad: Using a positional name that obscures the tested scenario
class ParagraphSlideLast : public ParagraphSlide_Base {
public:
    ParagraphSlideLast() { fName = "ParagraphSlideLast"; }
    void draw(SkCanvas* canvas) override {
        ...
        paragraph_style.setEllipsis(u"\u2026");
    }
};
DEF_SLIDE(return new ParagraphSlideLast();)
```

Good:

```cpp
// Good: Naming the slide after the specific scenario under test
class ParagraphSlideEllipsisInRTL : public ParagraphSlide_Base {
public:
    ParagraphSlideEllipsisInRTL() { fName = "ParagraphSlideEllipsisInRTL"; }
    void draw(SkCanvas* canvas) override {
        ...
        paragraph_style.setEllipsis(u"\u2026");
    }
};
DEF_SLIDE(return new ParagraphSlideEllipsisInRTL();)
```
