---
name: skia/gpu/encapsulate-transformed-command-parameters
description: When passing command parameters requiring context-dependent transformations, encapsulate raw coordinates in dedicated types whose accessors enforce passing transformation state
rubric: design
severity: should_fix
---

**Rule.** Encapsulate command parameters that require context-dependent
transformations (such as replay translation or clipping) into dedicated types
whose accessors require that transformation state to retrieve coordinates. Do
not pass raw geometric types (like SkIRect) across command buffer boundaries and
rely on backend subclasses to manually invoke base-class transformation helpers.

**Why.** Relying on backend command buffer implementations to manually call
base-class transformation helpers is error-prone, as implementers can easily
omit the transformation call. Encapsulating raw data within dedicated types
whose getters demand the transformation context enforces correct coordinate
handling at compile time.

## Examples

Bad:

```cpp
// Base CommandBuffer provides a helper that backend subclasses must remember to call:
SkIRect CommandBuffer::transformScissor(unsigned int left, unsigned int top,
                                        unsigned int width, unsigned int height);

void DawnCommandBuffer::setScissor(unsigned int left, unsigned int top,
                                   unsigned int width, unsigned int height) {
    SkIRect scissor = this->transformScissor(left, top, width, height);
    fActiveRenderPassEncoder.SetScissorRect(
            scissor.x(), scissor.y(), scissor.width(), scissor.height());
}
```

Good:

```cpp
// Encapsulate raw rect privately in a type that requires transformation state to access:
class Scissor {
public:
    explicit Scissor(const SkIRect& rect) : fRect(rect) {}
    SkIRect getRect(const SkIVector& replayTranslation, const SkIRect& replayClip) const {
        SkIRect rect = fRect.makeOffset(replayTranslation);
        if (!rect.intersect(replayClip)) { rect.setEmpty(); }
        return rect;
    }
private:
    const SkIRect fRect;
};

void DawnCommandBuffer::setScissor(const Scissor& scissor) {
    SkIRect rect = scissor.getRect(fReplayTranslation, fReplayClip);
    fActiveRenderPassEncoder.SetScissorRect(
            rect.x(), rect.y(), rect.width(), rect.height());
}
```
