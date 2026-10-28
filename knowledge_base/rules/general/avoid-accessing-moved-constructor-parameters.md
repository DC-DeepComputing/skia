---
name: skia/avoid-accessing-moved-constructor-parameters
description: When moving an object or parameter in call arguments or initializer lists, do not access it in sibling arguments or constructor bodies
rubric: correctness
severity: blocking
---

**Rule.** Do not access a variable or constructor parameter in an argument
expression if it is moved in another argument of the same call or initializer
list, as C++ argument evaluation is indeterminately sequenced; evaluate and
store needed properties in local variables before the move. Additionally, do not
access constructor parameters in the constructor body after moving from them in
the member initializer list; reference the initialized member variable (fMember)
instead.

**Why.** In C++, function and constructor argument expressions are
indeterminately sequenced, so moving an object in one argument may nullify or
empty it before a sibling argument accesses it. Similarly, moving a parameter in
a member initializer list leaves it empty before the constructor body executes,
causing null-pointer dereferences or use-after-move bugs.

## Examples

Bad:

```cpp
// Bad: Accessing realTypeface in sibling constructor call arguments where it is moved:
SkTypeface_Fuchsia(sk_sp<SkTypeface> realTypeface, TypefaceId id)
        : SkTypeface_proxy(std::move(realTypeface),
                           realTypeface->fontStyle(),
                           realTypeface->isFixedPitch())
        , fId(id) {}

// Bad: Accessing scanner in constructor body after moving in initializer list:
SkFontMgr_AndroidNDK(..., std::unique_ptr<SkFontScanner> scanner)
        : fAPI(androidFontAPI)
        , fScanner(std::move(scanner)) {
    while (SkAFont font = fontIter.next()) {
        sk_sp<SkTypeface_AndroidNDK> typeface =
                this->make(std::move(font), cacheFontFiles, scanner.get());
    }
}
```

Good:

```cpp
// Good: Query and store properties in local variables before moving in a call/factory:
const SkFontStyle style = realTypeface->fontStyle();
bool isFixedPitch = realTypeface->isFixedPitch();
return SkTypeface_Fuchsia::Make(std::move(realTypeface), style, isFixedPitch, id);

// Good: Access initialized member fScanner instead of moved scanner parameter:
SkFontMgr_AndroidNDK(..., std::unique_ptr<SkFontScanner> scanner)
        : fAPI(androidFontAPI)
        , fScanner(std::move(scanner)) {
    while (SkAFont font = fontIter.next()) {
        sk_sp<SkTypeface_AndroidNDK> typeface =
                this->make(std::move(font), cacheFontFiles, fScanner.get());
    }
}
```
