---
name: skia/gpu/forward-gpu-stats-to-finish-callbacks
description: When collecting GPU statistics for completion or finish callbacks, forward the gathered stats via setStats across all polling and teardown paths
rubric: correctness
severity: should_fix
---

**Rule.** When collecting GPU statistics (such as elapsed time from timer
queries) for finish or completion callbacks, ensure gathered stats are forwarded
to the callback wrapper via setStats() across all execution paths—including
drain or teardown routines like callAll() as well as regular polling
checks—before the callback is popped or triggered.

**Why.** Querying GPU statistics into a temporary local struct without assigning
them to the callback via setStats() causes the gathered measurements to be
discarded when the queue entry is popped. Callbacks dispatched during teardown
or flush drain then receive default-initialized empty statistics.

## Examples

Bad:

```cpp
skgpu::GpuStats stats;
if (doDelete && finishCallback.fSync) {
    fGpu->deleteSync(finishCallback.fSync);
    if (finishCallback.fTimerQuery) {
        stats.elapsedTime = fGpu->getTimerQueryResult(finishCallback.fTimerQuery);
    }
}
fCallbacks.pop_front();
```

Good:

```cpp
skgpu::GpuStats stats;
if (doDelete && finishCallback.fSync) {
    fGpu->deleteSync(finishCallback.fSync);
    if (finishCallback.fTimerQuery) {
        stats.elapsedTime = fGpu->getTimerQueryResult(finishCallback.fTimerQuery);
        if (finishCallback.fCallback.receivesGpuStats()) {
            finishCallback.fCallback.setStats(stats);
        }
    }
}
fCallbacks.pop_front();
```
