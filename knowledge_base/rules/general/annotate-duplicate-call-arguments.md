---
name: skia/annotate-duplicate-call-arguments
description: When passing identical expressions or variables to consecutive function parameters, annotate each argument with parameter-name comments to clarify intentional duplication
rubric: readability
severity: nit
---

**Rule.** When passing identical expressions, variables, or function calls to
consecutive parameters of the same type at a call site, annotate each argument
with a parameter-name comment (e.g., /*param=*/ or trailing // param). Clarify
each argument's semantic role so readers and reviewers know the duplicate value
is deliberate rather than an accidental copy-paste error.

**Why.** Passing repeated identical arguments to adjacent parameters looks like
an unintentional copy-paste bug. Parameter-name comments confirm that the
repeated value is intentional and make it obvious which parameter each argument
populates without needing to inspect the function declaration.

## Examples

Bad:

```cpp
return SkEncodedInfo::Make(width,
                           height,
                           skColor,
                           ToAlpha(rustColor, reader),
                           reader.output_bits_per_component(),
                           reader.output_bits_per_component(),
                           std::move(profile),
                           hdrMetadata);
```

Good:

```cpp
return SkEncodedInfo::Make(width,
                           height,
                           skColor,
                           ToAlpha(rustColor, reader),
                           reader.output_bits_per_component(), // bitsPerComponent
                           reader.output_bits_per_component(), // colorDepth
                           std::move(profile),
                           hdrMetadata);
```
