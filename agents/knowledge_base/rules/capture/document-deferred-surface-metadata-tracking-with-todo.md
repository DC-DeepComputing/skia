---
name: skia/capture/document-deferred-surface-metadata-tracking-with-todo
description: When storing captured drawing assets without tracking their originating surface or content ID, document deferred metadata tracking with an attributed TODO
rubric: readability
severity: should_fix
---

**Rule.** When recording or capturing drawing assets (such as SkPictures) from
surfaces or canvases without retaining their association to the originating
surface or content ID, document the deferred metadata tracking with an
attributed TODO(b/...) comment. Clarify the planned content ID and surface
tracking requirements for downstream serialization and replay.

**Why.** Downstream serialization, playback, and dependency analysis require
knowing which surface or content ID an asset originated from, especially when
snapshots are reused across canvases. When an incremental implementation stores
captured assets in a flat container without recording that link, adding an
attributed TODO preserves architectural intent and tracks the technical debt.

## Examples

Bad:

```cpp
if (picture) {
    fPictures.emplace_back(picture);
}
```

Good:

```cpp
if (picture) {
    // TODO(b/412351769): For every stored picture, track its content ID
    // and originating surface.
    fPictures.emplace_back(picture);
}
```
