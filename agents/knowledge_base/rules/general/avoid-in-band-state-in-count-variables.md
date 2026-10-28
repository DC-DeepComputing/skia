---
name: skia/avoid-in-band-state-in-count-variables
description: When an explicit state enum or bitmask tracks operational modes, store pure unsigned counts rather than multiplexing state using negative sentinels
rubric: readability
severity: should_fix
---

**Rule.** Do not encode operational modes, states, or dynamic behaviors into
numeric count or size variables using in-band sentinels, sign tricks (such as
negative values), or inverted arithmetic formulas when an explicit state enum or
bitmask is available. Store pure non-negative counts using unsigned integer
types (such as uint32_t or size_t) and determine operational modes directly from
the explicit state flags.

**Why.** Multiplexing mode information into signed count fields obscures
variable invariants and inverts operations (such as using std::min to track an
increasing maximum). Storing pure counts in unsigned fields simplifies bounds
checking and assertions while centralizing mode dispatch in explicit state
enums.

## Examples

Bad:

```cpp
// Sign-based state encoding: == 0 is static, > 0 is fixed instanced, < 0 is dynamic
int fTemplateCount;
...
fDrawer.fTemplateCount = std::min(fDrawer.fTemplateCount, -SkTo<int>(count) - 1);
```

Good:

```cpp
// Mode is explicitly tracked by fRenderState; fTemplateCount stores purely the count.
SkEnumBitMask<RenderStateFlags> fRenderState;
uint32_t fTemplateCount;
...
fDrawer.fTemplateCount = std::max(fDrawer.fTemplateCount, static_cast<uint32_t>(count));
```
