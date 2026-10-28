---
name: skia/api/document-non-standard-lifecycle-behavior
description: When APIs or parameters alter object lifecycle behavior—such as intentionally leaking instances on exit or suppressing destruction—document the lifecycle impact and rationale
rubric: readability
severity: should_fix
---

**Rule.** When an API method, parameter, or configuration flag alters standard
object lifecycle or ownership behavior—such as opting into intentionally leaking
a singleton on process exit for shutdown performance or suppressing destructor
invocation—document the lifecycle semantics and rationale in header doc
comments. Clarify that the object is deliberately not destroyed and explain why
this behavior is safe and beneficial so callers do not treat it as an unintended
leak or defect.

**Why.** Intentionally bypassing object destruction or leaking allocations on
process exit appears as a defect or resource leak to maintainers and API
consumers unless explicitly documented. Documenting the lifecycle contract and
its rationale (such as preventing shutdown race conditions or reducing process
teardown overhead) clarifies caller responsibilities and prevents unnecessary
refactoring.

## Examples

Bad:

```cpp
/**
 * Sets the event tracer.
 */
static bool SetInstance(SkEventTracer*, bool leak_tracer = false);
```

Good:

```cpp
/**
 * Sets the event tracer.
 *
 * Not deleting the tracer on process exit avoids teardown race conditions and
 * improves shutdown performance since the OS reclaims the entire heap on exit.
 *
 * @param leak_tracer When true, intentionally leaks the tracer on process exit
 *                    without running its destructor.
 */
static bool SetInstance(SkEventTracer*, bool leak_tracer = false);
```
