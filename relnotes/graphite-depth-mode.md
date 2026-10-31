Deprecate `ContextOptions::fAvoidDepthMode` in favor of setting the `DepthUsageMode` enum instead.
Initially, this only exposes `kAlways` (equivalent to `fAvoidDepthMode == false`) and `kNever`
(equivalent to `fAvoidDepthMode == true`). In the future, additional modes to more carefully
control when depth buffers are used will be added.
