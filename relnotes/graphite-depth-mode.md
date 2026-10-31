Deprecate `ContextOptions::fAvoidDepthMode` in favor of setting the `DepthUsageMode` enum instead.
Initially, this only exposes `kAlways` (equivalent to `fAvoidDepthMode == false`) and `kNever`
(equivalent to `fAvoidDepthMode == true`). `DepthUsageMode` also has a new mode: `kAvoidForInternal`
that will prevent internally allocated surfaces for simple operations (like copies, mipmap
generation or image filtering) from relying on the depth buffer.
