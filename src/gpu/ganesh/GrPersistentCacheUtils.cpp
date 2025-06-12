/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrPersistentCacheUtils.h"

#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/sksl/SkSLProgramSettings.h"

#include <algorithm>
#include <cstddef>

namespace GrPersistentCacheUtils {

static constexpr int kCurrentVersion = 12;

int GetCurrentVersion() {
    // The persistent cache stores a copy of the SkSL::Program::Interface struct. If you alter the
    // Program::Interface struct in any way, you must increment kCurrentVersion to invalidate the
    // outdated persistent cache files. The KnownSkSLProgramInterface struct must also be updated
    // to match the new contents of Program::Interface.
    struct KnownSkSLProgramInterface {
        bool useLastFragColor;
        bool useRTFlipUniform;
        bool outputSecondaryColor;
    };
    static_assert(sizeof(SkSL::Program::Interface) == sizeof(KnownSkSLProgramInterface));

    return kCurrentVersion;
}

sk_sp<SkData> PackCachedShaders(SkFourByteTag shaderType,
                                const std::string textShaders[],
                                const std::vector<uint32_t> binaryShaders[],
                                const SkSL::Program::Interface interfaces[],
                                int numInterfaces,
                                const ShaderMetadata* meta) {
    // For consistency (so tools can blindly pack and unpack cached shaders), we always write
    // kGrShaderTypeCount interfaces. If the backend gives us fewer, we just replicate the last one.
    SkASSERT(numInterfaces >= 1 && numInterfaces <= kGrShaderTypeCount);
    // Only one of text or binary shaders must be provided.
    SkASSERT((textShaders != nullptr) != (binaryShaders != nullptr));

    SkBinaryWriteBuffer writer({});
    writer.writeInt(kCurrentVersion);
    writer.writeUInt(shaderType);
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        if (textShaders) {
            writer.writeByteArray(textShaders[i].c_str(), textShaders[i].size());
        } else {
            writer.writeByteArray(binaryShaders[i].data(),
                                  binaryShaders[i].size() * sizeof(uint32_t));
        }

        writer.writePad32(&interfaces[std::min(i, numInterfaces - 1)],
                          sizeof(SkSL::Program::Interface));
    }
    writer.writeBool(SkToBool(meta));
    if (meta) {
        writer.writeBool(SkToBool(meta->fSettings));
        if (meta->fSettings) {
            writer.writeBool(meta->fSettings->fForceNoRTFlip);
            writer.writeBool(meta->fSettings->fFragColorIsInOut);
            writer.writeBool(meta->fSettings->fForceHighPrecision);
            writer.writeBool(meta->fSettings->fUseVulkanPushConstantsForGaneshRTAdjust);
        }

        writer.writeInt(meta->fAttributeNames.size());
        for (const auto& attr : meta->fAttributeNames) {
            writer.writeByteArray(attr.c_str(), attr.size());
        }

        writer.writeBool(meta->fHasSecondaryColorOutput);

        if (meta->fPlatformData) {
            writer.writeByteArray(meta->fPlatformData->data(), meta->fPlatformData->size());
        }
    }
    return writer.snapshotAsData();
}

SkFourByteTag GetType(SkReadBuffer* reader) {
    constexpr SkFourByteTag kInvalidTag = ~0;
    int version           = reader->readInt();
    SkFourByteTag typeTag = reader->readUInt();
    return reader->validate(version == kCurrentVersion) ? typeTag : kInvalidTag;
}

bool UnpackCachedShaders(SkReadBuffer* reader,
                         std::string textShaders[],
                         std::vector<uint32_t> binaryShaders[],
                         SkSL::Program::Interface interfaces[],
                         int numInterfaces,
                         ShaderMetadata* meta) {
    // Only one of text or binary shaders must be provided.
    SkASSERT((textShaders != nullptr) != (binaryShaders != nullptr));

    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        size_t shaderLen = 0;
        const void* shaderBuf = reader->skipByteArray(&shaderLen);
        if (shaderBuf) {
            if (textShaders) {
                textShaders[i].assign(static_cast<const char*>(shaderBuf), shaderLen);
            } else {
                const uint32_t* words = static_cast<const uint32_t*>(shaderBuf);
                SkASSERT(shaderLen % 4 == 0);
                binaryShaders[i].insert(
                        binaryShaders[i].end(), words, words + shaderLen / sizeof(uint32_t));
            }
        }

        // GL, for example, only wants one Interface
        if (i < numInterfaces) {
            reader->readPad32(&interfaces[i], sizeof(interfaces[i]));
        } else {
            reader->skip(sizeof(SkSL::Program::Interface));
        }
    }
    if (reader->readBool() && meta) {
        SkASSERT(meta->fSettings != nullptr);

        if (reader->readBool()) {
            meta->fSettings->fForceNoRTFlip                             = reader->readBool();
            meta->fSettings->fFragColorIsInOut                          = reader->readBool();
            meta->fSettings->fForceHighPrecision                        = reader->readBool();
            meta->fSettings->fUseVulkanPushConstantsForGaneshRTAdjust   = reader->readBool();
        }

        meta->fAttributeNames.resize(reader->readInt());
        for (auto& attr : meta->fAttributeNames) {
            size_t attrLen = 0;
            const char* attrName = static_cast<const char*>(reader->skipByteArray(&attrLen));
            if (attrName) {
                attr.assign(attrName, attrLen);
            }
        }

        meta->fHasSecondaryColorOutput = reader->readBool();

        // a given platform will be responsible for reading its data
    }

    if (!reader->isValid()) {
        for (int i = 0; i < kGrShaderTypeCount; ++i) {
            if (textShaders) {
                textShaders[i].clear();
            } else {
                binaryShaders[i].clear();
            }
        }
    }
    return reader->isValid();
}

}  // namespace GrPersistentCacheUtils
