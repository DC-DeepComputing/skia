#include "src/core/SkStreamPriv.h"

#include "include/core/SkData.h"
#include "include/core/SkTypes.h"

bool SkDebugfStream::write(const void* buffer, size_t size) {
    SkDebugf("%.*s", (int)size, (const char*)buffer);
    fBytesWritten += size;
    return true;
}

size_t SkDebugfStream::bytesWritten() const {
    return fBytesWritten;
}

sk_sp<SkData> SkCopyStreamToData(SkStream* stream) {
    SkASSERT(stream != nullptr);

    if (stream->hasLength()) {
        return SkData::MakeFromStream(stream, stream->getLength());
    }

    SkDynamicMemoryWStream tempStream;
    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    do {
        size_t bytesRead = stream->read(buffer, bufferSize);
        tempStream.write(buffer, bytesRead);
    } while (!stream->isAtEnd());
    return tempStream.detachAsData();
}

bool SkStreamCopy(SkWStream* out, SkStream* input) {
    const char* base = static_cast<const char*>(input->getMemoryBase());
    if (base && input->hasPosition() && input->hasLength()) {
        // Shortcut that avoids the while loop.
        size_t position = input->getPosition();
        size_t length = input->getLength();
        SkASSERT(length >= position);
        return out->write(&base[position], length - position);
    }
    char scratch[4096];
    size_t count;
    while (true) {
        count = input->read(scratch, sizeof(scratch));
        if (0 == count) {
            return true;
        }
        if (!out->write(scratch, count)) {
            return false;
        }
    }
}

bool StreamRemainingLengthIsBelow(SkStream* stream, size_t len) {
    if (stream->hasLength() && stream->hasPosition()) {
        size_t remainingBytes = stream->getLength() - stream->getPosition();
        return len > remainingBytes;
    }
    return false;
}
