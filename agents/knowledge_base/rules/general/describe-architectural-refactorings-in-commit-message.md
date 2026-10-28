---
name: skia/describe-architectural-refactorings-in-commit-message
description: When authoring commit messages, detail operational motivation, defect context, dependency pinning, and prerequisites, keeping descriptions accurate to actual changes and existing utilities
rubric: readability
severity: should_fix
---

**Rule.** In commit messages, provide an explanatory body detailing operational
motivation, defect context, and fix mechanisms ('why' and 'how') rather than a
bare subject or diff summary; never mislabel bug fixes as 'refactoring'. Specify
capability gating ('when available') for conditional features rather than
implying unconditional replacement. For dependency updates, specify concrete
versions and rationale, including build breakages when pinning non-HEAD
revisions. Itemize intended end-states and prerequisite CLs for refactorings.
Ensure descriptions accurately reflect actual changes and existing utilities
(avoiding inaccurate technical claims about APIs), updating titles and removing
omitted, deferred, or non-functional configurations. For stacked series,
describe each patch's mechanism before series-wide summaries.

**Why.** Reviewers and future maintainers need operational motivation, defect
context, and accurate descriptions to understand why a change was necessary and
how it behaves. A bare subject line or vague diff summary omits the rationale
behind enabling experiments or features. Mislabeling bug fixes as refactoring
misleads reviewers and complicates bisection. Implying unconditional replacement
for conditionally gated features obscures fallback execution paths. Omitting the
rationale for dependency updates or non-HEAD pins leaves maintainers and roll
tooling unaware of build breakages or version constraints. Omitting prerequisite
CLs, claiming support for omitted or non-functional configurations,
misrepresenting existing codebase utilities, leaving outdated titles or
descriptions, or relying solely on stack-level summaries obscures repository
evolution, misleads audits, and complicates git archaeology.

## Examples

Bad:

```cpp
// Omitting dependency pinning rationale or concrete versions:
Update dependency on abseil (for Dawn)

// Implying unconditional replacement for conditionally gated features:
Replace the existing emulated resolve implementation with Dawn's partial resolve feature when rendering via the D3D11 backend.

// Claiming support or test configurations that were omitted or non-functional:
Add jobs for MotoG73

This adds correctness and performance tests for:
 - Ganesh GL
 - Ganesh Vulkan
 - Native Graphite Vulkan
 - Dawn Graphite Vulkan

// Inaccurate claims about existing codebase utilities:
Remove redundant SkDoubleIsNaN function

Skia already has a double specialization of SkIsNan in SkFloatingPoint.h

// Stale title or description after review alters approach or file structure:
Make it possible to use most of MtlGraphiteTypes from C++

Change VertexWriters to always explicitly require an alignment on construction.

// Providing only a change stack summary without describing the specific CL:
[graphite] Snap scissor rects to 4px boundaries

Clip perf summary -----------------
By the end of this change stack (starting with CL 1009396), motionmark
GPU work improves.
```

Good:

```cpp
// Documenting dependency pinning rationale for non-HEAD commit:
Update dependency on abseil (for Dawn)

The latest version didn't work because of a missing file in the GN
build.

Change-Id: I281c8902e04dc880a54fde79e33470e7e4966edb

// Specifying concrete upstream dependency version:
Update SkVideoEncoder for new ffmpeg

Starting with ffmpeg 7.0, the write callback signature has changed.

// Accurately describing implemented configurations and noting deferred work:
Add jobs for MotoG73

This adds correctness and performance tests for:
 - Ganesh GL

Others don't yet work (see linked bug)

// Accurately describing existing codebase utilities:
Remove redundant SkDoubleIsNaN function

Skia already has SkIsNan in SkFloatingPoint.h

// Accurately describing conditional feature enablement:
Replace the existing emulated resolve implementation with Dawn's partial resolve feature when available.

// Describing specific patch mechanism before stack-level summary:
[graphite] Snap scissor rects to 4px boundaries

Before this change, the clip stack set the scissor per draw to the
outer pixel bounds of the stack. This change snaps the scissor to a
4px boundary containing the original scissor.

Clip perf summary -----------------
By the end of this change stack (starting with CL 1009396), motionmark
GPU work improves.
```
