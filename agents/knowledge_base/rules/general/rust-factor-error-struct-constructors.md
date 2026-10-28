---
name: skia/rust-factor-error-struct-constructors
description: When returning error result structs across early-exit branches, encapsulate repeated struct initialization and boxing in associated constructor helpers rather than open-coding boilerplate
rubric: design
severity: should_fix
---

**Rule.** When functions return composite result structs or boxed types on
failure across multiple early exits, encapsulate repeated struct instantiation
in associated constructor helper functions (such as Self::error(...) or
StructName::error(...)). Do not duplicate boilerplate struct fields, default
collections, or heap allocations (Box::new(...)) across individual
error-handling branches.

**Why.** Inlining struct literals and heap allocations across multiple error
exit points duplicates boilerplate fields (such as default enum values or empty
vectors), leaks internal initialization details across parsing branches, and
creates maintenance hazards if the struct layout or default state changes.

## Examples

Bad:

```cpp
if !is_ico_data(data) {
    return Box::new(DirectoryResult {
        status: DirectoryParseResult::InvalidSignature,
        file_type: IcoFileType::Icon,
        entries: Vec::new(),
    });
}
```

Good:

```cpp
impl DirectoryResult {
    fn error(status: DirectoryParseResult) -> Box<Self> {
        Self::error_for(status, IcoFileType::Icon)
    }

    fn error_for(status: DirectoryParseResult, file_type: IcoFileType) -> Box<Self> {
        Box::new(Self { status, file_type, entries: Vec::new() })
    }
}

// Call site:
if !is_ico_data(data) {
    return DirectoryResult::error(DirectoryParseResult::InvalidSignature);
}
```
