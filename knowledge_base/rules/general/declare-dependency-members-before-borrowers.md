---
name: skia/declare-dependency-members-before-borrowers
description: When declaring class members where one field references or borrows from another, declare dependency storage before dependent borrower members for safe reverse-order destruction
rubric: safety
severity: should_fix
---

**Rule.** When declaring class data members where one member depends on, borrows
from, or references another (such as a reader, adapter, view, or raw pointer
borrowing an underlying stream, buffer, or symbol table), declare the dependency
or storage member before the dependent member. Ensure member destruction in
reverse declaration order dismantles the dependent object first while the
underlying dependency remains valid.

**Why.** In C++, class data members are destroyed in reverse order of
declaration. If a dependent borrower is declared before the backing resource it
references, the backing resource is destroyed first during class teardown,
leaving the borrower with a dangling pointer or referencing deallocated memory
in its destructor and causing use-after-free crashes.

## Examples

Bad:

```cpp
class JpegDecoder {
    // BUG: C++ destroys members in reverse declaration order, so fPrivStream
    // is destroyed before fReader, causing use-after-free if the reader touches
    // the stream during teardown.
    rust::Box<rust_jpeg::Reader> fReader;
    std::unique_ptr<SkStream> fPrivStream;
};
```

Good:

```cpp
class JpegDecoder {
    // Dependencies declared before borrowers so fReader is destroyed first.
    std::unique_ptr<SkStream> fPrivStream;
    rust::Box<rust_jpeg::Reader> fReader;
};
```
