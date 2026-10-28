---
name: skia/api/query-backend-capabilities-via-caps
description: When querying backend capabilities, define checks on Caps rather than Context classes, and encapsulate capability predicates inside operations rather than exposing them to clients
rubric: design
severity: should_fix
---

**Rule.** Expose static backend capabilities, limits, and feature support on
Caps (pushed up to base Caps if common across backends) rather than adding
capability query methods or hardcoding ad-hoc backend API checks on Context,
SharedContext, or proxy classes. High-level classes should query generic Caps
methods via caps() rather than inspecting BackendApi directly. Furthermore,
avoid exposing backend capability queries or change-detection predicates in
public client APIs when operations can encapsulate the condition internally;
provide self-contained methods (such as syncPipelineData()) that perform
necessary checks internally.

**Why.** Caps subclasses are the authoritative system for static hardware limits
and API feature support established during initialization. Defining capability
queries directly in context or proxy classes violates layering and conflates
static backend support with dynamic runtime state. Moreover, exposing internal
capability queries and change-detection predicates to clients leaks backend
implementation details and forces callers to coordinate platform-specific
heuristics that should be encapsulated internally.

## Examples

Bad:

```cpp
// In SharedContext.h or Context.h (adding capability/detection queries to context):
virtual bool canDetectNewPipelineCacheData() const { return false; }
bool hasNewPipelineCacheData() const;

// In GrContextThreadSafeProxy.cpp (hardcoding ad-hoc backend checks):
bool avoidRenderBuffersOnGL = GrBackendApi::kOpenGL == backendFormat.backend() &&
                              (fCaps->avoidStencilBuffers() ||
                               fCaps->supportsProtectedContent());
if (avoidRenderBuffersOnGL && sampleCnt > 1) {
    return {};
}
```

Good:

```cpp
// In Caps.h (exposing static capability query on Caps):
bool canDetectNewPipelineCacheData() const { return fCanDetectNewPipelineCacheData; }

// In Context.cpp (encapsulating checks internally rather than exposing query methods to clients):
void Context::syncPipelineData(size_t maxSize) {
    if (fSharedContext->caps()->canDetectNewPipelineCacheData()) {
        fSharedContext->syncPipelineData(fPersistentPipelineStorage, maxSize);
    }
}

// In GrContextThreadSafeProxy.cpp (querying generic Caps methods):
if (!fCaps->isFormatAsColorTypeRenderable(grColorType, backendFormat, sampleCnt)) {
    return {};
}
```
