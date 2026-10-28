---
name: skia/capitalize-static-class-and-externed-functions
description: Use UpperCamelCase for static class and header-declared free functions, lowerCamelCase for member methods, and static snake_case without Sk-prefixes for local helpers
rubric: readability
severity: should_fix
---

**Rule.** Name static class functions and non-member free functions declared in
headers (externed functions) using UpperCamelCase with an initial capital letter
(e.g., RunAll(), FooInstanceCount(), ToStr()), never lowerCamelCase or
snake_case. Conversely, use lowerCamelCase for non-static class member methods
(e.g., executeStep(), trimTo()), and lowercase snake_case strictly for static
non-class and anonymous-namespace helper functions in implementation (.cpp)
files (e.g., compute_naive_mask(), decode_bmp()). Never use Sk-prefixed
PascalCase names for file-local or test helpers, and do not leave local helpers
with external linkage; declare them static or enclose them in an anonymous
namespace.

**Why.** Following Skia's naming conventions distinguishes function roles and
linkage scopes at a glance: UpperCamelCase and the Sk- prefix are reserved for
public or externally visible APIs and static class methods, lowerCamelCase
signifies member methods, and lowercase snake_case with static or
anonymous-namespace linkage is reserved strictly for translation-unit-local
helpers.

## Examples

Bad:

```cpp
// Bad: free function declared in header uses snake_case.
// In PaintParamsTestUtils.h
const char* to_str(ShaderType s);

// Bad: static class function uses lowerCamelCase; member method uses UpperCamelCase.
class FlattenTestRunner {
public:
    static void runAll(skiatest::Reporter* reporter);
    void ExecuteStep();
};

// Bad: translation-unit helper uses camelCase, or Sk-prefixed PascalCase without static linkage.
namespace {
uint8_t computeNaiveMask(SkPoint p0, SkPoint p1);
}
std::unique_ptr<SkCodec> SkBmpRustDecoderDecode(skiatest::Reporter* r, const char* path);
```

Good:

```cpp
// Good: free function declared in header uses UpperCamelCase.
// In PaintParamsTestUtils.h
const char* ToStr(ShaderType s);

// Good: static class function uses UpperCamelCase; member method uses lowerCamelCase.
class FlattenTestRunner {
public:
    static void RunAll(skiatest::Reporter* reporter);
    void executeStep();
};

// Good: translation-unit helpers use static or anonymous-namespace lowercase snake_case.
namespace {
uint8_t compute_naive_mask(SkPoint p0, SkPoint p1);
}
static std::unique_ptr<SkCodec> decode_bmp(skiatest::Reporter* r, const char* path);
```
