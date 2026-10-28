---
name: skia/gpu/pass-varying-buffer-alignment-as-parameter
description: When managing shared GPU buffers with varying element strides, maintain baseline minimum alignments and pass per-reservation alignment to suballocators to compute offsets via LCM
rubric: correctness
severity: should_fix
---

**Rule.** When managing shared GPU buffers with varying alignment requirements
(such as vertex or instance strides), maintain baseline hardware constraints as
constant minimum alignments (e.g., minimum_alignment(...), const uint32_t
fMinimumAlignment). Pass explicit element strides or alignment requirements per
reservation to allocator methods (such as prepareStaticData or
prepareMappedBindBuffer) rather than mutating shared buffer state or defaulting
to generic alignment, and compute offsets using the least common multiple (LCM)
of the requested alignment and the buffer's minimum alignment.

**Why.** Naming baseline constraints as starting or mutable alignments creates
confusion about whether the value changes dynamically. Omitting explicit element
strides causes suballocators to default to generic buffer alignment, creating
offset mismatches that break contiguous buffer tracking, while mutating shared
buffer state corrupts alignment for earlier queued copy ranges. Aligning per
reservation via LCM ensures hardware constraints and varying element strides are
both satisfied safely and reentrantly.

## Examples

Bad:

```cpp
// Bad: Naming baseline alignment as starting alignment, and mutating buffer state
uint32_t starting_alignment(BufferType type, bool useTransferBuffers, const Caps* caps);
struct BufferInfo { uint32_t fStartAlignment; };

// Bad: Defaulting to generic 4-byte buffer alignment instead of element stride
auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, "VertexBuffer", requiredBytes);

// Bad: Mutating shared buffer alignment state for per-reservation requirements
fVertexBufferState.fAlignment = stride > fRequiredTransferAlignment ? stride : fRequiredTransferAlignment;
void* data = this->prepareStaticData(&fVertexBufferState, size, binding);

// Bad: Overwriting shared alignment when queuing deferred copy ranges
fVertexBufferInfo.fAlignment = stride;
info->fData.push_back({transferBindInfo, target});
```

Good:

```cpp
// Good: Name baseline constraint as minimum alignment, and keep it constant
uint32_t minimum_alignment(BufferType type, bool useTransferBuffers, const Caps* caps);
struct BufferInfo { const uint32_t fMinimumAlignment; };

// Good: Explicitly pass per-reservation element stride to suballocation calls
auto [ptr, bindInfo] = this->prepareMappedBindBuffer(&info, "VertexBuffer", requiredBytes, stride);

// Good: Pass per-reservation alignment and align via LCM
const uint32_t alignment = BufferAligner::LcmAlignment(state->fMinimumAlignment, requiredAlignment);
void* data = this->prepareStaticData(&fVertexBufferState, size, requiredAlignment, binding);

// Good: Store required alignment per reservation struct and align each offset via LCM
info->fData.push_back({transferBindInfo, target, requiredAlignment});
for (const CopyRange& data : fData) {
    offset = align_to_req_min_lcm(offset, data.fRequiredAlignment, fMinimumAlignment);
    data.fTarget->fOffset = offset;
    offset += data.fSource.fSize;
}
```
