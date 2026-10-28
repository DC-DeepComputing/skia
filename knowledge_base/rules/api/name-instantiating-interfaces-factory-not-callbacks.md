---
name: skia/api/name-instantiating-interfaces-factory-not-callbacks
description: When defining customization interfaces or classes whose primary purpose is to instantiate objects, name them Factory rather than Callbacks
rubric: readability
severity: should_fix
---

**Rule.** Name customization interfaces, abstract classes, and injector methods
whose primary responsibility is instantiating and returning objects (e.g.,
providing make...() virtual methods) 'Factory' rather than generic 'Callbacks'.
Reserve 'Callback' or 'Handler' for event notifications and lifecycle hooks that
react to operations rather than producing objects on demand.

**Why.** The term 'Callback' denotes an event-handling or notification routine.
When an interface's primary responsibility is constructing and returning objects
on demand, naming it 'Factory' matches standard design pattern semantics,
clarifies its role in dependency injection, and prevents confusing object
creation with event listening.

## Examples

Bad:

```cpp
// Misleading: interface instantiates objects but is labeled Callbacks
class Callbacks : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) const = 0;
};
Builder& setTextShapingCallbacks(sk_sp<Callbacks> callbacks);
```

Good:

```cpp
// Clear: Factory reflects object instantiation and creation responsibility
class Factory : public SkRefCnt {
public:
    virtual std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) const = 0;
};
Builder& setTextShapingFactory(sk_sp<Factory> factory);
```
