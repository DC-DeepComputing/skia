---
name: skia/modules/hoist-scene-graph-revalidation-outside-node-loops
description: When mutating Skottie slot or property values, trigger SceneGraphRevalidator::revalidate once after updating nodes or adapters, outside any iteration loop
rubric: correctness
severity: should_fix
---

**Rule.** When mutating Skottie property handles or slot values (such as in
SlotManager::set*Slot or PropertyHandle::set), always trigger scene-graph
revalidation via fRevalidator->revalidate() or sksg::Scene::revalidate() so
updates propagate to subsequent draws. When updating multiple node bindings or
adapter targets in a loop, mutate or invalidate individual nodes (or invoke
adapter syncs) inside the loop and hoist the revalidation call once after the
loop completes.

**Why.** Modifying slot or property values updates underlying node or adapter
state but does not automatically propagate through the scene graph; invoking
SceneGraphRevalidator::revalidate() forces the root render node to revalidate so
updates take effect on subsequent draws. Calling revalidation inside iteration
loops causes redundant, expensive full-DAG scene graph traversals for every
mutated element.

## Examples

Bad:

```cpp
// Missing revalidation after mutating slot values:
bool SlotManager::setScalarSlot(const SlotID& slotID, float s) {
    if (const auto valueGroup = fScalarMap.find(slotID)) {
        for (auto& sPair : *valueGroup) {
            *(sPair.value) = s;
            sPair.adapter->onSync();
        }
        return true;
    }
    return false;
}

// Revalidating inside the loop on every iteration:
for (auto& cPair : *valueGroup) {
    *(cPair.value) = c;
    cPair.node->invalidate();
    fRevalidator->revalidate();
}
```

Good:

```cpp
// Mutate nodes or sync adapters in loop, then revalidate once afterward:
bool SlotManager::setScalarSlot(const SlotID& slotID, float s) {
    if (const auto valueGroup = fScalarMap.find(slotID)) {
        for (auto& sPair : *valueGroup) {
            *(sPair.value) = s;
            sPair.adapter->onSync();
        }
        fRevalidator->revalidate();
        return true;
    }
    return false;
}

for (auto& cPair : *valueGroup) {
    *(cPair.value) = c;
    cPair.node->invalidate();
}
fRevalidator->revalidate();
```
