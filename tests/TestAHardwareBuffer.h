/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestAHardwareBuffer_DEFINED
#define TestAHardwareBuffer_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "tests/Test.h"

#include <android/hardware_buffer.h>

class TestAHardwareBuffer {
public:
    static TestAHardwareBuffer Allocate(skiatest::Reporter* reporter,
                                        const AHardwareBuffer_Desc& hwbDesc) {
        AHardwareBuffer* buffer = nullptr;
        if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
            if (reporter) {
                ERRORF(reporter, "Failed to allocate hardware buffer, error: %d", error);
            }
            return {};
        }
        return TestAHardwareBuffer(buffer);
    }

    TestAHardwareBuffer() = default;

    TestAHardwareBuffer(TestAHardwareBuffer&& that) : fBuffer(that.fBuffer) {
        that.fBuffer = nullptr;
    }

    TestAHardwareBuffer& operator=(TestAHardwareBuffer&& that) {
        if (this != &that) {
            this->reset();
            fBuffer = that.fBuffer;
            that.fBuffer = nullptr;
        }
        return *this;
    }

    ~TestAHardwareBuffer() {
        this->reset();
    }

    void reset() {
        if (fBuffer) {
            AHardwareBuffer_release(fBuffer);
            fBuffer = nullptr;
        }
    }

    AHardwareBuffer* get() const { return fBuffer; }

    explicit operator bool() const { return fBuffer != nullptr; }

    void describe(AHardwareBuffer_Desc* outDesc) const {
        if (fBuffer) {
            AHardwareBuffer_describe(fBuffer, outDesc);
        }
    }

    int lock(uint64_t usage, int32_t fence, const ARect* rect, void** outVirtualAddress) const {
        if (fBuffer) {
            return AHardwareBuffer_lock(fBuffer, usage, fence, rect, outVirtualAddress);
        }
        return -1;
    }

    int unlock(int32_t* fence) const {
        if (fBuffer) {
            return AHardwareBuffer_unlock(fBuffer, fence);
        }
        return -1;
    }

private:
    explicit TestAHardwareBuffer(AHardwareBuffer* buffer) : fBuffer(buffer) {}

    TestAHardwareBuffer(const TestAHardwareBuffer&) = delete;
    TestAHardwareBuffer& operator=(const TestAHardwareBuffer&) = delete;

    AHardwareBuffer* fBuffer = nullptr;
};

#endif // SK_BUILD_FOR_ANDROID && __ANDROID_API__ >= 26

#endif // TestAHardwareBuffer_DEFINED
