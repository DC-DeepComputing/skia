---
name: skia/ports/avoid-drive-by-method-reordering-and-access-churn
description: When refactoring class hierarchies in port implementations, preserve existing method ordering and visibility specifiers instead of introducing drive-by access churn
rubric: design
severity: should_fix
---

**Rule.** When refactoring class hierarchies or proxy wrappers, do not reorder
existing member methods or modify their access specifiers (such as changing
public to protected) unless directly required by the change's design. Keep
existing method positions and visibility intact to prevent unnecessary diff
churn and retain predictable interface layout.

**Why.** Gratuitous reorganization and arbitrary visibility changes clutter
review diffs, obscure intentional behavioral edits, and risk altering external
or derived class access invariants without architectural necessity.

## Examples

Bad:

```cpp
// In a refactoring CL modifying base class or proxy usage:
// Reordering existing methods and introducing 'protected:' to previously public methods
public:
    std::unique_ptr<SkStreamAsset> makeStream() const;
    const SkString fFamilyName;
protected:
    void onGetFamilyName(SkString* familyName) const override;
```

Good:

```cpp
// Retain existing method ordering and visibility to avoid diff churn:
public:
    std::unique_ptr<SkStreamAsset> makeStream() const;
    void onGetFamilyName(SkString* familyName) const override;
    const SkString fFamilyName;
```
