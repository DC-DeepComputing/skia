---
name: skia/leak-global-singletons-to-avoid-shutdown-races
description: When managing process-wide singletons accessed by background threads, intentionally leak them with SK_INTENTIONALLY_LEAKED instead of deleting them in atexit handlers
rubric: safety
severity: should_fix
---

**Rule.** Do not delete process-wide global singletons or service handlers in
atexit() handlers when background worker threads may still access them during
application shutdown. Intentionally leak the singleton instead, marking the
pointer with SK_INTENTIONALLY_LEAKED(...) to satisfy leak sanitizers, and
perform any necessary shutdown flushing via an onExit() callback rather than the
destructor.

**Why.** In multithreaded applications, worker threads can remain active and
access global services (such as event tracers or loggers) during process
termination. Deleting singletons in atexit() creates data races and
use-after-free crashes with concurrent threads; leaking instances keeps them
valid until process teardown, while SK_INTENTIONALLY_LEAKED suppresses false
positives in leak sanitizers.

## Examples

Bad:

```cpp
bool SkEventTracer::SetInstance(SkEventTracer* tracer, bool leakTracer) {
    ...
    if (!leakTracer) {
        atexit([]() {
            auto tracer = gUserTracer.exchange(nullptr);
            delete tracer;
        });
    }
    return true;
}
```

Good:

```cpp
bool SkEventTracer::SetInstance(SkEventTracer* tracer) {
    ...
    // Once set, `tracer` remains alive until the process is destroyed.
    SK_INTENTIONALLY_LEAKED(tracer);
    atexit([]() { GetInstance()->onExit(); });
    return true;
}
```
