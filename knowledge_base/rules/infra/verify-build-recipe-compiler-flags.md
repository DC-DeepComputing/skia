---
name: skia/infra/verify-build-recipe-compiler-flags
description: When adding or adapting bot build recipe configs, verify that each compiler flag or preprocessor define is required rather than copying unneeded settings
rubric: readability
severity: should_fix
---

**Rule.** When introducing new build targets or recipe configurations based on
existing targets, verify that each compiler flag and preprocessor define (such
as extra_cflags) is actively required for the new target. Do not copy legacy
defines (such as -DMESA_EGL_NO_X11_HEADERS or target-specific workarounds)
across architectures or configurations without confirming their necessity.

**Why.** Copying flags and preprocessor defines between build configs or
architectures introduces obsolete and redundant technical debt. Ensuring only
necessary flags are carried prevents carrying dead configuration settings and
masking build differences across platforms.

## Examples

Bad:

```cpp
args['extra_cflags'] = [
    '--target=aarch64-linux-gnueabihf',
    '--sysroot=%s' % sysroot_dir,
    '-DMESA_EGL_NO_X11_HEADERS',
    '-U_GLIBCXX_DEBUG',
]
```

Good:

```cpp
args['extra_cflags'] = [
    '--target=aarch64-linux-gnueabihf',
    '--sysroot=%s' % sysroot_dir,
    '-U_GLIBCXX_DEBUG',
]
```
