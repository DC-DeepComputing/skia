---
name: skia/format-multiline-doxygen-comments-with-asterisks
description: When writing multiline Doxygen comments, place /** and */ on their own lines and indent each continuation line with ` * ` rather than omitting asterisks
rubric: readability
severity: nit
---

**Rule.** When formatting multiline Doxygen doc comments (/** ... */), place the
opening /** on its own line, indent each continuation line with ` * `, and place
the closing */ on its own line. Do not place comment text directly on the
opening or closing delimiter lines, and do not omit leading asterisks on
continuation lines.

**Why.** Placing delimiter markers on separate lines and prefixing continuation
lines with ` * ` conforms to standard Doxygen comment formatting across Skia.
This ensures documentation tooling parses block contents reliably and maintains
consistent visual indentation and scanning alongside declarations.

## Examples

Bad:

```cpp
// 1. Text placed directly on opening or closing delimiter lines:
/** Class that adds methods to PrecompileContext that are only intended for use internal to Skia.
 *  This class is purely a privileged window into PrecompileContext.
 */

// 2. Missing continuation asterisks on multiline doc comments:
/**
Class that adds methods to PrecompileContext that are only intended for use internal to Skia.
This class is purely a privileged window into PrecompileContext.
*/
```

Good:

```cpp
/**
 * Class that adds methods to PrecompileContext that are only intended for use internal to Skia.
 * This class is purely a privileged window into PrecompileContext. It should never have additional
 * data members or virtual methods.
 */
class PrecompileContextPriv {
    ...
};
```
