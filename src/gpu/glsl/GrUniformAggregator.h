/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrUniformAggregator_DEFINED
#define GrUniformAggregator_DEFINED

#include "include/core/SkString.h"
#include "include/private/SkChecksum.h"
#include "src/gpu/GrProcessor.h"

#include <unordered_map>
#include <vector>
#include <stdio.h>

/** Collects the uniforms from various processors comprising the shaders of a pipeline/program. */
class GrUniformAggregator {
public:
    struct Record {
        GrProcessor::Uniform uniform;
        int                  processorIndex;
        size_t               indexInProcessor;
        SkString             nameStorage;
    };

private:
    class Range  {
    public:
        Range(const GrUniformAggregator& agg) : fAgg(agg) {}
        auto begin() const { return Iter(fAgg.fRecords.cbegin()); }
        auto   end() const { return Iter(fAgg.fRecords.cend()); }

    private:
        class Iter {
        private:
            std::vector<Record>::const_iterator fIter;
        public:
            Iter(std::vector<Record>::const_iterator iter) : fIter(iter) {}
            const GrProcessor::Uniform& operator*() const { return fIter->uniform; }
            Iter& operator++() { ++fIter; return *this; }
            bool operator==(const Iter& that) const { return fIter == that.fIter; }
            bool operator!=(const Iter& that) const { return !(*this == that); }
        };
        const GrUniformAggregator& fAgg;
    };

public:
    GrUniformAggregator() = default;

    void addUniforms(const GrProcessor* p, const char* mangleSuffix);

    GrProcessor::Uniform get(const GrProcessor* p, size_t index) const;

    using UniformRecords = std::vector<Record>;

    /** Snaps off the recorded uniform and resets this object to its initial state. */
    UniformRecords snapUniformRecords();

    Range uniforms() const { return Range(*this); }

    size_t count() const { return fRecords.size(); }

    GrProcessor::Uniform getUniform(size_t index) const { return fRecords[index].uniform; }

private:
    GrUniformAggregator(const GrUniformAggregator&) = delete;
    GrUniformAggregator& operator=(const GrUniformAggregator&) = delete;

    using Handle = std::tuple<const GrProcessor*, size_t>;

    std::vector<Record> fRecords;

    static_assert(sizeof(Handle) == sizeof(const GrProcessor*) + sizeof(size_t));
    std::unordered_map<Handle, /*index in fRecords*/ int, SkGoodHash> fMap;

    int fProcessorIndex = 0;
};

#endif
