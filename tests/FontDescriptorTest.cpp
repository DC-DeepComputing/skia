/*
 * Copyright 2026 Sebastien Marchand
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/core/SkFontDescriptor.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace {

class FontDataWStream : public SkDynamicMemoryWStream {
public:
    explicit FontDataWStream(bool rejectPayload = false) : fRejectPayload(rejectPayload) {}

    bool write(const void* buffer, size_t size) override {
        fLargestWrite = std::max(fLargestWrite, size);
        return !(fRejectPayload && size >= 1024) && SkDynamicMemoryWStream::write(buffer, size);
    }

    size_t largestWrite() const { return fLargestWrite; }

private:
    bool fRejectPayload;
    size_t fLargestWrite = 0;
};

}  // namespace

DEF_TEST(FontDescriptorStreamSerialize, reporter) {
    auto payload = SkData::MakeUninitialized(16 * 1024);
    auto* bytes = static_cast<uint8_t*>(payload->writable_data());
    for (size_t i = 0; i < payload->size(); ++i) {
        bytes[i] = static_cast<uint8_t>(i);
    }

    for (bool memoryBacked : {true, false}) {
        std::unique_ptr<SkStreamAsset> input;
        if (memoryBacked) {
            input = std::make_unique<SkMemoryStream>(payload);
        } else {
            SkDynamicMemoryWStream blocks;
            for (size_t i = 0; i < payload->size(); i += 1024) {
                blocks.write(bytes + i, 1024);
            }
            input = blocks.detachAsStream();
        }
        REPORTER_ASSERT(reporter, (input->getMemoryBase() != nullptr) == memoryBacked);

        SkFontDescriptor descriptor;
        descriptor.setStream(std::move(input));
        FontDataWStream output;
        REPORTER_ASSERT(reporter, descriptor.serialize(&output));
        if (memoryBacked) {
            // Chunked writes repeatedly scan the output block list in debug builds.
            REPORTER_ASSERT(reporter, output.largestWrite() == payload->size());
        }

        auto serialized = output.detachAsStream();
        SkFontDescriptor restored;
        REPORTER_ASSERT(reporter,
                        SkFontDescriptor::Deserialize(serialized.get(),
                                                      &restored,
                                                      /*sanitizer=*/nullptr));
        REPORTER_ASSERT(reporter, restored.hasStream());
        if (auto stream = restored.detachStream()) {
            auto data = SkData::MakeFromStream(stream.get(), stream->getLength());
            REPORTER_ASSERT(reporter, data && data->equals(payload.get()));
        }

        FontDataWStream rejected(/*rejectPayload=*/true);
        REPORTER_ASSERT(reporter, !descriptor.serialize(&rejected));
    }
}
