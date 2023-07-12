/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_AsyncReadTypes_DEFINED
#define skgpu_AsyncReadTypes_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkMessageBus.h"
#include <algorithm>
#include <forward_list>

namespace skgpu {
/**
 * We sometimes hand clients objects that contain mapped buffers. The client may consume
 * the mapped buffer on another thread. This object manages receiving messages that buffers are
 * ready to be unmapped (on the owner's thread). It also handles cleaning up mapped
 * buffers if the owner is destroyed before the client has finished with the buffer.
 *
 * Buffers are first registered using insert() before being passed the client. process() should be
 * called periodically on the owner's thread to poll for messages and process them.
 */
template <typename BufferType, typename RecipientIDType>
class TClientMappedBufferManager {
public:
    /**
     * The message type that internal users of this should post to unmap the buffer.
     * Set fInboxID to inboxID(). fBuffer must have been previously passed to insert().
     */
    struct BufferFinishedMessage {
        BufferFinishedMessage(sk_sp<BufferType> buffer, RecipientIDType intendedRecipient)
                : fBuffer(std::move(buffer)), fIntendedRecipient(intendedRecipient) {}
        BufferFinishedMessage(BufferFinishedMessage&& other) {
            fBuffer = std::move(other.fBuffer);
            fIntendedRecipient = other.fIntendedRecipient;
            other.fIntendedRecipient.makeInvalid();
        }
        sk_sp<BufferType> fBuffer;
        RecipientIDType fIntendedRecipient;
    };
    using BufferFinishedMessageBus = SkMessageBus<BufferFinishedMessage, RecipientIDType,
                                                  false>;

    TClientMappedBufferManager(RecipientIDType ownerID)
            : fFinishedBufferInbox(ownerID) {}
    TClientMappedBufferManager(const TClientMappedBufferManager&) = delete;
    TClientMappedBufferManager(TClientMappedBufferManager&&) = delete;

    ~TClientMappedBufferManager() {
        this->process();
        if (!fAbandoned) {
            // If we're going down before we got the messages we go ahead and unmap all the buffers.
            // It's up to the client to ensure that they aren't being accessed on another thread
            // while this is happening (or afterwards on any thread).
            for (auto& b : fClientHeldBuffers) {
                b->unmap();
            }
        }
    }

    TClientMappedBufferManager& operator=(const TClientMappedBufferManager&) = delete;
    TClientMappedBufferManager& operator=(TClientMappedBufferManager&&) = delete;

    /** Initialize BufferFinishedMessage::fIntendedRecipient to this value. It is the
     *  unique ID of the object that owns this buffer manager.
     */
    RecipientIDType ownerID() const {
        return fFinishedBufferInbox.uniqueID();
    }

    /**
     * Let the manager know to expect a message with buffer 'b'. It's illegal for a buffer to be
     * inserted again before it is unmapped by process().
     */
    void insert(sk_sp<BufferType> b) {
        SkDEBUGCODE(auto end = fClientHeldBuffers.end());
        SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) == end);
        fClientHeldBuffers.emplace_front(std::move(b));
    }

    /** Poll for messages and unmap any incoming buffers. */
    void process() {
        skia_private::STArray<4, BufferFinishedMessage> messages;
        fFinishedBufferInbox.poll(&messages);
        if (!fAbandoned) {
            for (auto& m : messages) {
                this->remove(m.fBuffer);
                m.fBuffer->unmap();
            }
        }
    }

    /** Notifies the manager that the context has been abandoned. No more unmaps() will occur.*/
    void abandon() {
        fAbandoned = true;
        fClientHeldBuffers.clear();
    }

private:
    typename BufferFinishedMessageBus::Inbox fFinishedBufferInbox;
    std::forward_list<sk_sp<BufferType>> fClientHeldBuffers;
    bool fAbandoned = false;

    void remove(const sk_sp<BufferType>& b) {
        // There is no convenient remove only the first element that equals a value functionality in
        // std::forward_list.
        auto prev = fClientHeldBuffers.before_begin();
        auto end = fClientHeldBuffers.end();
        SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) != end);
        for (auto cur = fClientHeldBuffers.begin(); cur != end; prev = cur++) {
            if (*cur == b) {
                fClientHeldBuffers.erase_after(prev);
                break;
            }
        }
        SkASSERT(std::find(fClientHeldBuffers.begin(), end, b) == end);
    }
};

////////////////////////////////////////////////////////////////////////////////


template <typename BufferType, typename RecipientIDType>
class AsyncReadPlane {
public:
    using MappedBufferManager = TClientMappedBufferManager<BufferType, RecipientIDType>;

    AsyncReadPlane() = default;

    AsyncReadPlane(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes)
            : fData(std::move(data))
            , fPixmap(info, fData->data(), rowBytes) {}

    template <typename TransferResultType>
    static AsyncReadPlane MakeFromBuffer(const TransferResultType& transferResult,
                                         const SkImageInfo&        imageInfo,
                                         size_t                    rowBytes,
                                         MappedBufferManager*      manager) {
        SkASSERT(!transferResult.fTransferBuffer->isMapped());
        const void* mappedData = transferResult.fTransferBuffer->map();
        if (!mappedData) {
            return AsyncReadPlane();
        }

        if (transferResult.fPixelConverter) {
            size_t size = rowBytes*imageInfo.height();
            sk_sp<SkData> data = SkData::MakeUninitialized(size);
            transferResult.fPixelConverter(data->writable_data(), mappedData);
            auto plane = AsyncReadPlane(imageInfo, std::move(data), rowBytes);
            transferResult.fTransferBuffer->unmap();
            return plane;
        }

        manager->insert(transferResult.fTransferBuffer);
        return AsyncReadPlane(imageInfo,
                              std::move(transferResult.fTransferBuffer),
                              rowBytes,
                              manager->ownerID());
    }

    AsyncReadPlane(AsyncReadPlane&&) = default;

    ~AsyncReadPlane() {
        if (fMappedBuffer) {
            MappedBufferManager::BufferFinishedMessageBus::Post({std::move(fMappedBuffer), fRecipient});
        }
    }

    AsyncReadPlane& operator=(const AsyncReadPlane&) = delete;
    AsyncReadPlane& operator=(AsyncReadPlane&&) = default;

    const SkPixmap& pixmap() const { return fPixmap; }

    bool isValid() { return pixmap().addr(); }

private:
    AsyncReadPlane(const SkImageInfo& info, sk_sp<BufferType> buffer, size_t rowBytes, RecipientIDType recipient)
            : fMappedBuffer(CheckMapped(std::move(buffer)))
            , fPixmap(info, fMappedBuffer->map(), rowBytes)
            , fRecipient(recipient) {}

    static sk_sp<BufferType> CheckMapped(sk_sp<BufferType> buffer) {
        SkASSERT(buffer->isMapped());
        return buffer;
    }

    sk_sp<SkData> fData;
    sk_sp<BufferType> fMappedBuffer;
    SkPixmap fPixmap;
    RecipientIDType fRecipient;
};

/////////////////////////////////////////////////////////////////////////////

template <typename BufferType, typename RecipientIDType>
class RGBAAsyncReadResult : public SkImage::RGBAAsyncReadPixelsResult {
public:
    using Plane = AsyncReadPlane<BufferType, RecipientIDType>;

    static std::unique_ptr<SkImage::RGBAAsyncReadPixelsResult> Make(Plane plane) {
        if (!plane.isValid()) {
            return nullptr;
        }
        return std::unique_ptr<SkImage::RGBAAsyncReadPixelsResult>(new RGBAAsyncReadResult(std::move(plane)));
    }

    const SkPixmap& pixmap() const override { return fPlane.pixmap(); }

private:
    RGBAAsyncReadResult(Plane plane) : fPlane(std::move(plane)) {}
    Plane fPlane;
};

/////////////////////////////////////////////////////////////////////////////

template <typename BufferType, typename RecipientIDType>
class YUVAAsyncReadResult : public SkImage::YUVAAsyncReadPixelsResult {
public:
    using Plane = AsyncReadPlane<BufferType, RecipientIDType>;

    static std::unique_ptr<SkImage::YUVAAsyncReadPixelsResult> Make(const SkYUVAInfo& yuvaInfo, Plane planes[]) {
        SkPixmap pixmaps[SkYUVAPixmaps::kMaxPlanes];
        for (int i = 0; i < yuvaInfo.numPlanes(); ++i) {
            pixmaps[i] = planes[i].pixmap();
        }

        auto yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pixmaps);
        if (!yuvaPixmaps.isValid()) {
            return nullptr;
        }

        std::unique_ptr<YUVAAsyncReadResult> result(new YUVAAsyncReadResult);

        result->fYUVAPixmaps = std::move(yuvaPixmaps);
        for (int i = 0; i < yuvaInfo.numPlanes(); ++i) {
            result->fPlanes[i] = std::move(planes[i]);
        }

        return result;
    }

public:
    YUVAAsyncReadResult() = default;

    const SkYUVAPixmaps& pixmaps() const override { return fYUVAPixmaps; }

private:
    Plane fPlanes[SkYUVAPixmaps::kMaxPlanes];
    SkYUVAPixmaps fYUVAPixmaps;
};

}  // namespace skgpu

#endif  // skgpu_AsyncReadTypes_DEFINED

