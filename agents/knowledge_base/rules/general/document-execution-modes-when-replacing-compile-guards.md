---
name: skia/document-execution-modes-when-replacing-compile-guards
description: When replacing compile-time preprocessor guards with runtime parameter flags or separate entry points, update comments to document execution-mode mappings and error-handling divergence
rubric: readability
severity: should_fix
---

**Rule.** When refactoring compile-time preprocessor guards (`#ifdef`) or
build-target separations into runtime parameter flags or dedicated entry points,
update explanatory comments to clarify how flag values correspond to execution
environments (such as standalone CLI tools vs. runtime library usage).
Explicitly document the rationale for diverging behavior across branches, such
as why one mode reports non-fatal errors to continue diagnostic collection while
another triggers fatal debug assertions (`SkDEBUGFAILF` or aborts).

**Why.** Preprocessor guards explicitly convey execution context and build
targets through macro structure, whereas runtime boolean parameters obscure
which callers select each branch. Clear comments detailing environment mappings
and error-handling policies prevent maintainers from confusing debug diagnostics
with fatal assertion failures or unintentionally breaking CLI tooling workflows.

## Examples

Bad:

```cpp
// Verify that the SPIR-V we produced is valid. At runtime, we will abort() with a message
// explaining the error. In standalone mode (skslc), we will send the message, plus the
// entire disassembled SPIR-V (for easier context & debugging) as *our* error message.
bool result = tools.Validate(programData, programSize);
if (!result) {
    if (disassemble) {
        reporter.error(Position(), errors);
    } else {
        SkDEBUGFAILF("%s", errors.c_str());
    }
}
```

Good:

```cpp
bool result = tools.Validate(programData, programSize);
if (!result) {
    // If disassemble is set, we won't abort but instead append errors. This allows
    // standalone tools (skslc) to collect all expected validation errors.
    if (disassemble) {
        reporter.error(Position(), errors);
    } else {
        // Normal runtime validation mode: abort in debug builds on invalid output.
        SkDEBUGFAILF("%s", errors.c_str());
    }
}
```
