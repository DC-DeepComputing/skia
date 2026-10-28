---
name: skia/avoid-redundant-expect-error-in-ts-tests
description: In TypeScript type-definition tests using @ts-expect-error to assert compile failures, omit redundant legacy // $ExpectError inline comments on the same statement
rubric: readability
severity: nit
---

**Rule.** In TypeScript type-definition tests, do not append legacy dtslint `//
$ExpectError` inline comments when `// @ts-expect-error` is already specified on
the preceding line. Rely on the standard `// @ts-expect-error` directive to
assert expected type-checking errors.

**Why.** TypeScript natively supports `// @ts-expect-error` to suppress and
assert compile-time type errors on subsequent statements. Duplicating the
assertion with a legacy dtslint `// $ExpectError` comment adds visual clutter
and redundant assertion directives without providing additional type safety.

## Examples

Bad:

```cpp
// @ts-expect-error
paint.setShader(colorFilter); // $ExpectError
```

Good:

```cpp
// @ts-expect-error
paint.setShader(colorFilter);
```
