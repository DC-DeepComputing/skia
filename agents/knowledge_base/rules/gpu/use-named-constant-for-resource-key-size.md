---
name: skia/gpu/use-named-constant-for-resource-key-size
description: When constructing or unpacking GPU resource and unique keys, define static constexpr constants for key data size and field indices instead of magic numbers
rubric: readability
severity: should_fix
---

**Rule.** When initializing a ResourceKey::Builder,
GraphiteResourceKey::Builder, or UniqueKey::Builder, define an explicit static
constexpr uint16_t constant (such as kKeySize or kNum32DataCnt) for the 32-bit
word count rather than passing a raw integer literal. Additionally, when packing
or extracting multi-field keys across builder slots or raw key data buffers,
define shared named static constexpr index constants (such as k<Field>Index) for
each field rather than using hardcoded numeric literals (e.g., rawKeyData[2]) or
sequential increments (builder[idx++]), keeping creation and extraction
synchronized.

**Why.** Passing raw numeric literals obscures the allocated payload size and
invites sizing discrepancies between allocation and indexing. Similarly, using
hardcoded numeric indices or sequential increments (idx++) when packing and
unpacking key buffers obscures field meaning and risks silent misalignment or
out-of-bounds reads when key layouts evolve. Explicit static constexpr constants
for size and field indices document the payload layout and keep key creation,
capacity, and extraction synchronized.

## Examples

Bad:

```cpp
// Sizing and packing with magic numeric literals and sequential increments:
GraphiteResourceKey::Builder builder(&key, kType, 4);
builder[idx++] = static_cast<uint32_t>(pipelineDesc.renderStepID());
builder[idx++] = pipelineDesc.paintParamsID().asUInt();
builder[idx++] = this->getRenderPassDescKeyForPipeline(renderPassDesc);

// Unpacking key fields with hardcoded numeric indices:
auto stepID = static_cast<RenderStep::RenderStepID>(rawKeyData[0]);
const uint32_t rpDescBits = rawKeyData[2];
```

Good:

```cpp
static constexpr uint16_t kPipelineKeyData32Count = 4;
static constexpr int kPipelineKeyRenderStepIDIndex = 0;
static constexpr int kPipelineKeyPaintParamsIDIndex = 1;
static constexpr int kPipelineKeyRenderPassDescIndex = 2;

// Sizing and packing using explicit named constants:
GraphiteResourceKey::Builder builder(&key, kType, kPipelineKeyData32Count);
builder[kPipelineKeyRenderStepIDIndex] = static_cast<uint32_t>(pipelineDesc.renderStepID());
builder[kPipelineKeyPaintParamsIDIndex] = pipelineDesc.paintParamsID().asUInt();
builder[kPipelineKeyRenderPassDescIndex] = this->getRenderPassDescKeyForPipeline(renderPassDesc);

// Unpacking key fields using shared named index constants:
auto stepID = static_cast<RenderStep::RenderStepID>(rawKeyData[kPipelineKeyRenderStepIDIndex]);
const uint32_t rpDescBits = rawKeyData[kPipelineKeyRenderPassDescIndex];
```
