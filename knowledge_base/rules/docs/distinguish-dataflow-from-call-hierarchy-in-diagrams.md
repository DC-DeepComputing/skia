---
name: skia/docs/distinguish-dataflow-from-call-hierarchy-in-diagrams
description: When creating or updating architecture and design diagrams in documentation, distinguish data flow from call hierarchy in titles, legends, and arrow semantics
rubric: readability
severity: nit
---

**Rule.** Do not conflate call or dependency hierarchy with data flow when
defining arrow semantics in architecture and design diagrams. When arrows depict
data movement (such as inputs passed into processing APIs), explicitly title the
diagram as a data flow diagram and document arrow conventions in the legend to
avoid misinterpreting inputs as callers.

**Why.** In architecture diagrams, arrows conventionally indicate call hierarchy
or control flow ('A calls B'). Using the same arrow style to represent data flow
(such as 'SkPath -> SkCanvas') suggests that passive input objects initiate
calls into processors, confusing readers unless the diagram taxonomy and arrow
semantics are explicitly stated in the title and legend.

## Examples

Bad:

```cpp
// Skia CPU Backend Architecture Diagram
// Line Styles:
//   - Solid: Calls / Uses (A function is called, or an object is used as a parameter)
SkPath -> SkCanvas;
```

Good:

```cpp
// Skia CPU Backend Dataflow Diagram
// Line Styles:
//   - Solid: Data Flow (Objects passed in as parameters in function calls)
SkPath -> SkCanvas;
```
