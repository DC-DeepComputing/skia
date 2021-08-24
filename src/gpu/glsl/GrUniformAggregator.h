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

#include <cstdio>
#include <unordered_map>
#include <vector>

/** Collects the uniforms from various processors comprising the shaders of a pipeline/program. */
class GrUniformAggregator {
public:
    struct Record {
        GrProcessor::Uniform uniform;
        size_t               indexInProcessor;
        SkString             nameStorage;
    };

    using ProcessorUniformMap = std::unordered_map</*processor index*/ int, std::vector<Record>>;

private:
    class Range {
    public:
        Range(const GrUniformAggregator& agg) : fAgg(agg) {}
        auto begin() const { return Iter(fAgg); }
        auto   end() const { return Iter::MakeEnd(fAgg); }

    private:
        class Iter {
        private:
            Iter() = default;
            std::vector<Record>::const_iterator fVectorIter;
            ProcessorUniformMap::const_iterator fMapIter;
            ProcessorUniformMap::const_iterator fMapEnd;

        public:
            Iter(const GrUniformAggregator& agg)
                    : fMapIter(agg.fMap.cbegin()), fMapEnd(agg.fMap.cend()) {
                if (fMapIter != agg.fMap.cend()) {
                    fVectorIter = fMapIter->second.cbegin();
                }
            }
            static Iter MakeEnd(const GrUniformAggregator& agg) {
                Iter end;
                end.fMapIter = agg.fMap.cend();
                end.fMapEnd  = agg.fMap.cend();
                return end;
            }
            std::tuple</*processor index*/    int,
                       /*index in processor*/ int,
                       const GrProcessor::Uniform&>
            operator*() const {
                return {fMapIter->first,
                        fVectorIter - fMapIter->second.cbegin(),
                        fVectorIter->uniform};
            }
            Iter& operator++() {
                ++fVectorIter;
                if (fVectorIter == fMapIter->second.cend()) {
                    ++fMapIter;
                    if (fMapIter != fMapEnd) {
                        fVectorIter = fMapIter->second.cbegin();
                    } else {
                        fVectorIter = {};  // this ensures we match MakeEnd().
                    }
                }
                return *this;
            }
            bool operator==(const Iter& that) const {
                return fMapIter == that.fMapIter && fVectorIter == that.fVectorIter;
            }
            bool operator!=(const Iter& that) const { return !(*this == that); }
        };
        const GrUniformAggregator& fAgg;
    };

public:
    GrUniformAggregator() = default;

    void addUniforms(const GrProcessor& p, int processorIndex, const char* mangleSuffix);

    GrProcessor::Uniform findUniform(const GrProcessor* p, size_t index) const;

    /** Snaps off the recorded uniform and resets this object to its initial state. */
    ProcessorUniformMap snapUniforms();

    Range uniforms() const { return Range(*this); }

    int uniformCount() const { return fTotalUniformCount; }

private:
    GrUniformAggregator(const GrUniformAggregator&) = delete;
    GrUniformAggregator& operator=(const GrUniformAggregator&) = delete;

    ProcessorUniformMap fMap;

    // Used to implement findUniform.
    std::unordered_map<const GrProcessor*, /*processor index*/ int> fProcessorToIndexMap;

    int fTotalUniformCount = 0;
};

#endif
