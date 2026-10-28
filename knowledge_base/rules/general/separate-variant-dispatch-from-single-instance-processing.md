---
name: skia/separate-variant-dispatch-from-single-instance-processing
description: When emitting specialized or multi-variant elements, encapsulate dispatch loops inside element handlers and separate iteration from single-instance processing instead of self-recursing with sentinels
rubric: design
severity: should_fix
---

**Rule.** When processing constructs with multiple specializations or variants
in code generators or traversers, encapsulate specialization lookup and dispatch
iteration inside dedicated element handlers rather than exposing iteration in
top-level driver loops. Within the handler, separate collection iteration from
single-instance processing into distinct functions; avoid using a single
self-recursive function that inspects sentinel indices (such as kUnspecialized
or kAll) to loop and call itself.

**Why.** Exposing specialization iteration in top-level driver loops clutters
traversal logic with element-specific dispatch details. Furthermore,
multiplexing multi-variant dispatch and single-item generation into a single
self-recursive function with sentinel parameters obscures function termination,
risks runaway recursion, causes redundant map lookups per iteration, and leaks
internal sentinel parameters into caller APIs.

## Examples

Bad:

```cpp
// Bad: Exposing specialization lookup and iteration in top-level driver loop:
for (const ProgramElement* e : program.elements()) {
    if (e->is<FunctionDefinition>()) {
        if (auto* specs = fSpecializationMap.find(&e->as<FunctionDefinition>().declaration())) {
            for (int i = 0; i < specs->size(); i++) {
                this->writeFunction(e->as<FunctionDefinition>(), i, out);
            }
        }
    }
}

// Bad: Or multiplexing iteration and emission via self-recursion and sentinel index:
void CodeGenerator::writeFunction(const FunctionDefinition& f, Index specIndex, OutputStream& out) {
    if (auto* specs = fSpecializationMap.find(&f.declaration())) {
        if (specIndex == kUnspecialized) {
            for (int i = 0; i < specs->size(); i++) {
                this->writeFunction(f, i, out);
            }
            return;
        }
    }
    // Emit single function instantiation...
}
```

Good:

```cpp
// Good: Driver loop delegates directly to the element handler:
for (const ProgramElement* e : program.elements()) {
    if (e->is<FunctionDefinition>()) {
        this->writeFunction(e->as<FunctionDefinition>(), out);
    }
}

// Good: Dispatch entrypoint handles iteration and delegates to a dedicated single-instance helper:
void CodeGenerator::writeFunction(const FunctionDefinition& f, OutputStream& out) {
    if (auto* specs = fSpecializationMap.find(&f.declaration())) {
        for (int i = 0; i < specs->size(); i++) {
            this->writeFunctionInstantiation(f, i, out);
        }
    } else {
        this->writeFunctionInstantiation(f, kUnspecialized, out);
    }
}

void CodeGenerator::writeFunctionInstantiation(const FunctionDefinition& f, Index specIndex, OutputStream& out) {
    // Emit single function instantiation...
}
```
