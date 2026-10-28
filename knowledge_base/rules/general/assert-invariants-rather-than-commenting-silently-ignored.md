---
name: skia/assert-invariants-rather-than-commenting-silently-ignored
description: When unhandled flags or parameters cannot occur due to invariants, assert with SkASSERT and document the guarantee rather than commenting that they are silently ignored
rubric: readability
severity: should_fix
---

**Rule.** Do not describe unhandled flags, modifiers, or parameters as 'silently
ignored' when language, format, or subsystem invariants guarantee they cannot be
present or are handled implicitly. Document the invariant explaining why the
values cannot occur or do not need explicit handling, and enforce the invariant
with SkASSERT so invalid inputs fail fast in debug builds.

**Why.** Commenting that unhandled flags, parameters, or modifiers are 'silently
ignored' suggests that valid inputs or features might be erroneously dropped or
unhandled, confusing reviewers and maintainers. Documenting the invariant and
verifying it with SkASSERT clarifies intent, prevents misunderstanding, and
ensures that unexpected values fail fast rather than continuing silently.

## Examples

Bad:

```cpp
// We silently ignore modifiers for anything that is a built-in
this->writeBuiltinIODecl(type, name, *builtin, delimiter);
return;
```

Good:

```cpp
// Builtin IO parameters should only have in/out modifiers, which are then implicit in
// the generated WGSL, hence why writeBuiltinIODecl does not need them passed in.
SkASSERT(!(modifiers & ~(ModifierFlag::kIn | ModifierFlag::kOut)));
this->writeBuiltinIODecl(type, name, *builtin, delimiter);
return;
```
