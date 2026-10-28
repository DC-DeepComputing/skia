---
name: skia/gpu/document-resource-lifecycle-by-sharing-category
description: When documenting GPU resource lifecycle, thread safety, or labeling models, explain backend sync deferral and structure invariants explicitly by resource-sharing category
rubric: readability
severity: should_fix
---

**Rule.** When documenting thread-safety models, mutation invariants, or
lifecycle update policies for GPU resources, structure the explanation
explicitly around discrete resource-sharing categories (such as initial
creation, shareable, non-shareable, and scratch/task-local) rather than
procedural descriptions. Additionally, when documenting the Resource labeling
model, clarify that internal Resource labels exist unconditionally for debugging
even when backend object labels are disabled via Caps, and document that
synchronizing label updates to backend objects upon reuse is deferred to
recording insertion (insertRecording) to prevent worker-thread data races and
maintain capture stability.

**Why.** Categorizing multi-threaded lifecycle, caching, and mutation policies
by discrete sharing tiers provides maintainers with a clear mental model of
safety invariants across threads. Clarifying that internal labels are
unconditional while backend synchronization is deferred to insertRecording
explains how Graphite avoids data races across concurrent worker threads and
prevents clobbering labels during active GPU/web captures.

## Examples

Bad:

```cpp
 * Resource labels, if enabled, are updated in two steps:
 * 1. Setting the label on the Resource object.
 * 2. Syncing the updated label to the backend resource.
 *
 * Label update policy per resource shareability:
 *   - Shareable resources can be returned from the cache at any time. Therefore, the resource
 *     provider should only update shareable resource labels upon initial insertion into the cache.
```

Good:

```cpp
 * Every Resource has a label; backend objects optionally store labels (via Caps) for debugging.
 * Outside of initial creation, syncing label updates to backend objects is deferred to
 * insertRecording to prevent cross-thread data races and maintain capture stability.
 *
 * Resource thread-safe label update policy based on shareability:
 *   - Any resource can set its label at initial creation or cache insertion.
 *   - Shareable resource labels are immutable and never change upon cache reuse.
 *   - Non-shareable resource labels can be updated by ResourceCache upon exclusive reuse.
 *   - Scratch resources manage labels across and within recorder threads.
```
