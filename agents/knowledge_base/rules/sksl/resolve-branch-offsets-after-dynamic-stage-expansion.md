---
name: skia/sksl/resolve-branch-offsets-after-dynamic-stage-expansion
description: When lowering pipeline instructions with dynamic child stages or subprograms, resolve relative branch offsets after appending all expanded child stages
rubric: correctness
severity: should_fix
---

**Rule.** When translating or lowering control flow in instruction or pipeline
builders where operations can dynamically expand (such as invoking child
effects, subprograms, or callbacks), do not precompute branch target offsets
before stage expansion. Record branch and label positions during stage
generation and resolve relative jump offsets only after all dynamic child stages
have been appended to the pipeline.

**Why.** Relative branch jump offsets depend on the exact number of intervening
pipeline stages between the branch instruction and its target label.
Precomputing offsets before expanding dynamic child stages causes subsequent
inserted stages to shift relative positions, resulting in branches jumping into
the middle of unrelated stages or out of bounds.

## Examples

Bad:

```cpp
// Precomputing branch offsets before dynamic child stages are expanded
for (int i = 0; i < fNumBranches; ++i) {
    int branchFrom = branchContexts[i]->offset;
    int branchTo = labelOffsets[branchGoesToLabel[i]];
    branchContexts[i]->offset = branchTo - branchFrom;
}

// Later, dynamic child effects expand and inject extra stages
for (const Stage& stage : stages) {
    if (stage.op == ProgramOp::invoke_shader) {
        callbacks->appendShader(...);  // Shifts relative stage distances!
    }
}
```

Good:

```cpp
// Append all stages and expand dynamic child stages first
for (const Stage& stage : stages) {
    switch (stage.op) {
        case ProgramOp::invoke_shader:
            callbacks->appendShader(...);
            break;
        case ProgramOp::label:
            labelOffsets[stage.labelID] = pipeline->getNumStages();
            break;
        case ProgramOp::jump:
            stage.branchCtx->offset = pipeline->getNumStages();
            branchContexts.push_back(stage.branchCtx);
            break;
    }
}

// Resolve relative branch offsets after all dynamic stages are appended
for (int i = 0; i < branchContexts.size(); ++i) {
    int branchFrom = branchContexts[i]->offset;
    int branchTo = labelOffsets[branchGoesToLabel[i]];
    branchContexts[i]->offset = branchTo - branchFrom;
}
```
