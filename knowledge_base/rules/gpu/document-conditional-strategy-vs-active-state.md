---
name: skia/gpu/document-conditional-strategy-vs-active-state
description: When declaring member variables or accessors storing conditional execution strategies or fallback policies, document whether they represent configured handling rules rather than active recorded state
rubric: readability
severity: nit
---

**Rule.** When declaring member variables or accessor methods on GPU context,
recorder, or pass classes that store conditional strategies or fallback policies
(such as DstReadStrategy), document whether they represent a configured handling
rule to be used if an operation requires it, rather than reflecting the active
state of recorded operations. Clarify in comments that the field does not
indicate whether current draw lists or passes actually need or execute that
strategy.

**Why.** Member variables and getters on context or manager objects that hold
execution strategies or fallback policies can easily be misinterpreted as
reflecting whether currently recorded operations or passes actually require that
feature. Explicit comments prevent callers from conflating configured fallback
mechanisms with active recorded draw state.

## Examples

Bad:

```cpp
class DrawContext {
public:
    DstReadStrategy dstReadStrategy() const { return fDstReadStrategy; }

private:
    DstReadStrategy fDstReadStrategy;
};
```

Good:

```cpp
class DrawContext {
public:
    // Returns the dst read strategy to use if a paint requires a dst read.
    DstReadStrategy dstReadStrategy() const { return fDstReadStrategy; }

private:
    // Does not reflect whether a dst read is needed by recorded draw lists;
    // specifies the strategy to use should an encountered paint require it.
    DstReadStrategy fDstReadStrategy;
};
```
