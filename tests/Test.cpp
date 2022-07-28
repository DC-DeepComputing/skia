/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include <stdlib.h>

#include "include/core/SkString.h"
#include "include/core/SkTime.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(tmpDir, t, nullptr, "Temp directory to use.");

void skiatest::Reporter::bumpTestCount() {}

bool skiatest::Reporter::allowExtendedTest() const { return false; }

bool skiatest::Reporter::verbose() const { return false; }


void skiatest::Reporter::reportFailedWithContext(const skiatest::Failure& f) {
    SkString fullMessage = f.message;
    if (!fContextStack.empty()) {
        fullMessage.append(" [");
        for (int i = 0; i < fContextStack.count(); ++i) {
            if (i > 0) {
                fullMessage.append(", ");
            }
            fullMessage.append(fContextStack[i]);
        }
        fullMessage.append("]");
    }
    this->reportFailed(skiatest::Failure(f.fileName, f.lineNo, f.condition, fullMessage));
}

SkString skiatest::Failure::toString() const {
    SkString result = SkStringPrintf("%s:%d\t", this->fileName, this->lineNo);
    if (!this->message.isEmpty()) {
        result.append(this->message);
        if (strlen(this->condition) > 0) {
            result.append(": ");
        }
    }
    result.append(this->condition);
    return result;
}

SkString skiatest::GetTmpDir() {
    if (!FLAGS_tmpDir.isEmpty()) {
        return SkString(FLAGS_tmpDir[0]);
    }
#ifdef SK_BUILD_FOR_ANDROID
    const char* environmentVariable = "TMPDIR";
    const char* defaultValue = "/data/local/tmp";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
    const char* environmentVariable = "TMPDIR";
    const char* defaultValue = "/tmp";
#elif defined(SK_BUILD_FOR_WIN)
    const char* environmentVariable = "TEMP";
    const char* defaultValue = nullptr;
#else
    const char* environmentVariable = nullptr;
    const char* defaultValue = nullptr;
#endif
    const char* tmpdir = environmentVariable ? getenv(environmentVariable) : nullptr;
    return SkString(tmpdir ? tmpdir : defaultValue);
}

skiatest::Timer::Timer() : fStartNanos(SkTime::GetNSecs()) {}

double skiatest::Timer::elapsedNs() const {
    return SkTime::GetNSecs() - fStartNanos;
}

double skiatest::Timer::elapsedMs() const { return this->elapsedNs() * 1e-6; }

SkMSec skiatest::Timer::elapsedMsInt() const {
    const double elapsedMs = this->elapsedMs();
    SkASSERT(SK_MSecMax >= elapsedMs);
    return static_cast<SkMSec>(elapsedMs);
}

const CtsEnforcement& CtsEnforcement::No() {
    static constexpr CtsEnforcement noEnforcement(CtsEnforcement::NEVER, CtsRelaxed::NEVER);
    return noEnforcement;
}

CtsEnforcement::RunMode CtsEnforcement::eval(int apiLevel) const {
    const int32_t relaxedVersion = static_cast<int32_t>(fWorkaroundsVersion);

    // always return false if both the strict and relaxed versions are undefined
    if (fStrictVersion == NEVER && fStrictVersion == relaxedVersion) {
        return RunMode::kSkip;
    }

    // do a check for strict mode
    if (fStrictVersion != NEVER && apiLevel >= fStrictVersion){
        return RunMode::kRunStrict;
    }

    // all strict mode checks failed so do a final check for relaxed mode
    return (apiLevel >= relaxedVersion) ? RunMode::kRunWithWorkarounds : RunMode::kSkip;
}
