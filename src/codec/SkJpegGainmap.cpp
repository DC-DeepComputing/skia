/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegGainmap.h"

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkGainmapInfo.h"
#include "include/utils/SkParse.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/xml/SkDOM.h"

#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

/*
 * Helper function to search through an XML tree for a node that indicating that the image is an HDR
 * gainmap. The format of the node must be:
 *
 *   <rdf:Description rdf:about=""
 *         xmlns:apdi="http://ns.apple.com/pixeldatainfo/1.0/"
 *         xmlns:HDRGainMap="http://ns.apple.com/HDRGainMap/1.0/">
 *      <apdi:AuxiliaryImageType>urn:com:apple:photo:2020:aux:hdrgainmap</apdi:AuxiliaryImageType>
 *      <apdi:StoredFormat>1278226488</apdi:StoredFormat>
 *      <apdi:NativeFormat>1278226488</apdi:NativeFormat>
 *      <HDRGainMap:HDRGainMapVersion>65536</HDRGainMap:HDRGainMapVersion>
 *   </rdf:Description>
 *
 * The StoredFormat and NativeFormat correspond to 'L008' or kCVPixelFormatType_OneComponent8,
 * which is redundant information, so ignore these. Require all other fields.
 */
static bool FindHDRGainMapNode(const SkDOM& dom, const SkDOM::Node* node, SkGainmapInfo* info) {
    // Recurse on children before checking this node.
    for (const auto* child = dom.getFirstChild(node); child; child = dom.getNextSibling(child)) {
        if (FindHDRGainMapNode(dom, child, info)) {
            return true;
        }
    }
    if (dom.getType(node) != SkDOM::kElement_Type) {
        return false;
    }
    const char* nodeName = dom.getName(node);
    if (!nodeName || strcmp("rdf:Description", nodeName) != 0) {
        return false;
    }
    if (!dom.hasAttr(node, "xmlns:apdi", "http://ns.apple.com/pixeldatainfo/1.0/")) {
        return false;
    }
    if (!dom.hasAttr(node, "xmlns:HDRGainMap", "http://ns.apple.com/HDRGainMap/1.0/")) {
        return false;
    }
    bool foundExpectedVersion = false;
    bool foundExpectedAuxiliaryImageType = false;
    for (const auto* child = dom.getFirstChild(node); child; child = dom.getNextSibling(child)) {
        const char* childName = dom.getName(child);
        if (!childName) {
            continue;
        }
        if (!strcmp("HDRGainMap:HDRGainMapVersion", childName)) {
            for (const auto* grandChild = dom.getFirstChild(child); grandChild;
                 grandChild = dom.getNextSibling(grandChild)) {
                if (dom.getType(grandChild) != SkDOM::kText_Type) {
                    continue;
                }
                const char* grandChildName = dom.getName(grandChild);
                if (!grandChildName) {
                    continue;
                }
                int32_t value = 0;
                if (!SkParse::FindS32(grandChildName, &value)) {
                    continue;
                }
                if (value == 65536) {
                    foundExpectedVersion = true;
                }
            }
        }
        if (!strcmp("apdi:AuxiliaryImageType", childName)) {
            for (const auto* grandChild = dom.getFirstChild(child); grandChild;
                 grandChild = dom.getNextSibling(grandChild)) {
                if (dom.getType(grandChild) != SkDOM::kText_Type) {
                    continue;
                }
                const char* grandChildName = dom.getName(grandChild);
                if (!grandChildName) {
                    continue;
                }
                if (!strstr(grandChildName, "urn:com:apple:photo:2020:aux:hdrgainmap")) {
                    continue;
                }
                foundExpectedAuxiliaryImageType = true;
            }
        }
    }
    if (!foundExpectedVersion || !foundExpectedAuxiliaryImageType) {
        return false;
    }

    // Populate the gainmap rendering parameters for this format.
    constexpr float kLogRatioMin = 0.f;
    constexpr float kLogRatioMax = 1.f;
    info->fLogRatioMin = {kLogRatioMin, kLogRatioMin, kLogRatioMin, 1.f};
    info->fLogRatioMax = {kLogRatioMax, kLogRatioMax, kLogRatioMax, 1.f};
    info->fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
    info->fEpsilonSdr = 1 / 128.f;
    info->fEpsilonHdr = 1 / 128.f;
    info->fHdrRatioMin = 1.f;
    info->fHdrRatioMax = sk_float_exp(kLogRatioMax);
    return true;
}

static bool SkJpegGetMultiPictureGainmapInternal(SkStream* decoderStream,
                                                 SkGainmapInfo* outGainmapInfo,
                                                 std::unique_ptr<SkStream>* outGainmapImageStream) {
    // Scan of the original decoder stream.
    auto scan = SkJpegSegmentScan::Create(decoderStream, SkJpegSegmentScan::Options());
    if (!scan) {
        return false;
    }

    // Extract the Multi-Picture image streams in the original decoder image.
    auto mpStreams = SkJpegExtractMultiPictureStreams(scan.get());
    if (!mpStreams) {
        return false;
    }

    // Iterate over the MP image streams.
    for (auto& mpImage : mpStreams->images) {
        if (!mpImage.stream) {
            continue;
        }

        // Create a scan of this MP image.
        auto mpImageScan =
                SkJpegSegmentScan::Create(mpImage.stream.get(), SkJpegSegmentScan::Options());
        if (!mpImageScan) {
            continue;
        }

        // Search for the XMP metadata in the MP image's scan.
        for (const auto& segment : mpImageScan->segments()) {
            if (segment.marker != kXMPMarker) {
                continue;
            }
            auto xmpMetadata = mpImageScan->copyParameters(segment, kXMPSig, sizeof(kXMPSig));
            if (!xmpMetadata) {
                continue;
            }

            // Parse the XMP metadata.
            std::unique_ptr<SkMemoryStream> xmlStream =
                    SkMemoryStream::MakeDirect(xmpMetadata->data(), xmpMetadata->size());
            if (!xmlStream) {
                continue;
            }
            SkDOM dom;
            const SkDOM::Node* rootNode = dom.build(*xmlStream);
            if (!rootNode) {
                continue;
            }

            // Search for a gainmap-indicating node in the XML tree.
            if (!FindHDRGainMapNode(dom, rootNode, outGainmapInfo)) {
                continue;
            }

            // This MP image is the gainmap image. Return it.
            if (outGainmapImageStream) {
                if (!mpImage.stream->rewind()) {
                    SkCodecPrintf("Failed to rewind gainmap image stream.\n");
                    return false;
                }
                *outGainmapImageStream = std::move(mpImage.stream);
            }
            return true;
        }
    }
    return false;
}

bool SkJpegGetMultiPictureGainmap(sk_sp<const SkData> decoderMpfMetadata,
                                  SkStream* decoderStream,
                                  SkGainmapInfo* outInfo,
                                  std::unique_ptr<SkStream>* outGainmapImageStream) {
    // The decoder has already scanned for MPF metadata. If it doesn't exist, or it doesn't parse,
    // then early-out.
    if (!decoderMpfMetadata) {
        return false;
    }
    if (!SkJpegParseMultiPicture(decoderMpfMetadata)) {
        return false;
    }

    // The current implementation of Multi-Picture images requires a seekable stream.
    // Save the position so that it can be restored before returning.
    if (!decoderStream->hasPosition()) {
        SkCodecPrintf("Multi-Picture gainmap extraction requires a seekable stream.\n");
        return false;
    }
    size_t streamPositionToRestore = decoderStream->getPosition();
    if (!decoderStream->rewind()) {
        SkCodecPrintf("Failed to rewind decoder stream.\n");
        return false;
    }

    // Search for a gainmap amongst the Multi-Picture images.
    bool result =
            SkJpegGetMultiPictureGainmapInternal(decoderStream, outInfo, outGainmapImageStream);

    // Restore the decoder's stream to its initial state.
    if (!decoderStream->seek(streamPositionToRestore)) {
        SkCodecPrintf("Failed reset decoder stream.\n");
    }
    return result;
}

// Helper macro for iterating SkDOM::Node children.
#define ITER_CHILDREN(PARENT, CHILD, NAME)               \
    const auto* CHILD = dom.getFirstChild(PARENT, NAME); \
    CHILD;                                               \
    CHILD = dom.getNextSibling(CHILD, NAME)

static bool SkJpegGetJpegRGainmapParseXMP(sk_sp<const SkData> xmpMetadata,
                                          size_t* outOffset,
                                          size_t* outSize,
                                          float* outRangeScalingFactor) {
    printf("** SkJpegGetJpegRGainmap\n");
    if (!xmpMetadata) {
        return false;
    }
    std::unique_ptr<SkMemoryStream> xmlStream =
            SkMemoryStream::MakeDirect(xmpMetadata->data(), xmpMetadata->size());
    if (!xmlStream) {
        return false;
    }
    SkDOM dom;
    const SkDOM::Node* root = dom.build(*xmlStream);
    if (!root) {
        return false;
    }

    for (ITER_CHILDREN(root, rdfRdfNode, "rdf:RDF")) {
        for (ITER_CHILDREN(rdfRdfNode, rdfDescriptionNode, "rdf:Description")) {
            if (!dom.hasAttr(rdfDescriptionNode,
                             "xmlns:GContainer",
                             "http://ns.google.com/photos/1.0/container/")) {
                continue;
            }
            if (!dom.hasAttr(rdfDescriptionNode,
                             "xmlns:RecoveryMap",
                             "http://ns.google.com/photos/1.0/recoverymap/")) {
                continue;
            }
            const auto* version = dom.getFirstChild(rdfDescriptionNode, "GContainer:Version");
            if (!version) {
                SkCodecPrintf("Missing GContainer:Version");
                return false;
            }
            const auto* directory = dom.getFirstChild(rdfDescriptionNode, "GContainer:Directory");
            if (!version) {
                SkCodecPrintf("Missing GContainer:Directory");
                return false;
            }
            const auto* seq = dom.getFirstChild(directory, "rdf:Seq");
            if (!seq) {
                SkCodecPrintf("Missing rdf:Seq");
                return false;
            }

            size_t totalOffset = 0;
            size_t gContainerItemIndex = 0;
            for (ITER_CHILDREN(seq, li, "rdf:li")) {
                const auto* item = dom.getFirstChild(li, "GContainer:Item");
                if (!item) {
                    SkCodecPrintf("List item does not have GContainer:Item.\n");
                    return false;
                }
                if (gContainerItemIndex == 0) {
                    if (!dom.hasAttr(item, "GContainer:ItemSemantic", "Primary")) {
                        SkCodecPrintf("First item is not Primary.\n");
                        return false;
                    }
                    if (!dom.hasAttr(item, "RecoveryMap:Version", "1")) {
                        SkCodecPrintf("RecoveryMap:Version is not 1.");
                        return false;
                    }
                    SkScalar rangeScalingFactor = 1.f;
                    if (!dom.findScalars(
                                item, "RecoveryMap:RangeScalingFactor", &rangeScalingFactor, 1)) {
                        SkCodecPrintf("RecoveryMap:RangeScalingFactor is absent.");
                        return false;
                    }
                    *outRangeScalingFactor = rangeScalingFactor;
                } else {
                    int32_t length = 0;
                    if (!dom.findS32(item, "GContainer:ItemLength", &length)) {
                        SkCodecPrintf("GContainer:ItemLength is absent.");
                        return false;
                    }
                    if (!dom.hasAttr(item, "GContainer:ItemSemantic", "RecoveryMap")) {
                        totalOffset += length;
                        continue;
                    }
                    *outOffset = totalOffset;
                    *outSize = length;
                    return true;
                }
                gContainerItemIndex += 1;
            }
        }
    }
    return true;
}

bool SkJpegGetJpegRGainmap(sk_sp<const SkData> xmpMetadata,
                           SkStream* decoderStream,
                           SkGainmapInfo* outInfo,
                           std::unique_ptr<SkStream>* outGainmapImageStream) {
    size_t itemOffset = 0;
    size_t itemSize = 0;
    float rangeScalingFactor = 1.f;
    if (!SkJpegGetJpegRGainmapParseXMP(xmpMetadata, &itemOffset, &itemSize, &rangeScalingFactor)) {
        return false;
    }
    // printf("%d %d %f\n", (int)offset, (int)size, rangeScalingFactor);


    // The current implementation of Multi-Picture images requires a seekable stream.
    // Save the position so that it can be restored before returning.
    if (!decoderStream->hasPosition()) {
        SkCodecPrintf("Multi-Picture gainmap extraction requires a seekable stream.\n");
        return false;
    }
    size_t streamPositionToRestore = decoderStream->getPosition();
    if (!decoderStream->rewind()) {
        SkCodecPrintf("Failed to rewind decoder stream.\n");
        return false;
    }

    ////


    SkJpegSegmentScan::Options options;
    options.stopOnStartOfScan = false;
    auto scan = SkJpegSegmentScan::Create(decoderStream, options);
    if (!scan) {
      printf("no scan!\n");
      return false;
    }

    const auto& lastSegment = scan->segments().back();
    printf("%x\n", lastSegment.marker);

    auto gainmapImageStream = scan->getSubsetStream(
          lastSegment.offset + SkJpegSegmentScan::kMarkerCodeSize + itemOffset, itemSize);
    if (!gainmapImageStream) {
        return false;
    }

    FILE* fout = fopen("out.jpg", "wb");
    uint8_t byte;
    while (gainmapImageStream->read(&byte, 1) == 1) {
        fwrite(&byte, 1, 1, fout);
    }
    fclose(fout);
    gainmapImageStream->rewind();

    *outGainmapImageStream = std::move(gainmapImageStream);

    // Restore the decoder's stream to its initial state.
    if (!decoderStream->seek(streamPositionToRestore)) {
        SkCodecPrintf("Failed reset decoder stream.\n");
    }
    return true;
}
