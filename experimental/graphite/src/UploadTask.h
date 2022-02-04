/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UploadTask_DEFINED
#define skgpu_UploadTask_DEFINED

#include "experimental/graphite/src/Task.h"

#include <vector>

#include "include/core/SkRefCnt.h"

namespace skgpu {

class Buffer;
struct BufferTextureCopyData;
class CommandBuffer;
class Recorder;
class ResourceProvider;
class TextureProxy;
class UploadList;

/**
 * An UploadCommand represents a single set of uploads from a buffer to texture that
 * can be processed in a single command.
 */
class UploadCommand {
public:
    // Process the current UploadList to make a list of UploadCommands
    static std::vector<UploadCommand> MakeCommands(Recorder*,
                                                   const std::unique_ptr<UploadList>&);
    UploadCommand(sk_sp<Buffer>, sk_sp<TextureProxy>, std::vector<BufferTextureCopyData>);

    void addCommand(ResourceProvider*, CommandBuffer*) const;

private:
    sk_sp<Buffer> fBuffer;
    sk_sp<TextureProxy> fTextureProxy;
    std::vector<BufferTextureCopyData> fCopyData;
};

/*
 * An UploadTask is a collection of UploadCommands.
 *
 * When adding commands to the commandBuffer the texture proxies in those
 * commands will be instantiated and the copy command added.
 */
class UploadTask final : public Task {
public:
    static sk_sp<UploadTask> Make(std::vector<UploadCommand>);

    ~UploadTask() override;

    void addCommands(ResourceProvider*, CommandBuffer*) override;

private:
    UploadTask(std::vector<UploadCommand>);

    std::vector<UploadCommand> fCommands;
};

} // namespace skgpu

#endif // skgpu_UploadTask_DEFINED
