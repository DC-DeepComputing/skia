---
name: skia/prefix-test-only-methods-with-testing-only
description: Avoid adding test-only hooks to production classes when tests can instantiate subcomponents directly; prefix necessary test-only methods with testingOnly_
rubric: design
severity: should_fix
---

**Rule.** Do not add test-only methods, accessors, or #if
defined(GPU_TEST_UTILS) hooks to higher-level production classes when unit tests
can instantiate the target subcomponent directly using local or stack-allocated
dependencies. When test-only helper methods, accessors, or hooks are genuinely
required on production classes, prefix their names with testingOnly_ (giving
them descriptive names) rather than standard method names or Chromium-style
ForTesting suffixes.

**Why.** Exposing test hooks or factory methods on production classes adds
unnecessary maintenance overhead, leaks internal details, and risks accidental
invocation from production code. Direct subcomponent instantiation in tests
simplifies fixtures without polluting production types, while the testingOnly_
prefix clearly alerts callers and reviewers that any unavoidable hook violates
normal encapsulation.

## Examples

Bad:

```cpp
// Bad: adding unnecessary test factory/hook to production class
#if defined(GPU_TEST_UTILS)
std::unique_ptr<DrawWriter> DrawPass::getDrawWriter(DrawBufferManager* bufferMgr) {
    return std::make_unique<DrawWriter>(&fCommandList, bufferMgr);
}
// Bad: test hooks using standard method names without testingOnly_ prefix
void setInitialStaticVertexInfo(skia_private::TArray<uint32_t>);
int32_t getBackingRefCnt() const;
#endif
```

Good:

```cpp
// Good: test constructs subcomponent directly with stack dependencies
DrawPassCommands::List commandList;
auto dw = std::make_unique<DrawWriter>(&commandList, bufferMgr);

// Good: unavoidable test hooks prefixed with testingOnly_
#if defined(GPU_TEST_UTILS)
void testingOnly_SetStaticVertexInfo(skia_private::TArray<StaticVertexCopyRanges>,
                                     sk_sp<Buffer>);
int32_t testingOnly_getBackingRefCnt() const;
#endif
```
