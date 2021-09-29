/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include <memory>
#include <unordered_map>

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class ErrorReporter;
class Mangler;
class ModifiersPool;
struct ProgramConfig;

/**
 * Intrinsics are passed between the Compiler and the IRGenerator using IRIntrinsicMaps.
 */
class IRIntrinsicMap {
public:
    IRIntrinsicMap(IRIntrinsicMap* parent) : fParent(parent) {}

    void insertOrDie(String key, std::unique_ptr<ProgramElement> element) {
        SkASSERT(fIntrinsics.find(key) == fIntrinsics.end());
        fIntrinsics[key] = Intrinsic{std::move(element), false};
    }

    const ProgramElement* find(const String& key) {
        auto iter = fIntrinsics.find(key);
        if (iter == fIntrinsics.end()) {
            return fParent ? fParent->find(key) : nullptr;
        }
        return iter->second.fIntrinsic.get();
    }

    // Only returns an intrinsic that isn't already marked as included, and then marks it.
    const ProgramElement* findAndInclude(const String& key) {
        auto iter = fIntrinsics.find(key);
        if (iter == fIntrinsics.end()) {
            return fParent ? fParent->findAndInclude(key) : nullptr;
        }
        if (iter->second.fAlreadyIncluded) {
            return nullptr;
        }
        iter->second.fAlreadyIncluded = true;
        return iter->second.fIntrinsic.get();
    }

    void resetAlreadyIncluded() {
        for (auto& pair : fIntrinsics) {
            pair.second.fAlreadyIncluded = false;
        }
        if (fParent) {
            fParent->resetAlreadyIncluded();
        }
    }

private:
    struct Intrinsic {
        std::unique_ptr<ProgramElement> fIntrinsic;
        bool fAlreadyIncluded = false;
    };

    std::unordered_map<String, Intrinsic> fIntrinsics;
    IRIntrinsicMap* fParent = nullptr;
};

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(ErrorReporter& errors, const ShaderCapsClass& caps, Mangler& mangler);
    ~Context();

    // The Context holds all of the built-in types.
    BuiltinTypes fTypes;

    // The Context holds a reference to our shader caps bits.
    const ShaderCapsClass& fCaps;

    // The Context holds a pointer to our pool of modifiers.
    ModifiersPool* fModifiersPool = nullptr;

    // The Context holds a pointer to the configuration of the program being compiled.
    ProgramConfig* fConfig = nullptr;

    // The Context holds a pointer to our error reporter.
    ErrorReporter* fErrors;

    // The Context holds a pointer to the shared name-mangler.
    Mangler* fMangler = nullptr;

    // Symbols which have definitions in the include files.
    IRIntrinsicMap* fIntrinsics = nullptr;
};

}  // namespace SkSL

#endif
