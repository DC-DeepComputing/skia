---
name: skia/api/wrap-primitive-ids-in-typed-structs
description: When defining domain-specific identifiers, wrap primitive integers in strongly typed classes with private storage and asUInt() accessors rather than raw uint32_t or unencapsulated structs
rubric: readability
severity: should_fix
---

**Rule.** Wrap domain-specific identifiers, tokens, and session handles (such as
content, breakpoint, or context IDs) in dedicated strongly typed classes rather
than passing or storing raw primitive integers like uint32_t. Encapsulate the
underlying integer in a private member variable instead of exposing raw public
struct fields, and expose only minimal required methods: explicit constructors,
comparison operators (==, !=), a validity check (isValid() or operator bool()),
and an explicit accessor such as asUInt() const.

**Why.** Exposing raw integers or unencapsulated struct members couples callers
to internal representations, permits accidental interchange of unrelated counts
or IDs, and makes refactoring storage or bit widths error-prone. A strongly
typed, encapsulated class enforces type safety at compile time, clarifies API
contracts, and hides internal ID representation.

## Examples

Bad:

```cpp
// Bad: Passing raw integer for domain ID, or using struct with exposed integer member
virtual uint32_t createCaptureBreakpoint(SkSurface*) = 0;
void restoreBreakpoint(uint32_t id);

struct SkContentID {
    uint32_t id = 0;
};
```

Good:

```cpp
// Good: Strongly typed class encapsulating private storage and minimal accessors
class SkContentID {
public:
    SkContentID() = default;
    explicit constexpr SkContentID(uint32_t id) : fID(id) {}

    bool operator==(const SkContentID& other) const { return fID == other.fID; }
    bool operator!=(const SkContentID& other) const { return !(*this == other); }
    bool isValid() const { return fID != 0; }
    uint32_t asUInt() const { return fID; }

private:
    uint32_t fID = 0;
};

virtual SkContentID createCaptureBreakpoint(SkSurface*) = 0;
void restoreBreakpoint(SkContentID id);
```
