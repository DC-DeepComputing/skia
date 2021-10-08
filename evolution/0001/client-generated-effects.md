# Client Generated Effects

* Proposal: [SK-0001](NNNN-filename.md)
* Authors: [Herb](https://skia-review.googlesource.com/dashboard/5210)
* Review Manager: TBD
* Status: **Awaiting implementation**

*During the review process, add the following fields as needed:*

* Implementation: [skia#NNNNN](https://skia.googlesource.com/skia/evolution/NNNN)
* Decision Notes: [Rationale](https://skia.googlesource.com/skia/evolution/NNNN/rational),
  [Additional Commentary](https://skia.googlesource.com/skia/evolution/NNNN/commentary)
* Bugs: [SK-NNNN](https://bugs.chromium.org/p/skia/issues/detail?id=NNNN),
  [SR-MMMM](https://bugs.chromium.org/p/skia/issues/detail?id=MMMM)
* Previous
  Revision: [1](https://skia.googlesource.com/skia/evolution/blob/...commit-ID.../proposals/NNNN-filename.md)
* Previous Proposal: [SKE-XXXX](XXXX-filename.md)

## Introduction

User interface designers would like to provide set pieces in their designs that delight users. These
set pieces or effects are not possible, are awkward to implement using the Skia API, or the
implementation using Skia is too inefficient.

## Motivation

These delights can be encoded in SkSL, but deploying current SkSL using the software renderer is
problematic. Currently, the system uses a JIT for AVX2 and ARM64 chips, but jitting system is slow,
complicated and contains a lot of source. If the JIT is not available, then an interpreter is used
to evaluate the code. In addition, the JIT would need major refitting to accommodate flow control in
the proposed future changes of SkSL.

There are two broad uses cases for how client generated effects are used; enhancing the user
experience for end users, and building new set pieces to delight users by UI creators. The resource
use of these two uses cases are at odds with each other. The creator wants to produce small power
efficient code to implement the set pieces for the end user, but the creators themselves want a very
flexible system to explore the design space of set pieces. The current JIT/interpreter system
doesn't really satisfy either use case.

## Proposed solution

I propose that we produce a tool built on new Skia module called SkDelight. For optimized end user
systems, SkDelight would back a tool that takes SkSL as input at compile time and produce a
SkRasterPipeline segment directly into a .o file. In addition, the module could be linked into Skia,
and produce SkRasterPipeline segments as DLL which could be used by content creation systems like
Adobe AfterEffects producing a flexible environment for content creators. SkDelight would have LLVM
as dependency, but LLVM would only need to be shipped when Skia is used in a content creation tool.

## Detailed design

Describe the design of the solution in detail. If it's a new API, show the full API and its
documentation comments detailing what it does. The detail in this section should be sufficient for
someone who is *not* one of the authors to be able to reasonably implement the feature.

## API compatibility

Skia API compatibility describes how this feature will interact with existing APIs and concepts.
Will client's need to change their code to accommodate this change? Will client's need to account
for new behavior in the existing API? Will this change be in a separate module?

## Alternatives considered

Describe alternative approaches to addressing the same problem, and why you chose this approach
instead.

## Acknowledgments

If significant changes or improvements suggested by members of the community were incorporated into
the proposal as it developed, take a moment here to thank them for their contributions. Skia
evolution is a collaborative process, and everyone's input should receive recognition!
