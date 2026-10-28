---
name: skia/suffix-compile-time-tag-parameters-with-tag
description: When passing compile-time tag types or std::bool_constant to generic lambdas or functions, suffix tag parameters with 'Tag' to distinguish them from runtime values
rubric: readability
severity: nit
---

**Rule.** When passing compile-time tag objects (such as std::bool_constant,
std::integral_constant, or tag structs) to generic lambdas or template helpers
for tag dispatch, suffix the tag parameter name with 'Tag' (e.g., isWindingTag
rather than isWinding). Ensure tag parameter names clearly signal that the
argument is a type tag used for compile-time deduction (such as
decltype(tag)::value) rather than a runtime value.

**Why.** In generic lambdas or functions accepting both compile-time tags and
runtime parameters, naming a tag parameter after a condition without a 'Tag'
suffix falsely implies it is a runtime boolean or object. Appending 'Tag' makes
its role as a compile-time type carrier explicit and avoids confusion when
extracting compile-time values via decltype.

## Examples

Bad:

```cpp
Dispatch(fillType, [&](auto isWinding, bool isInverse) {
    constexpr bool kIsWinding = decltype(isWinding)::value;
    ProcessStrips<kIsWinding>(isInverse);
});
```

Good:

```cpp
Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
    constexpr bool kIsWinding = decltype(isWindingTag)::value;
    ProcessStrips<kIsWinding>(isInverse);
});
```
