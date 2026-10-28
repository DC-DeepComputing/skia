---
name: skia/prefer-standard-verbs-for-task-initiation
description: When naming methods that initiate, schedule, or enqueue tasks or operations, use standard verbs like start, begin, or post instead of colloquial phrasal verbs like kickOff
rubric: readability
severity: nit
---

**Rule.** Prefer conventional action verbs (such as start, begin, post,
dispatch, or enqueue) over colloquial phrasal verbs (such as kickOff, spinUp, or
fireOff) when naming methods that initiate, schedule, or dispatch tasks or
operations.

**Why.** Colloquial phrasal verbs like 'kickOff' or 'spinUp' are informal,
non-standard in C++ API design, and inconsistent with established naming
conventions across the codebase. Using standard action verbs provides clear,
predictable, and uniform interfaces that match conventional task dispatch
terminology.

## Examples

Bad:

```cpp
// Bad: Uses colloquial phrasal verb 'kickOff' to initiate task execution
void kickOffPipelineCreationTask(ResourceProvider*,
                                 sk_sp<const RuntimeEffectDictionary>,
                                 const GraphicsPipelineHandle&);
```

Good:

```cpp
// Good: Uses standard action verb 'start' for task initiation
void startPipelineCreationTask(ResourceProvider*,
                               sk_sp<const RuntimeEffectDictionary>,
                               const GraphicsPipelineHandle&);
```
