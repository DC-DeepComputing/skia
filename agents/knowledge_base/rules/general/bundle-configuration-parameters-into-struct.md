---
name: skia/bundle-configuration-parameters-into-struct
description: When constructors or functions accept multiple related configuration, sizing, or tuning parameters, bundle them into a dedicated options struct instead of passing loose primitive arguments
rubric: design
severity: should_fix
---

**Rule.** Bundle multiple related configuration, sizing, or tuning parameters
into a dedicated struct (such as an inner struct Options or dedicated settings
struct) defined in the consuming class's header, rather than passing a long list
of primitive arguments to a constructor or function. Group correlated fields
together and provide sensible default in-class member initializers where
appropriate.

**Why.** Passing numerous loose primitive parameters (such as multiple separate
buffer sizes or limits) creates unwieldy, fragile signatures that require
widespread call-site churn whenever options are added, removed, or changed.
Bundling parameters into a dedicated struct establishes an organized, cohesive
unit of configuration, enables named initialization and default values, and
allows signatures to remain stable over time.

## Examples

Bad:

```cpp
DrawBufferManager(ResourceProvider* resourceProvider,
                  const Caps* caps,
                  UploadBufferManager* uploadManager,
                  uint32_t vertexBufferMinSize,
                  uint32_t vertexBufferMaxSize,
                  uint32_t indexBufferSize,
                  uint32_t uniformBufferSize,
                  uint32_t storageBufferMinSize,
                  uint32_t storageBufferMaxSize);
```

Good:

```cpp
struct Options {
    uint32_t fVertexBufferMinSize  = 16 << 10;
    uint32_t fVertexBufferMaxSize  = 1 << 20;
    uint32_t fIndexBufferSize      = 2 << 10;
    uint32_t fUniformBufferSize    = 2 << 10;
    uint32_t fStorageBufferMinSize = 2 << 10;
    uint32_t fStorageBufferMaxSize = 1 << 20;
};

DrawBufferManager(ResourceProvider* resourceProvider,
                  const Caps* caps,
                  UploadBufferManager* uploadManager,
                  Options options = {});
```
