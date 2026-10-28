---
name: skia/distinguish-ci-jobs-from-bots-in-commit-messages
description: When describing CI build or test configurations in commit messages, refer to them as jobs or compile/test jobs rather than bots or buildbots
rubric: readability
severity: nit
---

**Rule.** In commit messages, do not use the term 'bot' or 'buildbot' when
referring to CI build or test configurations; use precise terms such as 'job',
'compile job', or 'test job' instead. Reserve 'bot' specifically for physical or
virtual Swarming worker machines.

**Why.** In Skia infrastructure, a 'bot' is the physical or virtual Swarming
machine that executes tasks, whereas a 'job' is a directed acyclic graph (DAG)
of tasks scheduled by the Task Scheduler. Conflating CI build or test
configurations with bots creates confusion between machine worker nodes and
scheduled execution DAGs.

## Examples

Bad:

```cpp
Add two WASM buildbots

Build changes to enable these bots to succeed:
```

Good:

```cpp
Add two WASM compile jobs

Build changes to enable these jobs to succeed:
```
