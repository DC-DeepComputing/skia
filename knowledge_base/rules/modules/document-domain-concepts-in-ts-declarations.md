---
name: skia/modules/document-domain-concepts-in-ts-declarations
description: When adding methods, properties, or feature groups to CanvasKit TypeScript declarations, provide JSDoc comments explaining behavior, return values, tool requirements, and domain concepts
rubric: readability
severity: should_fix
---

**Rule.** In public TypeScript declaration files (such as CanvasKit's
`types/index.d.ts`), provide JSDoc documentation (`/** ... */`) for newly added
methods, functions, and properties describing their behavior, return values, and
intended use cases. Additionally, when exposing new domain concepts or API
groups (such as Lottie slots), document their background origin, required tool
or file versions, and key/parameter semantics rather than leaving bare
signatures or terse section comments.

**Why.** CanvasKit's `index.d.ts` serves as both TypeScript declarations and the
primary public API documentation and editor tooltip reference for npm consumers.
Bare method signatures leave consumers without guidance on return values or
usage rationale, while undocumented domain concepts leave developers unable to
determine required authoring tool versions or valid key formats.

## Examples

Bad:

```cpp
    // SlotManager API
    setColorSlot(key: string, color: InputColor): boolean;

    getText(): string;
```

Good:

```cpp
    // Slots in Lottie were exposed with bodymovin version 5.11.0.
    // Properties tracked under the Essential Graphics window in AE will be "slotted".
    setColorSlot(key: string, color: InputColor): boolean;

    /**
     * Returns the entire Paragraph text (which is useful in case that text
     * was produced as a set of addText calls).
     */
    getText(): string;
```
