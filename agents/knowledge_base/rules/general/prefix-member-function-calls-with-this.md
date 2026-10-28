---
name: skia/prefix-member-function-calls-with-this
description: When calling instance member functions, prefix with this-> instead of class qualification or bare calls; omit this-> for static methods and f-fields
rubric: readability
severity: should_fix
---

**Rule.** Prefix non-static (instance) member function calls on the current
object with 'this->' (e.g., 'this->method()') when invoked from within member
functions, rather than calling them unqualified or qualifying them with the
current class name ('Class::method()'), unless explicitly invoking a base-class
override like 'Parent::method()'. Conversely, do not prefix static member
function calls with 'this->' (call them unqualified or with class qualification
like 'Class::StaticMethod()'), and do not prefix member variable accesses with
'this->'; access 'f'-prefixed member fields directly (e.g., 'fMember', not
'this->fMember').

**Why.** Explicitly dereferencing 'this->' distinguishes instance member
function calls from free helper or static functions at a glance and makes member
dispatch transparent, whereas qualifying instance calls with the current class
name misleadingly mimics static method calls or parent overrides. Conversely,
static member functions do not operate on an instance, so invoking them via
'this->' is redundant and misleadingly implies instance state access. Similarly,
Skia's member fields are already distinguished from local variables and
arguments by their leading 'f' prefix, making 'this->' qualification redundant
when accessing variables.

## Examples

Bad:

```cpp
// Bad: calling instance methods without this->, qualifying with current class, or prefixing fields/static methods with this->
bool BufferManager::hasSpace(size_t requiredBytes, size_t alignStride) {
    this->fAvoidStencilBuffers = true;
    return willBufferOverflow(&fCurrentBuffers[kVertexBufferIndex], requiredBytes, alignStride);
}
uint16_t tileColumns = this->DivCeil(viewportWidth, kTileWidth) - 1;
sk_sp<SkShader> SkBitmap::makeShader(const SkSamplingOptions& sampling, const SkMatrix* lm) const {
    return SkBitmap::makeShader(SkTileMode::kClamp, SkTileMode::kClamp, sampling, lm);
}
```

Good:

```cpp
// Good: prefix instance method calls with this->, access fields directly, and omit this-> on static methods
bool BufferManager::hasSpace(size_t requiredBytes, size_t alignStride) {
    fAvoidStencilBuffers = true;
    auto [overflows, offset] = this->willBufferOffsetOverflow(
            &fCurrentBuffers[kVertexBufferIndex], requiredBytes, alignStride);
    return !overflows;
}
uint16_t tileColumns = DivCeil(viewportWidth, kTileWidth) - 1;
sk_sp<SkShader> SkBitmap::makeShader(const SkSamplingOptions& sampling, const SkMatrix* lm) const {
    return this->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, sampling, lm);
}
```
