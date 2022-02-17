/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderCodeDictionary_DEFINED
#define SkShaderCodeDictionary_DEFINED

#include <array>
#include <unordered_map>
#include <vector>
#include "include/core/SkSpan.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkUniform.h"

enum class DataPayloadType {
    kByte,
};

struct SkDataPayloadInfo {
    const char* fName;
    DataPayloadType fType;
    uint32_t fCount;
};

class SkShaderInfo {
public:
    struct SnippetEntry;
    using GenerateGlueCodeForEntry = std::string (*)(const std::string& resultName,
                                                     int entryIndex, // for uniform name mangling
                                                     const SnippetEntry&,
                                                     const std::vector<std::string>& childNames,
                                                     int indent);

    struct SnippetEntry {
        SnippetEntry() = default;

        SnippetEntry(SkSpan<const SkUniform> uniforms,
                     const char* functionName,
                     const char* code,
                     GenerateGlueCodeForEntry glueCodeGenerator,
                     int numChildren,
                     SkSpan<const SkDataPayloadInfo> dataPayloadExpectations)
             : fUniforms(uniforms)
             , fStaticFunctionName(functionName)
             , fStaticSkSL(code)
             , fGlueCodeGenerator(glueCodeGenerator)
             , fNumChildren(numChildren)
             , fDataPayloadExpectations(dataPayloadExpectations) {
        }

        SkSpan<const SkUniform> fUniforms;
        const char* fStaticFunctionName = nullptr;
        const char* fStaticSkSL = nullptr;
        GenerateGlueCodeForEntry fGlueCodeGenerator = nullptr;
        int fNumChildren = 0;
        SkSpan<const SkDataPayloadInfo> fDataPayloadExpectations;
    };

    void add(const SnippetEntry& entry) {
        fEntries.push_back(entry);
    }

    // TODO: writing to color should be a property of the SnippetEntries and accumulated as the
    // entries are added. _Not_ set manually via 'setWritesColor'.
    void setWritesColor() { fWritesColor = true; }
    bool writesColor() const { return fWritesColor; }

#if SK_SUPPORT_GPU && defined(SK_GRAPHITE_ENABLED) && defined(SK_METAL)
    std::string toSkSL() const;
#endif

private:
    std::string emitGlueCodeForEntry(int* entryIndex, std::string* result, int indent) const;

    std::vector<SnippetEntry> fEntries;
    bool fWritesColor = false;
};

class SkShaderCodeDictionary {
public:
    SkShaderCodeDictionary();

    struct Entry {
    public:
        SkUniquePaintParamsID uniqueID() const {
            SkASSERT(fUniqueID.isValid());
            return fUniqueID;
        }
        const SkPaintParamsKey* paintParamsKey() const { return fKey.get(); }

    private:
        friend class SkShaderCodeDictionary;

        Entry(std::unique_ptr<SkPaintParamsKey> key) : fKey(std::move(key)) {}

        void setUniqueID(uint32_t newID) {
            SkASSERT(!fUniqueID.isValid());
            fUniqueID = SkUniquePaintParamsID(newID);
        }

        SkUniquePaintParamsID fUniqueID;  // fixed-size (uint32_t) unique ID assigned to a key
        std::unique_ptr<SkPaintParamsKey> fKey; // variable-length paint key descriptor
    };

    const Entry* findOrCreate(std::unique_ptr<SkPaintParamsKey>) SK_EXCLUDES(fSpinLock);

    const Entry* lookup(SkUniquePaintParamsID) const SK_EXCLUDES(fSpinLock);

    SkSpan<const SkUniform> getUniforms(SkBuiltInCodeSnippetID) const;

    SkSpan<const SkDataPayloadInfo> getDataPayloadExpectations(int codeSnippetID) const;

    // This method can return nullptr
    const SkShaderInfo::SnippetEntry* getEntry(int codeSnippetID) const;
    const SkShaderInfo::SnippetEntry* getEntry(SkBuiltInCodeSnippetID codeSnippetID) const {
        return this->getEntry(SkTo<int>(codeSnippetID));
    }

    void getShaderInfo(SkUniquePaintParamsID, SkShaderInfo*);

    int maxCodeSnippetID() const {
        return static_cast<int>(SkBuiltInCodeSnippetID::kLast) + fUserDefinedCodeSnippets.size();
    }

    // TODO: this is still experimental but, most likely, it will need to be made thread-safe
    // It returns the code snippet ID to use to identify the supplied user-defined code
    // TODO: add hooks for user to actually provide code.
    int addUserDefinedSnippet(const char* name,
                              SkSpan<const SkDataPayloadInfo> dataPayloadExpectations);

private:
    Entry* makeEntry(std::unique_ptr<SkPaintParamsKey>);

    struct Hash {
        size_t operator()(const SkPaintParamsKey*) const;
    };

    std::array<SkShaderInfo::SnippetEntry, kBuiltInCodeSnippetIDCount> fBuiltInCodeSnippets;

    // The value returned from 'getEntry' must be stable so, hold the user-defined code snippet
    // entries as pointers.
    std::vector<std::unique_ptr<SkShaderInfo::SnippetEntry>> fUserDefinedCodeSnippets;

    // TODO: can we do something better given this should have write-seldom/read-often behavior?
    mutable SkSpinlock fSpinLock;

    std::unordered_map<const SkPaintParamsKey*, Entry*, Hash> fHash SK_GUARDED_BY(fSpinLock);
    std::vector<Entry*> fEntryVector SK_GUARDED_BY(fSpinLock);

    // This arena holds:
    //    the Entries held in 'fHash' and 'fEntryVector' - thus, guarded by 'fSpinLock'
    SkArenaAlloc fArena{256};
};

#endif // SkShaderCodeDictionary_DEFINED
