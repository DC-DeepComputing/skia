---
name: skia/api/prefer-iterator-interface-over-mutating-container-cursor
description: When designing collection or queue classes, provide iterators with begin() and end() or indexed access instead of embedding mutating cursor methods
rubric: design
severity: should_fix
---

**Rule.** When designing collection, queue, or sequence classes, provide an
iterator interface (begin() and end()) or const index-based access (operator[])
rather than embedding internal cursor state and mutating advance methods (such
as next() or hasMore()). If stateful or single-pass traversal is needed,
encapsulate the cursor in a separate iterator or view type.

**Why.** Embedding traversal state directly in a collection forces iteration to
mutate the container itself, preventing const container usage, concurrent or
repeated traversals, and range-based for loops. Decoupling traversal state into
iterators keeps the container immutable during inspection and aligns with
standard C++ container idioms.

## Examples

Bad:

```cpp
class EventQueue {
public:
    bool hasMoreEvents() const { return fCurrentEvent < fEvents.size(); }
    Event nextEvent() { return fEvents[fCurrentEvent++]; }
private:
    int32_t fCurrentEvent = 0;
    std::vector<Event> fEvents;
};
```

Good:

```cpp
class EventQueue {
public:
    class Iterator {
    public:
        Event operator*() const { return fQueue[fIndex]; }
        Iterator& operator++() { ++fIndex; return *this; }
        bool operator!=(const Iterator& other) const { return fIndex != other.fIndex; }
    private:
        const EventQueue& fQueue;
        size_t fIndex;
    };

    Iterator begin() const { return Iterator{*this, 0}; }
    Iterator end() const { return Iterator{*this, fEvents.size()}; }
    Event operator[](size_t i) const { return fEvents[i]; }
private:
    std::vector<Event> fEvents;
};
```
