---
name: skia/avoid-leading-pipe-in-ts-union-types
description: 'In TypeScript declarations and type annotations, omit leading pipe characters from single-line inline union types instead of writing param?: | Type | null'
rubric: readability
severity: nit
---

**Rule.** In TypeScript type annotations, property signatures, and single-line
type declarations, do not include a leading pipe (|) before the first member of
an inline union type. Write 'param?: Type | null' rather than 'param?: | Type |
null'.

**Why.** While TypeScript's grammar permits a leading pipe in union types to
format multi-line type declarations cleanly, adding a leading pipe to
single-line inline type signatures is extraneous, inconsistent with TypeScript
conventions, and clutters method and property signatures.

## Examples

Bad:

```cpp
words?: | InputWords | null,
lineBreaks?: | InputLineBreaks | null
```

Good:

```cpp
words?: InputWords | null,
lineBreaks?: InputLineBreaks | null
```
