---
name: skia/gpu/document-complementary-backend-query-helpers
description: When declaring complementary GPU backend query helpers or Caps capability accessors, document their semantic differences, hardware guarantees, and underlying extension requirements
rubric: readability
severity: nit
---

**Rule.** When declaring closely related or complementary backend query helpers
or Caps capability accessors, document their semantic differences, underlying
hardware mechanisms, and driver requirements in header comments. Explicitly
clarify whether a query represents an automatic driver/hardware capability
(e.g., handling operations implicitly vs. requiring manual fallback passes) or
an inherent hardware behavioral guarantee (such as hardware coherency across
known architectures) versus tracking whether a specific backend API extension is
enabled to govern pipeline creation flags or descriptor setup.

**Why.** Query helpers and capability accessors with similar names confuse
callers regarding which predicate to check at different pipeline stages.
Documenting underlying driver extensions, hardware guarantees, and API flag
requirements clarifies architectural responsibilities, preventing callers from
passing invalid pipeline creation flags, emitting redundant barriers, or making
incorrect assumptions about hardware behavior.

## Examples

Bad:

```cpp
// Undocumented complementary queries leave caller semantics ambiguous.
bool RenderPassDescWillLoadMSAAFromResolve(const RenderPassDesc& renderPassDesc);
bool RenderPassDescWillImplicitlyLoadMSAA(const RenderPassDesc& renderPassDesc);

// Ambiguous capability accessors: does coherency imply the API extension is active?
bool isInputAttachmentReadCoherent() const { return fIsInputAttachmentReadCoherent; }
bool supportsRasterizationOrderColorAttachmentAccess() const {
    return fSupportsRasterizationOrderColorAttachmentAccess;
}
```

Good:

```cpp
// Clarify automatic driver/hardware handling vs manual software pass:
// When multisampling is used, MSAA data may need to be initialized from resolve.
// RenderPassDescWillLoadMSAAFromResolve returns true if this load is needed.
// When VK_EXT_multisampled_render_to_single_sampled is available, the driver loads
// it automatically (RenderPassDescWillImplicitlyLoadMSAA returns true); otherwise,
// Graphite manually executes a fallback draw.
bool RenderPassDescWillLoadMSAAFromResolve(const RenderPassDesc& renderPassDesc);
bool RenderPassDescWillImplicitlyLoadMSAA(const RenderPassDesc& renderPassDesc);

// Clarify inherent hardware guarantee vs backend API extension creation flag:
bool isInputAttachmentReadCoherent() const { return fIsInputAttachmentReadCoherent; }
// isInputAttachmentReadCoherent() is true if VK_EXT_rasterization_order_attachment_access
// is enabled OR if the architecture inherently guarantees coherency. The following
// indicates specifically if the extension is enabled (requiring pipeline creation flags).
bool supportsRasterizationOrderColorAttachmentAccess() const {
    return fSupportsRasterizationOrderColorAttachmentAccess;
}
```
