---
name: skia/validate-optional-deserialization-has-value
description: When validating deserialization results returning std::optional alongside bytes-read out-parameters, check optional.has_value() directly rather than testing whether bytes-read is non-zero
rubric: correctness
severity: should_fix
---

**Rule.** When validating deserialization results from functions that return
std::optional<T> alongside a bytes-read count out-parameter, validate against
optional.has_value() directly rather than checking whether bytes read is
non-zero (such as size != 0).

**Why.** A non-zero byte count is an indirect output that does not guarantee
successful parsing; deserialization routines may consume partial bytes before
encountering invalid data or report bytes read on failure. Checking
optional.has_value() directly verifies valid object construction and prevents
fragile assumptions about consumed buffer sizes.

## Examples

Bad:

```cpp
size_t size = 0;
auto path = SkPath::ReadFromMemory(fCurr, this->available(), &size);
if (!this->validate((SkAlign4(size) == size) && (0 != size))) {
    path = std::nullopt;
}
```

Good:

```cpp
size_t size = 0;
auto path = SkPath::ReadFromMemory(fCurr, this->available(), &size);
(void)this->validate(SkAlign4(size) == size && path.has_value());
```
