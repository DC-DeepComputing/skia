---
name: skia/modules/gate-skparagraph-layout-changes
description: When introducing SkParagraph layout or shaping changes, gate them behind opt-in ParagraphStyle flags rather than adding parameters to Paragraph::layout()
rubric: design
severity: blocking
---

**Rule.** Gate new SkParagraph text layout, line-breaking, and shaping behaviors
that alter existing text output behind runtime configuration flags on
ParagraphStyle (such as setRenderSoftHyphens() or setApplyRoundingHack()) rather
than adding optional parameters to Paragraph::layout() or paint(). Default these
options to false (or the established legacy behavior) so existing client layouts
remain unaffected until embedders explicitly opt in.

**Why.** Downstream embedders like Flutter and Chromium rely on SkParagraph's
established line breaks, text metrics, and glyph shaping. Configuring behavior
via ParagraphStyle flags preserves clean execution method signatures, avoids
widespread call-site churn across tests and clients, and allows embedders to
migrate and test rendering changes safely.

## Examples

Bad:

```cpp
// Bad: adding optional boolean configuration flags to core layout() method
paragraph->layout(TestCanvasWidth, /*applyRoundingHack=*/false);

// Bad: unconditionally executing new layout or shaping behavior for all clients
line.createSoftHyphen();
```

Good:

```cpp
// modules/skparagraph/include/ParagraphStyle.h
bool applyRoundingHack() const { return fApplyRoundingHack; }
void setApplyRoundingHack(bool value) { fApplyRoundingHack = value; }
...
bool fApplyRoundingHack = true;

// Configure behavior on style before layout without changing layout() signature
ParagraphStyle style;
style.setApplyRoundingHack(false);
...
paragraph->layout(TestCanvasWidth);
```
