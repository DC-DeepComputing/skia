---
name: skia/gpu/separate-simulated-errors-from-genuine-failures
description: When implementing simulated error injection or test fault branches, keep simulation checks separate from genuine failure paths to emit explicit simulation diagnostics
rubric: readability
severity: nit
---

**Rule.** When supporting simulated error injection or test fault injection,
keep simulated error checks in distinct branches from genuine runtime failure
paths. Emit explicit simulation diagnostic messages rather than sharing or
conflating genuine failure logs.

**Why.** Coupling simulated error conditions with real runtime failure checks
causes artificial test failures to log misleading error messages (such as
reporting genuine command buffer or pipeline compilation failures). Branching
separately ensures logs clearly distinguish intentional test simulations from
real system failures.

## Examples

Bad:

```cpp
if (!info.fRecording->priv().addCommands(...) ||
    info.fSimulatedStatus != InsertStatus::kSuccess) {
    if (validPipelines && info.fSimulatedStatus != InsertStatus::kAsyncShaderCompilesFailed) {
        return fail(InsertStatus::kAddCommandsFailed,
                    "Adding Recording commands to the CommandBuffer has failed");
    } else {
        return fail(InsertStatus::kAsyncShaderCompilesFailed,
                    "Async pipeline compiles failed, unable to add Recording commands");
    }
}
```

Good:

```cpp
if (!info.fRecording->priv().addCommands(...)) {
    if (validPipelines) {
        return fail(InsertStatus::kAddCommandsFailed,
                    "Adding Recording commands to the CommandBuffer has failed");
    } else {
        return fail(InsertStatus::kAsyncShaderCompilesFailed,
                    "Async pipeline compiles failed, unable to add Recording commands");
    }
} else if (info.fSimulatedStatus == InsertStatus::kAddCommandsFailed) {
    return fail(InsertStatus::kAddCommandsFailed,
                "Simulating 'kAddCommandsFailed' failure");
} else if (info.fSimulatedStatus == InsertStatus::kAsyncShaderCompilesFailed) {
    return fail(InsertStatus::kAsyncShaderCompilesFailed,
                "Simulating 'kAsyncShaderCompilesFailed' failure");
}
```
