Deprecate `ContextOptions::fAvoidDepthMode` in favor of setting the `DepthUsageMode` enum instead.
Initially, this only exposes `kAlways` (equivalent to `fAvoidDepthMode == false`) and `kNever`
(equivalent to `fAvoidDepthMode == true`). `DepthUsageMode` also has two new modes:

  1. `kAvoidForInternal` that will prevent internally allocated surfaces for simple operations (like
copies, mipmap generation or image filtering) from relying on the depth buffer.
  2. `kOnlyWhenRequired` that extends `kAvoidForInternal` to detect when client-controlled surfaces
(and `saveLayers`) would not benefit from depth-based occlusion culling or clipping, and avoid
using a depth buffer. This can save overhead during rendering and in allocating attachments, but
can also make it harder to predict the render passes required for content, and can lead to more
pipeline compiles.
