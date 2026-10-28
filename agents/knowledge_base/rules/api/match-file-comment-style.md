---
name: skia/api/match-file-comment-style
description: When adding comments, use // line comments inside functions, match established /** ... */ declaration style, and keep sibling member doc comments consistently formatted
rubric: readability
severity: should_fix
---

**Rule.** Always use '//' line comments for implementation notes inside function
bodies, even for multi-line explanations; do not introduce '/* ... */' or '/**
... */' block comments inside functions. For declarations, headers, and
file-level documentation, match the file's established style by using '/** ...
*/' Doxygen block comments where established, rather than introducing
inconsistent '//' line comments. Additionally, within an enum, struct, or class
declaration, keep documentation comment formatting consistent across sibling
members; do not mix trailing inline doc comments (e.g., `//!<`) with preceding
multiline block comments (`/** ... */`).

**Why.** In Skia, implementation code inside function bodies conventionally uses
'//' line comments, reserving '/** ... */' Doxygen block comments for
declarations outside functions. Mixing block comments into function bodies
disrupts visual scanning, while mixing trailing inline doc comments with
preceding block comments within the same enum or struct produces a disjointed
layout that impairs readability.

## Examples

Bad:

```cpp
// Bad: Using block comments inside a function body:
void CheckFormat(const BufferDesc& bufferDesc) {
    /**
     * Check for a special case - the usage of AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM - when
     * building for the Android framework.
     */
    if (bufferDesc.format == AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM) { ... }
}

// Bad: Mixing trailing inline comments and preceding block comments within an enum:
enum AddPathMode {
    kAppend_AddPathMode, //!< appended to destination as new contours
    /** extends open last contour of destination with line to start of source */
    kExtend_AddPathMode,
};
```

Good:

```cpp
// Good: Use // line comments inside function bodies, even for multi-line notes:
void CheckFormat(const BufferDesc& bufferDesc) {
    // Check for a special case - the usage of AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM - when
    // building for the Android framework.
    if (bufferDesc.format == AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM) { ... }
}

// Good: Keep comment style consistent across sibling enum/struct members:
enum AddPathMode {
    /** Contours are appended to the destination path as new contours. */
    kAppend_AddPathMode,
    /** Extends open last contour of destination with line to start of source. */
    kExtend_AddPathMode,
};
```
