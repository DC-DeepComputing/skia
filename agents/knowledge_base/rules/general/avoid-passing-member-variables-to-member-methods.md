---
name: skia/avoid-passing-member-variables-to-member-methods
description: When implementing operations requiring class state or configuration, access member variables directly in member functions instead of threading instance fields through parameters or freestanding helpers
rubric: design
severity: should_fix
---

**Rule.** Do not declare non-static member methods with parameters for state or
resources already accessible via member variables of the class, and do not pass
member variables (such as fMember or fMember.get()) as arguments when invoking
member functions. Likewise, convert file-local or anonymous-namespace helpers
into class member methods once they require class configuration or internal
state already owned by the class, rather than retaining a freestanding helper
with a trivial forwarding wrapper. In stateful visitors or generators, update
contextual member state before calling internal helpers so they query instance
fields directly rather than threading parameters.

**Why.** Non-static member methods already have direct access to instance fields
via this. Accepting or passing member variables or tracked contextual state as
function parameters introduces unnecessary arguments, creates dead or duplicate
parameters, clutters call sites, and misleadingly suggests configurability for
methods tightly coupled to instance state. Similarly, keeping freestanding
helpers with trivial forwarding member wrappers introduces redundant parameter
threading and indirection when a direct member method can query class state
directly.

## Examples

Bad:

```cpp
// 1. Passing member variable to a member method:
class SkFontMgr_AndroidNDK : public SkFontMgr {
    sk_sp<SkTypeface_AndroidNDK> make(SkAFont font, bool cacheFontFiles,
                                      const SkFontScanner* scanner) const {
        return scanner->MakeFromStream(...);
    }
    void foo(SkAFont font) {
        this->make(std::move(font), true, fScanner.get());
    }
};

// 2. Threading contextual state into helper instead of setting member field:
void SPIRVCodeGenerator::writeFunctionInstantiation(
        const FunctionDefinition& f,
        Analysis::SpecializationIndex specializationIndex,
        const Analysis::SpecializedParameters* specializedParams,
        OutputStream& out) {
    this->writeFunctionStart(f.declaration(), specializationIndex, specializedParams, out);
    fActiveSpecialization = specializedParams;
    fActiveSpecializationIndex = specializationIndex;
}

// 3. Freestanding helper requiring class context with forwarding wrapper:
std::string to_wgsl_type(const Type& type, const ProgramSettings& settings) { ... }

std::string WGSLCodeGenerator::getTypeName(const Type& t) {
    return to_wgsl_type(t, fProgram.fConfig->fSettings);
}
```

Good:

```cpp
// 1. Access member variable directly in member method:
class SkFontMgr_AndroidNDK : public SkFontMgr {
    sk_sp<SkTypeface_AndroidNDK> make(SkAFont font, bool cacheFontFiles) const {
        return fScanner->MakeFromStream(...);
    }
    void foo(SkAFont font) {
        this->make(std::move(font), true);
    }
};

// 2. Update member state before helper call so helper accesses fields directly:
void SPIRVCodeGenerator::writeFunctionInstantiation(
        const FunctionDefinition& f,
        Analysis::SpecializationIndex specializationIndex,
        const Analysis::SpecializedParameters* specializedParams,
        OutputStream& out) {
    fActiveSpecialization = specializedParams;
    fActiveSpecializationIndex = specializationIndex;
    this->writeFunctionStart(f.declaration(), out);
}

// 3. Member method directly queries class state without wrapper indirection:
std::string WGSLCodeGenerator::getTypeName(const Type& type) {
    const ProgramSettings& settings = fProgram.fConfig->fSettings;
    ...
}
```
