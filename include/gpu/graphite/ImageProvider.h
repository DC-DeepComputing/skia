/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ImageProvider_DEFINED
#define skgpu_graphite_ImageProvider_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

enum class Mipmapped : bool;
class Recorder;

/*
 * This class provides a centralized location for clients to perform any caching of images
 * they desire. Whenever Graphite encounters an SkImage which is not Graphite-backed
 * it will call ImageProvider::findOrCreate. The client's derived version of this class should
 * return a Graphite-backed version of the provided SkImage that meets the specified
 * requirements. If the requirements are not met by the returned image, Graphite
 * will draw opaque red.
 *
 * Note: by default, Graphite will not perform any caching of images
 *
 * Threading concerns:
 *   If the same ImageProvider is given to multiple Recorders it is up to the
 *   client to handle any required thread synchronization. This is not limited to just
 *   restricting access to whatever map a derived class may have but extends to ensuring
 *   that an image created on one Recorder has had its creation work submitted before it
 *   is used by any work submitted by another Recorder. Please note, this requirement
 *   (re the submission of creation work and image usage on different threads) is common to all
 *   graphite SkImages and isn't unique to SkImages returned by the ImageProvider.
 *
 * TODO(b/240996632): add documentation re shutdown order.
 * TODO(b/240997067): add unit tests
 */
class SK_API ImageProvider : public SkRefCnt {
public:
    class Key {
    public:
        Key(const SkImage* image, Mipmapped mipmapped) {
            fData = (static_cast<uint64_t>(image->uniqueID()) << 32) |
                    (mipmapped == Mipmapped::kYes ? 0x1 : 0x0);
        }

        Key withMipmapping() const { return Key(this->imageID(), Mipmapped::kYes); }
        Key withOutMipmapping() const { return Key(this->imageID(), Mipmapped::kNo); }

        uint64_t asUInt64() const { return fData; }

    private:
        Key(uint32_t imageID, Mipmapped mipmapped) {
            fData = (static_cast<uint64_t>(imageID) << 32) |
                    (mipmapped == Mipmapped::kYes ? 0x1 : 0x0);
        }

        uint32_t imageID() const { return fData >> 32; }

        uint64_t fData;
    };

    // If the client's derived class already has a Graphite-backed image that has the same
    // contents as 'image' and meets the requirements, then it can be returned.
    // makeTextureImage can always be called to create an acceptable Graphite-backed image
    // which could then be cached.
    virtual sk_sp<SkImage> find1(Recorder*,
                                Key,
                                SkImage::RequiredImageProperties) = 0;
    virtual void insert(Recorder*,
                        Key,
                        sk_sp<SkImage> imageToInsert) = 0;

};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ImageProvider_DEFINED
