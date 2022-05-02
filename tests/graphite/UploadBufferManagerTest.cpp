/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/UploadBufferManager.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UploadBufferManagerTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    UploadBufferManager* bufferManager = recorder->priv().uploadBufferManager();
    sk_sp<CommandBuffer> commandBuffer = resourceProvider->createCommandBuffer();

    // The test source data.
    char src[8] = {
            1, 2, 3, 4,
            5, 6, 7, 8,
    };

    // Test multiple writes to a reused buffer.
    auto [writer0, bufferInfo0] = bufferManager->getUploadWriter(10, 1);
    writer0.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/3, /*rowCount=*/2);
    writer0.write(/*offset=*/6, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/2);

    auto [writer1, bufferInfo1] = bufferManager->getUploadWriter(4, 1);
    writer1.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/2);

    REPORTER_ASSERT(reporter, bufferInfo0.fBuffer == bufferInfo1.fBuffer);
    REPORTER_ASSERT(reporter, bufferInfo0.fOffset == 0);
    REPORTER_ASSERT(reporter, bufferInfo1.fOffset == 10);
    REPORTER_ASSERT(reporter, bufferInfo0.fBuffer->isMapped());
    const void* bufferMap0 = const_cast<Buffer*>(bufferInfo0.fBuffer)->map();
    const char expectedBufferMap0[14] = {
            // From writer0.
            1, 2, 3,
            5, 6, 7,

            1, 2,
            5, 6,

            // From writer1.
            1, 2,
            5, 6,
    };
    REPORTER_ASSERT(reporter,
                    memcmp(bufferMap0, expectedBufferMap0, sizeof(expectedBufferMap0)) == 0);
    bufferManager->transferToCommandBuffer(commandBuffer.get());

    // Test a large write, which should get its own dedicated buffer.
    auto [writer3, bufferInfo3] = bufferManager->getUploadWriter((64 << 10) + 1, 1);
    writer3.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/2);

    REPORTER_ASSERT(reporter, bufferInfo3.fBuffer != bufferInfo0.fBuffer);
    REPORTER_ASSERT(reporter, bufferInfo3.fOffset == 0);
    REPORTER_ASSERT(reporter, bufferInfo3.fBuffer->isMapped());
    const void* bufferMap3 = const_cast<Buffer*>(bufferInfo3.fBuffer)->map();
    const char expectedBufferMap3[4] = {
            1, 2,
            5, 6,
    };
    REPORTER_ASSERT(reporter,
                    memcmp(bufferMap3, expectedBufferMap3, sizeof(expectedBufferMap3)) == 0);
    bufferManager->transferToCommandBuffer(commandBuffer.get());

    // Test writes with a required alignment.
    auto [writer4, bufferInfo4] = bufferManager->getUploadWriter(6, 4);
    writer4.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/3, /*rowCount=*/2);

    auto [writer5, bufferInfo5] = bufferManager->getUploadWriter(2, 4);
    writer5.write(/*offset=*/0, src, /*srcRowBytes=*/4, /*trimRowBytes=*/2, /*rowCount=*/1);

    REPORTER_ASSERT(reporter, bufferInfo4.fBuffer == bufferInfo5.fBuffer);
    REPORTER_ASSERT(reporter, bufferInfo4.fOffset == 0);
    REPORTER_ASSERT(reporter, bufferInfo5.fOffset == 8);
    REPORTER_ASSERT(reporter, bufferInfo4.fBuffer->isMapped());
    const void* bufferMap4 = const_cast<Buffer*>(bufferInfo4.fBuffer)->map();
    const char expectedBufferMap4[10] = {
            // From writer4.
            1, 2, 3,
            5, 6, 7,

            // From alignment.
            0, 0,

            // From writer5.
            1, 2,
    };
    REPORTER_ASSERT(reporter,
                    memcmp(bufferMap4, expectedBufferMap4, sizeof(expectedBufferMap4)) == 0);
}

}  // namespace skgpu::graphite
