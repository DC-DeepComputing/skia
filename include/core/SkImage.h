/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_DEFINED
#define SkImage_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAPI.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

class GrBackendFormat;
class GrBackendTexture;
class GrContextThreadSafeProxy;
class GrDirectContext;
class GrRecordingContext;
class GrYUVABackendTextureInfo;
class GrYUVABackendTextures;
class SkBitmap;
class SkColorSpace;
class SkData;
class SkImageFilter;
class SkImageGenerator;
class SkMatrix;
class SkMipmap;
class SkPaint;
class SkPicture;
class SkPixmap;
class SkPromiseImageTexture;
class SkShader;
class SkSurfaceProps;
class SkYUVAPixmaps;
enum GrSurfaceOrigin : int;
enum SkColorType : int;
enum class GrSemaphoresSubmitted : bool;
enum class SkEncodedImageFormat;
enum class SkTileMode;
struct GrFlushInfo;
struct SkIPoint;
struct SkSamplingOptions;

namespace skgpu {
enum class Mipmapped : bool;
enum class Budgeted : bool;
}

/** \class SkImage
    SkImage describes a two dimensional array of pixels to draw. The pixels may be
    decoded in a raster bitmap, encoded in a SkPicture or compressed data stream,
    or located in GPU memory as a GPU texture.

    SkImage cannot be modified after it is created. SkImage may allocate additional
    storage as needed; for instance, an encoded SkImage may decode when drawn.

    SkImage width and height are greater than zero. Creating an SkImage with zero width
    or height returns SkImage equal to nullptr.

    SkImage may be created from SkBitmap, SkPixmap, SkSurface, SkPicture, encoded streams,
    GPU texture, YUV_ColorSpace data, or hardware buffer. Encoded streams supported
    include BMP, GIF, HEIF, ICO, JPEG, PNG, WBMP, WebP. Supported encoding details
    vary with platform.

    See sk_image_factory namespace for the static factory methods to make SkImages.
*/
class SK_API SkImage : public SkRefCnt {
public:
    /** Returns a SkImageInfo describing the width, height, color type, alpha type, and color space
        of the SkImage.

        @return  image info of SkImage.
    */
    const SkImageInfo& imageInfo() const { return fInfo; }

    /** Returns pixel count in each row.

        @return  pixel width in SkImage
    */
    int width() const { return fInfo.width(); }

    /** Returns pixel row count.

        @return  pixel height in SkImage
    */
    int height() const { return fInfo.height(); }

    /** Returns SkISize { width(), height() }.

        @return  integral size of width() and height()
    */
    SkISize dimensions() const { return SkISize::Make(fInfo.width(), fInfo.height()); }

    /** Returns SkIRect { 0, 0, width(), height() }.

        @return  integral rectangle from origin to width() and height()
    */
    SkIRect bounds() const { return SkIRect::MakeWH(fInfo.width(), fInfo.height()); }

    /** Returns value unique to image. SkImage contents cannot change after SkImage is
        created. Any operation to create a new SkImage will receive generate a new
        unique number.

        @return  unique identifier
    */
    uint32_t uniqueID() const { return fUniqueID; }

    /** Returns SkAlphaType.

        SkAlphaType returned was a parameter to an SkImage constructor,
        or was parsed from encoded data.

        @return  SkAlphaType in SkImage

        example: https://fiddle.skia.org/c/@Image_alphaType
    */
    SkAlphaType alphaType() const;

    /** Returns SkColorType if known; otherwise, returns kUnknown_SkColorType.

        @return  SkColorType of SkImage

        example: https://fiddle.skia.org/c/@Image_colorType
    */
    SkColorType colorType() const;

    /** Returns SkColorSpace, the range of colors, associated with SkImage.  The
        reference count of SkColorSpace is unchanged. The returned SkColorSpace is
        immutable.

        SkColorSpace returned was passed to an SkImage constructor,
        or was parsed from encoded data. SkColorSpace returned may be ignored when SkImage
        is drawn, depending on the capabilities of the SkSurface receiving the drawing.

        @return  SkColorSpace in SkImage, or nullptr

        example: https://fiddle.skia.org/c/@Image_colorSpace
    */
    SkColorSpace* colorSpace() const;

    /** Returns a smart pointer to SkColorSpace, the range of colors, associated with
        SkImage.  The smart pointer tracks the number of objects sharing this
        SkColorSpace reference so the memory is released when the owners destruct.

        The returned SkColorSpace is immutable.

        SkColorSpace returned was passed to an SkImage constructor,
        or was parsed from encoded data. SkColorSpace returned may be ignored when SkImage
        is drawn, depending on the capabilities of the SkSurface receiving the drawing.

        @return  SkColorSpace in SkImage, or nullptr, wrapped in a smart pointer

        example: https://fiddle.skia.org/c/@Image_refColorSpace
    */
    sk_sp<SkColorSpace> refColorSpace() const;

    /** Returns true if SkImage pixels represent transparency only. If true, each pixel
        is packed in 8 bits as defined by kAlpha_8_SkColorType.

        @return  true if pixels represent a transparency mask

        example: https://fiddle.skia.org/c/@Image_isAlphaOnly
    */
    bool isAlphaOnly() const;

    /** Returns true if pixels ignore their alpha value and are treated as fully opaque.

        @return  true if SkAlphaType is kOpaque_SkAlphaType
    */
    bool isOpaque() const { return SkAlphaTypeIsOpaque(this->alphaType()); }

    /**
     *  Make a shader with the specified tiling and mipmap sampling.
     */
    sk_sp<SkShader> makeShader(SkTileMode tmx, SkTileMode tmy, const SkSamplingOptions&,
                               const SkMatrix* localMatrix = nullptr) const;
    sk_sp<SkShader> makeShader(SkTileMode tmx, SkTileMode tmy, const SkSamplingOptions& sampling,
                               const SkMatrix& lm) const;
    /** Defaults to clamp in both X and Y. */
    sk_sp<SkShader> makeShader(const SkSamplingOptions& sampling, const SkMatrix& lm) const;
    sk_sp<SkShader> makeShader(const SkSamplingOptions& sampling,
                               const SkMatrix* lm = nullptr) const;

    /**
     *  makeRawShader functions like makeShader, but for images that contain non-color data.
     *  This includes images encoding things like normals, material properties (eg, roughness),
     *  heightmaps, or any other purely mathematical data that happens to be stored in an image.
     *  These types of images are useful with some programmable shaders (see: SkRuntimeEffect).
     *
     *  Raw image shaders work like regular image shaders (including filtering and tiling), with
     *  a few major differences:
     *    - No color space transformation is ever applied (the color space of the image is ignored).
     *    - Images with an alpha type of kUnpremul are *not* automatically premultiplied.
     *    - Bicubic filtering is not supported. If SkSamplingOptions::useCubic is true, these
     *      factories will return nullptr.
     */
    sk_sp<SkShader> makeRawShader(SkTileMode tmx, SkTileMode tmy, const SkSamplingOptions&,
                                  const SkMatrix* localMatrix = nullptr) const;
    sk_sp<SkShader> makeRawShader(SkTileMode tmx, SkTileMode tmy, const SkSamplingOptions& sampling,
                                  const SkMatrix& lm) const;
    /** Defaults to clamp in both X and Y. */
    sk_sp<SkShader> makeRawShader(const SkSamplingOptions& sampling, const SkMatrix& lm) const;
    sk_sp<SkShader> makeRawShader(const SkSamplingOptions& sampling,
                                  const SkMatrix* lm = nullptr) const;

    /** Copies SkImage pixel address, row bytes, and SkImageInfo to pixmap, if address
        is available, and returns true. If pixel address is not available, return
        false and leave pixmap unchanged.

        @param pixmap  storage for pixel state if pixels are readable; otherwise, ignored
        @return        true if SkImage has direct access to pixels

        example: https://fiddle.skia.org/c/@Image_peekPixels
    */
    bool peekPixels(SkPixmap* pixmap) const;

    /** Returns true if the contents of SkImage was created on or uploaded to GPU memory,
        and is available as a GPU texture.

        @return  true if SkImage is a GPU texture

        example: https://fiddle.skia.org/c/@Image_isTextureBacked
    */
    bool isTextureBacked() const;

    /** Returns an approximation of the amount of texture memory used by the image. Returns
        zero if the image is not texture backed or if the texture has an external format.
     */
    size_t textureSize() const;

    /** Returns true if SkImage can be drawn on either raster surface or GPU surface.
        If context is nullptr, tests if SkImage draws on raster surface;
        otherwise, tests if SkImage draws on GPU surface associated with context.

        SkImage backed by GPU texture may become invalid if associated context is
        invalid. lazy image may be invalid and may not draw to raster surface or
        GPU surface or both.

        @param context  GPU context
        @return         true if SkImage can be drawn

        example: https://fiddle.skia.org/c/@Image_isValid
    */
    bool isValid(GrRecordingContext* context) const;

    /** \enum SkImage::CachingHint
        CachingHint selects whether Skia may internally cache SkBitmap generated by
        decoding SkImage, or by copying SkImage from GPU to CPU. The default behavior
        allows caching SkBitmap.

        Choose kDisallow_CachingHint if SkImage pixels are to be used only once, or
        if SkImage pixels reside in a cache outside of Skia, or to reduce memory pressure.

        Choosing kAllow_CachingHint does not ensure that pixels will be cached.
        SkImage pixels may not be cached if memory requirements are too large or
        pixels are not accessible.
    */
    enum CachingHint {
        kAllow_CachingHint,    //!< allows internally caching decoded and copied pixels
        kDisallow_CachingHint, //!< disallows internally caching decoded and copied pixels
    };

    /** Copies SkRect of pixels from SkImage to dstPixels. Copy starts at offset (srcX, srcY),
        and does not exceed SkImage (width(), height()).

        dstInfo specifies width, height, SkColorType, SkAlphaType, and SkColorSpace of
        destination. dstRowBytes specifies the gap from one destination row to the next.
        Returns true if pixels are copied. Returns false if:
        - dstInfo.addr() equals nullptr
        - dstRowBytes is less than dstInfo.minRowBytes()
        - SkPixelRef is nullptr

        Pixels are copied only if pixel conversion is possible. If SkImage SkColorType is
        kGray_8_SkColorType, or kAlpha_8_SkColorType; dstInfo.colorType() must match.
        If SkImage SkColorType is kGray_8_SkColorType, dstInfo.colorSpace() must match.
        If SkImage SkAlphaType is kOpaque_SkAlphaType, dstInfo.alphaType() must
        match. If SkImage SkColorSpace is nullptr, dstInfo.colorSpace() must match. Returns
        false if pixel conversion is not possible.

        srcX and srcY may be negative to copy only top or left of source. Returns
        false if width() or height() is zero or negative.
        Returns false if abs(srcX) >= Image width(), or if abs(srcY) >= Image height().

        If cachingHint is kAllow_CachingHint, pixels may be retained locally.
        If cachingHint is kDisallow_CachingHint, pixels are not added to the local cache.

        @param context      the GrDirectContext in play, if it exists
        @param dstInfo      destination width, height, SkColorType, SkAlphaType, SkColorSpace
        @param dstPixels    destination pixel storage
        @param dstRowBytes  destination row length
        @param srcX         column index whose absolute value is less than width()
        @param srcY         row index whose absolute value is less than height()
        @param cachingHint  whether the pixels should be cached locally
        @return             true if pixels are copied to dstPixels
    */
    bool readPixels(GrDirectContext* context,
                    const SkImageInfo& dstInfo,
                    void* dstPixels,
                    size_t dstRowBytes,
                    int srcX, int srcY,
                    CachingHint cachingHint = kAllow_CachingHint) const;

    /** Copies a SkRect of pixels from SkImage to dst. Copy starts at (srcX, srcY), and
        does not exceed SkImage (width(), height()).

        dst specifies width, height, SkColorType, SkAlphaType, SkColorSpace, pixel storage,
        and row bytes of destination. dst.rowBytes() specifics the gap from one destination
        row to the next. Returns true if pixels are copied. Returns false if:
        - dst pixel storage equals nullptr
        - dst.rowBytes is less than SkImageInfo::minRowBytes
        - SkPixelRef is nullptr

        Pixels are copied only if pixel conversion is possible. If SkImage SkColorType is
        kGray_8_SkColorType, or kAlpha_8_SkColorType; dst.colorType() must match.
        If SkImage SkColorType is kGray_8_SkColorType, dst.colorSpace() must match.
        If SkImage SkAlphaType is kOpaque_SkAlphaType, dst.alphaType() must
        match. If SkImage SkColorSpace is nullptr, dst.colorSpace() must match. Returns
        false if pixel conversion is not possible.

        srcX and srcY may be negative to copy only top or left of source. Returns
        false if width() or height() is zero or negative.
        Returns false if abs(srcX) >= Image width(), or if abs(srcY) >= Image height().

        If cachingHint is kAllow_CachingHint, pixels may be retained locally.
        If cachingHint is kDisallow_CachingHint, pixels are not added to the local cache.

        @param context      the GrDirectContext in play, if it exists
        @param dst          destination SkPixmap: SkImageInfo, pixels, row bytes
        @param srcX         column index whose absolute value is less than width()
        @param srcY         row index whose absolute value is less than height()
        @param cachingHint  whether the pixels should be cached locallyZ
        @return             true if pixels are copied to dst
    */
    bool readPixels(GrDirectContext* context,
                    const SkPixmap& dst,
                    int srcX,
                    int srcY,
                    CachingHint cachingHint = kAllow_CachingHint) const;

#ifndef SK_IMAGE_READ_PIXELS_DISABLE_LEGACY_API
    /** Deprecated. Use the variants that accept a GrDirectContext. */
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY, CachingHint cachingHint = kAllow_CachingHint) const;
    bool readPixels(const SkPixmap& dst, int srcX, int srcY,
                    CachingHint cachingHint = kAllow_CachingHint) const;
#endif

    /** The result from asyncRescaleAndReadPixels() or asyncRescaleAndReadPixelsYUV420(). */
    class AsyncReadResult {
    public:
        AsyncReadResult(const AsyncReadResult&) = delete;
        AsyncReadResult(AsyncReadResult&&) = delete;
        AsyncReadResult& operator=(const AsyncReadResult&) = delete;
        AsyncReadResult& operator=(AsyncReadResult&&) = delete;

        virtual ~AsyncReadResult() = default;
        virtual int count() const = 0;
        virtual const void* data(int i) const = 0;
        virtual size_t rowBytes(int i) const = 0;

    protected:
        AsyncReadResult() = default;
    };

    /** Client-provided context that is passed to client-provided ReadPixelsContext. */
    using ReadPixelsContext = void*;

    /**  Client-provided callback to asyncRescaleAndReadPixels() or
         asyncRescaleAndReadPixelsYUV420() that is called when read result is ready or on failure.
     */
    using ReadPixelsCallback = void(ReadPixelsContext, std::unique_ptr<const AsyncReadResult>);

    enum class RescaleGamma : bool { kSrc, kLinear };

    enum class RescaleMode {
        kNearest,
        kLinear,
        kRepeatedLinear,
        kRepeatedCubic,
    };

    /** Makes image pixel data available to caller, possibly asynchronously. It can also rescale
        the image pixels.

        Currently asynchronous reads are only supported on the GPU backend and only when the
        underlying 3D API supports transfer buffers and CPU/GPU synchronization primitives. In all
        other cases this operates synchronously.

        Data is read from the source sub-rectangle, is optionally converted to a linear gamma, is
        rescaled to the size indicated by 'info', is then converted to the color space, color type,
        and alpha type of 'info'. A 'srcRect' that is not contained by the bounds of the image
        causes failure.

        When the pixel data is ready the caller's ReadPixelsCallback is called with a
        AsyncReadResult containing pixel data in the requested color type, alpha type, and color
        space. The AsyncReadResult will have count() == 1. Upon failure the callback is called with
        nullptr for AsyncReadResult. For a GPU image this flushes work but a submit must occur to
        guarantee a finite time before the callback is called.

        The data is valid for the lifetime of AsyncReadResult with the exception that if the SkImage
        is GPU-backed the data is immediately invalidated if the context is abandoned or
        destroyed.

        @param info            info of the requested pixels
        @param srcRect         subrectangle of image to read
        @param rescaleGamma    controls whether rescaling is done in the image's gamma or whether
                               the source data is transformed to a linear gamma before rescaling.
        @param rescaleMode     controls the technique (and cost) of the rescaling
        @param callback        function to call with result of the read
        @param context         passed to callback
    */
    void asyncRescaleAndReadPixels(const SkImageInfo& info,
                                   const SkIRect& srcRect,
                                   RescaleGamma rescaleGamma,
                                   RescaleMode rescaleMode,
                                   ReadPixelsCallback callback,
                                   ReadPixelsContext context) const;

    /**
        Similar to asyncRescaleAndReadPixels but performs an additional conversion to YUV. The
        RGB->YUV conversion is controlled by 'yuvColorSpace'. The YUV data is returned as three
        planes ordered y, u, v. The u and v planes are half the width and height of the resized
        rectangle. The y, u, and v values are single bytes. Currently this fails if 'dstSize'
        width and height are not even. A 'srcRect' that is not contained by the bounds of the
        image causes failure.

        When the pixel data is ready the caller's ReadPixelsCallback is called with a
        AsyncReadResult containing the planar data. The AsyncReadResult will have count() == 3.
        Upon failure the callback is called with nullptr for AsyncReadResult. For a GPU image this
        flushes work but a submit must occur to guarantee a finite time before the callback is
        called.

        The data is valid for the lifetime of AsyncReadResult with the exception that if the SkImage
        is GPU-backed the data is immediately invalidated if the context is abandoned or
        destroyed.

        @param yuvColorSpace  The transformation from RGB to YUV. Applied to the resized image
                              after it is converted to dstColorSpace.
        @param dstColorSpace  The color space to convert the resized image to, after rescaling.
        @param srcRect        The portion of the image to rescale and convert to YUV planes.
        @param dstSize        The size to rescale srcRect to
        @param rescaleGamma   controls whether rescaling is done in the image's gamma or whether
                              the source data is transformed to a linear gamma before rescaling.
        @param rescaleMode    controls the technique (and cost) of the rescaling
        @param callback       function to call with the planar read result
        @param context        passed to callback
     */
    void asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                         sk_sp<SkColorSpace> dstColorSpace,
                                         const SkIRect& srcRect,
                                         const SkISize& dstSize,
                                         RescaleGamma rescaleGamma,
                                         RescaleMode rescaleMode,
                                         ReadPixelsCallback callback,
                                         ReadPixelsContext context) const;

    /** Copies SkImage to dst, scaling pixels to fit dst.width() and dst.height(), and
        converting pixels to match dst.colorType() and dst.alphaType(). Returns true if
        pixels are copied. Returns false if dst.addr() is nullptr, or dst.rowBytes() is
        less than dst SkImageInfo::minRowBytes.

        Pixels are copied only if pixel conversion is possible. If SkImage SkColorType is
        kGray_8_SkColorType, or kAlpha_8_SkColorType; dst.colorType() must match.
        If SkImage SkColorType is kGray_8_SkColorType, dst.colorSpace() must match.
        If SkImage SkAlphaType is kOpaque_SkAlphaType, dst.alphaType() must
        match. If SkImage SkColorSpace is nullptr, dst.colorSpace() must match. Returns
        false if pixel conversion is not possible.

        If cachingHint is kAllow_CachingHint, pixels may be retained locally.
        If cachingHint is kDisallow_CachingHint, pixels are not added to the local cache.

        @param dst            destination SkPixmap: SkImageInfo, pixels, row bytes
        @return               true if pixels are scaled to fit dst
    */
    bool scalePixels(const SkPixmap& dst, const SkSamplingOptions&,
                     CachingHint cachingHint = kAllow_CachingHint) const;

    /** Encodes SkImage pixels, returning result as SkData.

        Returns nullptr if encoding fails, or if encodedImageFormat is not supported.

        SkImage encoding in a format requires both building with one or more of:
        SK_ENCODE_JPEG, SK_ENCODE_PNG, SK_ENCODE_WEBP; and platform support
        for the encoded format.

        If SK_BUILD_FOR_MAC or SK_BUILD_FOR_IOS is defined, encodedImageFormat can
        additionally be one of: SkEncodedImageFormat::kICO, SkEncodedImageFormat::kBMP,
        SkEncodedImageFormat::kGIF.

        quality is a platform and format specific metric trading off size and encoding
        error. When used, quality equaling 100 encodes with the least error. quality may
        be ignored by the encoder.

        @param encodedImageFormat  one of: SkEncodedImageFormat::kJPEG, SkEncodedImageFormat::kPNG,
                                   SkEncodedImageFormat::kWEBP
        @param quality             encoder specific metric with 100 equaling best
        @return                    encoded SkImage, or nullptr

        example: https://fiddle.skia.org/c/@Image_encodeToData
    */
    sk_sp<SkData> encodeToData(SkEncodedImageFormat encodedImageFormat, int quality) const;

    /** Encodes SkImage pixels, returning result as SkData. Returns existing encoded data
        if present; otherwise, SkImage is encoded with SkEncodedImageFormat::kPNG. Skia
        must be built with SK_ENCODE_PNG to encode SkImage.

        Returns nullptr if existing encoded data is missing or invalid, and
        encoding fails.

        @return  encoded SkImage, or nullptr

        example: https://fiddle.skia.org/c/@Image_encodeToData_2
    */
    sk_sp<SkData> encodeToData() const;

    /** Returns encoded SkImage pixels as SkData, if SkImage was created from supported
        encoded stream format. Platform support for formats vary and may require building
        with one or more of: SK_ENCODE_JPEG, SK_ENCODE_PNG, SK_ENCODE_WEBP.

        Returns nullptr if SkImage contents are not encoded.

        @return  encoded SkImage, or nullptr

        example: https://fiddle.skia.org/c/@Image_refEncodedData
    */
    sk_sp<SkData> refEncodedData() const;

    /** Returns subset of this image.

        Returns nullptr if any of the following are true:
          - Subset is empty
          - Subset is not contained inside the image's bounds
          - Pixels in the image could not be read or copied

        If this image is texture-backed, the context parameter is required and must match the
        context of the source image. If the context parameter is provided, and the image is
        raster-backed, the subset will be converted to texture-backed.

        @param subset  bounds of returned SkImage
        @param context the GrDirectContext in play, if it exists
        @return        the subsetted image, or nullptr

        example: https://fiddle.skia.org/c/@Image_makeSubset
    */
    sk_sp<SkImage> makeSubset(const SkIRect& subset, GrDirectContext* direct = nullptr) const;

    /**
     *  Returns true if the image has mipmap levels.
     */
    bool hasMipmaps() const;

    /**
     *  Returns an image with the same "base" pixels as the this image, but with mipmap levels
     *  automatically generated and attached.
     */
    sk_sp<SkImage> withDefaultMipmaps() const;

    /** Returns raster image or lazy image. Copies SkImage backed by GPU texture into
        CPU memory if needed. Returns original SkImage if decoded in raster bitmap,
        or if encoded in a stream.

        Returns nullptr if backed by GPU texture and copy fails.

        @return  raster image, lazy image, or nullptr

        example: https://fiddle.skia.org/c/@Image_makeNonTextureImage
    */
    sk_sp<SkImage> makeNonTextureImage() const;

    /** Returns raster image. Copies SkImage backed by GPU texture into CPU memory,
        or decodes SkImage from lazy image. Returns original SkImage if decoded in
        raster bitmap.

        Returns nullptr if copy, decode, or pixel read fails.

        If cachingHint is kAllow_CachingHint, pixels may be retained locally.
        If cachingHint is kDisallow_CachingHint, pixels are not added to the local cache.

        @return  raster image, or nullptr

        example: https://fiddle.skia.org/c/@Image_makeRasterImage
    */
    sk_sp<SkImage> makeRasterImage(CachingHint cachingHint = kDisallow_CachingHint) const;

    /** Creates filtered SkImage. filter processes original SkImage, potentially changing
        color, position, and size. subset is the bounds of original SkImage processed
        by filter. clipBounds is the expected bounds of the filtered SkImage. outSubset
        is required storage for the actual bounds of the filtered SkImage. offset is
        required storage for translation of returned SkImage.

        Returns nullptr if SkImage could not be created or if the recording context provided doesn't
        match the GPU context in which the image was created. If nullptr is returned, outSubset
        and offset are undefined.

        Useful for animation of SkImageFilter that varies size from frame to frame.
        Returned SkImage is created larger than required by filter so that GPU texture
        can be reused with different sized effects. outSubset describes the valid bounds
        of GPU texture returned. offset translates the returned SkImage to keep subsequent
        animation frames aligned with respect to each other.

        @param context     the GrRecordingContext in play - if it exists
        @param filter      how SkImage is sampled when transformed
        @param subset      bounds of SkImage processed by filter
        @param clipBounds  expected bounds of filtered SkImage
        @param outSubset   storage for returned SkImage bounds
        @param offset      storage for returned SkImage translation
        @return            filtered SkImage, or nullptr
    */
    sk_sp<SkImage> makeWithFilter(GrRecordingContext* context,
                                  const SkImageFilter* filter, const SkIRect& subset,
                                  const SkIRect& clipBounds, SkIRect* outSubset,
                                  SkIPoint* offset) const;

    /** Deprecated.
     */
    enum LegacyBitmapMode {
        kRO_LegacyBitmapMode, //!< returned bitmap is read-only and immutable
    };

    /** Deprecated.
        Creates raster SkBitmap with same pixels as SkImage. If legacyBitmapMode is
        kRO_LegacyBitmapMode, returned bitmap is read-only and immutable.
        Returns true if SkBitmap is stored in bitmap. Returns false and resets bitmap if
        SkBitmap write did not succeed.

        @param bitmap            storage for legacy SkBitmap
        @param legacyBitmapMode  bitmap is read-only and immutable
        @return                  true if SkBitmap was created
    */
    bool asLegacyBitmap(SkBitmap* bitmap,
                        LegacyBitmapMode legacyBitmapMode = kRO_LegacyBitmapMode) const;

    /** Returns true if SkImage is backed by an image-generator or other service that creates
        and caches its pixels or texture on-demand.

        @return  true if SkImage is created as needed

        example: https://fiddle.skia.org/c/@Image_isLazyGenerated_a
        example: https://fiddle.skia.org/c/@Image_isLazyGenerated_b
    */
    bool isLazyGenerated() const;

    /** Creates SkImage in target SkColorSpace.
        Returns nullptr if SkImage could not be created.

        Returns original SkImage if it is in target SkColorSpace.
        Otherwise, converts pixels from SkImage SkColorSpace to target SkColorSpace.
        If SkImage colorSpace() returns nullptr, SkImage SkColorSpace is assumed to be sRGB.

        If this image is texture-backed, the context parameter is required and must match the
        context of the source image.

        @param target  SkColorSpace describing color range of returned SkImage
        @param direct  The GrDirectContext in play, if it exists
        @return        created SkImage in target SkColorSpace

        example: https://fiddle.skia.org/c/@Image_makeColorSpace
    */
    sk_sp<SkImage> makeColorSpace(sk_sp<SkColorSpace> target,
                                  GrDirectContext* direct = nullptr) const;

    /** Experimental.
        Creates SkImage in target SkColorType and SkColorSpace.
        Returns nullptr if SkImage could not be created.

        Returns original SkImage if it is in target SkColorType and SkColorSpace.

        If this image is texture-backed, the context parameter is required and must match the
        context of the source image.

        @param targetColorType  SkColorType of returned SkImage
        @param targetColorSpace SkColorSpace of returned SkImage
        @param direct           The GrDirectContext in play, if it exists
        @return                 created SkImage in target SkColorType and SkColorSpace
    */
    sk_sp<SkImage> makeColorTypeAndColorSpace(SkColorType targetColorType,
                                              sk_sp<SkColorSpace> targetColorSpace,
                                              GrDirectContext* direct = nullptr) const;

    /** Creates a new SkImage identical to this one, but with a different SkColorSpace.
        This does not convert the underlying pixel data, so the resulting image will draw
        differently.
    */
    sk_sp<SkImage> reinterpretColorSpace(sk_sp<SkColorSpace> newColorSpace) const;

    // ===============================================================================
    // Methods below here are no-op for non-gpu backed images

    /** Returns SkImage backed by GPU texture associated with context. Returned SkImage is
        compatible with SkSurface created with dstColorSpace. The returned SkImage respects
        mipmapped setting; if mipmapped equals skgpu::Mipmapped::kYes, the backing texture
        allocates mip map levels.
        The mipmapped parameter is effectively treated as kNo if MIP maps are not supported by the
        GPU.
        Returns original SkImage if the image is already texture-backed, the context matches, and
        mipmapped is compatible with the backing GPU texture. skgpu::Budgeted is ignored in this
       case.
        Returns nullptr if context is nullptr, or if SkImage was created with another
        GrDirectContext.
        @param GrDirectContext the GrDirectContext in play, if it exists
        @param skgpu::Mipmapped Whether created SkImage texture must allocate mip map levels.
                                Defaults to no.
        @param skgpu::Budgeted Whether to count a newly created texture for the returned image
                               counts against the context's budget. Defaults to yes.
        @return                created SkImage, or nullptr
    */
    sk_sp<SkImage> makeTextureImage(GrDirectContext*, skgpu::Mipmapped, skgpu::Budgeted) const;
    sk_sp<SkImage> makeTextureImage(GrDirectContext*, skgpu::Mipmapped) const;
    sk_sp<SkImage> makeTextureImage(GrDirectContext*) const;

     /** Flushes any pending uses of texture-backed images in the GPU backend. If the image is not
        texture-backed (including promise texture images) or if the GrDirectContext does not
        have the same context ID as the context backing the image then this is a no-op.
        If the image was not used in any non-culled draws in the current queue of work for the
        passed GrDirectContext then this is a no-op unless the GrFlushInfo contains semaphores or
        a finish proc. Those are respected even when the image has not been used.
        @param context  the context on which to flush pending usages of the image.
        @param info     flush options
     */
    GrSemaphoresSubmitted flush(GrDirectContext* context, const GrFlushInfo& flushInfo) const;

    void flush(GrDirectContext* context) const;

    /** Version of flush() that uses a default GrFlushInfo. Also submits the flushed work to the
        GPU.
    */
    void flushAndSubmit(GrDirectContext*) const;

    /** Retrieves the back-end texture. If SkImage has no back-end texture, an invalid
        object is returned. Call GrBackendTexture::isValid to determine if the result
        is valid.
        If flushPendingGrContextIO is true, completes deferred I/O operations.
        If origin in not nullptr, copies location of content drawn into SkImage.
        @param flushPendingGrContextIO  flag to flush outstanding requests
        @return                         back-end API texture handle; invalid on failure
    */
    GrBackendTexture getBackendTexture(bool flushPendingGrContextIO,
                                       GrSurfaceOrigin* origin = nullptr) const;

private:
    SkImage(const SkImageInfo& info, uint32_t uniqueID);

    friend class SkBitmap;
    friend class SkImage_Base;   // for private ctor
    friend class SkImage_Raster; // for withMipmaps
    friend class SkMipmapBuilder;

    SkImageInfo     fInfo;
    const uint32_t  fUniqueID;

    sk_sp<SkImage> withMipmaps(sk_sp<SkMipmap>) const;

    using INHERITED = SkRefCnt;
};

namespace sk_image_factory {

/** Caller data passed to RasterReleaseProc; may be nullptr.
*/
using ReleaseContext = void*;

/** Creates SkImage from SkPixmap and copy of pixels. Since pixels are copied, SkPixmap
    pixels may be modified or deleted without affecting SkImage.

    SkImage is returned if SkPixmap is valid. Valid SkPixmap parameters include:
    dimensions are greater than zero;
    each dimension fits in 29 bits;
    SkColorType and SkAlphaType are valid, and SkColorType is not kUnknown_SkColorType;
    row bytes are large enough to hold one row of pixels;
    pixel address is not nullptr.

    @param pixmap  SkImageInfo, pixel address, and row bytes
    @return        copy of SkPixmap pixels, or nullptr

    example: https://fiddle.skia.org/c/@Image_MakeRasterCopy
*/
sk_sp<SkImage> MakeRasterCopy(const SkPixmap& pixmap);

/** Creates SkImage from SkImageInfo, sharing pixels.

    SkImage is returned if SkImageInfo is valid. Valid SkImageInfo parameters include:
    dimensions are greater than zero;
    each dimension fits in 29 bits;
    SkColorType and SkAlphaType are valid, and SkColorType is not kUnknown_SkColorType;
    rowBytes are large enough to hold one row of pixels;
    pixels is not nullptr, and contains enough data for SkImage.

    @param info      contains width, height, SkAlphaType, SkColorType, SkColorSpace
    @param pixels    address or pixel storage
    @param rowBytes  size of pixel row or larger
    @return          SkImage sharing pixels, or nullptr
*/
sk_sp<SkImage> MakeRasterData(const SkImageInfo& info, sk_sp<SkData> pixels,
                                     size_t rowBytes);

/** Function called when SkImage no longer shares pixels. ReleaseContext is
    provided by caller when SkImage is created, and may be nullptr.
*/
typedef void (*RasterReleaseProc)(const void* pixels, ReleaseContext);

/** Creates SkImage from pixmap, sharing SkPixmap pixels. Pixels must remain valid and
    unchanged until rasterReleaseProc is called. rasterReleaseProc is passed
    releaseContext when SkImage is deleted or no longer refers to pixmap pixels.

    Pass nullptr for rasterReleaseProc to share SkPixmap without requiring a callback
    when SkImage is released. Pass nullptr for releaseContext if rasterReleaseProc
    does not require state.

    SkImage is returned if pixmap is valid. Valid SkPixmap parameters include:
    dimensions are greater than zero;
    each dimension fits in 29 bits;
    SkColorType and SkAlphaType are valid, and SkColorType is not kUnknown_SkColorType;
    row bytes are large enough to hold one row of pixels;
    pixel address is not nullptr.

    @param pixmap             SkImageInfo, pixel address, and row bytes
    @param rasterReleaseProc  function called when pixels can be released; or nullptr
    @param releaseContext     state passed to rasterReleaseProc; or nullptr
    @return                   SkImage sharing pixmap
*/
sk_sp<SkImage> MakeFromRaster(const SkPixmap& pixmap,
                                     RasterReleaseProc rasterReleaseProc,
                                     ReleaseContext releaseContext);

/** Creates SkImage from bitmap, sharing or copying bitmap pixels. If the bitmap
    is marked immutable, and its pixel memory is shareable, it may be shared
    instead of copied.

    SkImage is returned if bitmap is valid. Valid SkBitmap parameters include:
    dimensions are greater than zero;
    each dimension fits in 29 bits;
    SkColorType and SkAlphaType are valid, and SkColorType is not kUnknown_SkColorType;
    row bytes are large enough to hold one row of pixels;
    pixel address is not nullptr.

    @param bitmap  SkImageInfo, row bytes, and pixels
    @return        created SkImage, or nullptr

    example: https://fiddle.skia.org/c/@Image_MakeFromBitmap
*/
sk_sp<SkImage> MakeFromBitmap(const SkBitmap& bitmap);

/** Creates SkImage from data returned by imageGenerator. Generated data is owned by SkImage and
    may not be shared or accessed.

    SkImage is returned if generator data is valid. Valid data parameters vary by type of data
    and platform.

    imageGenerator may wrap SkPicture data, codec data, or custom data.

    @param imageGenerator  stock or custom routines to retrieve SkImage
    @return                created SkImage, or nullptr
*/
sk_sp<SkImage> MakeFromGenerator(std::unique_ptr<SkImageGenerator> imageGenerator);

/**
 *  Return an image backed by the encoded data, but attempt to defer decoding until the image
 *  is actually used/drawn. This deferral allows the system to cache the result, either on the
 *  CPU or on the GPU, depending on where the image is drawn. If memory is low, the cache may
 *  be purged, causing the next draw of the image to have to re-decode.
 *
 *  If alphaType is nullopt, the image's alpha type will be chosen automatically based on the
 *  image format. Transparent images will default to kPremul_SkAlphaType. If alphaType contains
 *  kPremul_SkAlphaType or kUnpremul_SkAlphaType, that alpha type will be used. Forcing opaque
 *  (passing kOpaque_SkAlphaType) is not allowed, and will return nullptr.
 *
 *  This is similar to DecodeTo[Raster,Texture], but this method will attempt to defer the
 *  actual decode, while the DecodeTo... method explicitly decode and allocate the backend
 *  when the call is made.
 *
 *  If the encoded format is not supported, nullptr is returned.
 *
 *  @param encoded  the encoded data
 *  @return         created SkImage, or nullptr

    example: https://fiddle.skia.org/c/@Image_MakeFromEncoded
*/
sk_sp<SkImage> MakeFromEncoded(sk_sp<SkData> encoded,
                                      std::optional<SkAlphaType> alphaType = std::nullopt);

/*
 * Experimental:
 *   Skia                | GL_COMPRESSED_*     | MTLPixelFormat*      | VK_FORMAT_*_BLOCK
 *  --------------------------------------------------------------------------------------
 *   kETC2_RGB8_UNORM    | ETC1_RGB8           | ETC2_RGB8 (iOS-only) | ETC2_R8G8B8_UNORM
 *                       | RGB8_ETC2           |                      |
 *  --------------------------------------------------------------------------------------
 *   kBC1_RGB8_UNORM     | RGB_S3TC_DXT1_EXT   | N/A                  | BC1_RGB_UNORM
 *  --------------------------------------------------------------------------------------
 *   kBC1_RGBA8_UNORM    | RGBA_S3TC_DXT1_EXT  | BC1_RGBA (macOS-only)| BC1_RGBA_UNORM
 */
enum class CompressionType {
    kNone,
    kETC2_RGB8_UNORM, // the same as ETC1

    kBC1_RGB8_UNORM,
    kBC1_RGBA8_UNORM,
    kLast = kBC1_RGBA8_UNORM,
};

constexpr int kCompressionTypeCount = static_cast<int>(CompressionType::kLast) + 1;

const CompressionType kETC1_CompressionType = CompressionType::kETC2_RGB8_UNORM;

/** Creates a CPU-backed SkImage from compressed data.

    This method will decompress the compressed data and create an image wrapping
    it. Any mipmap levels present in the compressed data are discarded.

    @param data     compressed data to store in SkImage
    @param width    width of full SkImage
    @param height   height of full SkImage
    @param type     type of compression used
    @return         created SkImage, or nullptr
*/
sk_sp<SkImage> MakeRasterFromCompressed(sk_sp<SkData> data,
                                               int width, int height,
                                               CompressionType type);

enum class BitDepth {
    kU8,  //!< uses 8-bit unsigned int per color component
    kF16, //!< uses 16-bit float per color component
};

/** Creates SkImage from picture. Returned SkImage width and height are set by dimensions.
    SkImage draws picture with matrix and paint, set to bitDepth and colorSpace.

    If matrix is nullptr, draws with identity SkMatrix. If paint is nullptr, draws
    with default SkPaint. colorSpace may be nullptr.

    @param picture     stream of drawing commands
    @param dimensions  width and height
    @param matrix      SkMatrix to rotate, scale, translate, and so on; may be nullptr
    @param paint       SkPaint to apply transparency, filtering, and so on; may be nullptr
    @param bitDepth    8-bit integer or 16-bit float: per component
    @param colorSpace  range of colors; may be nullptr
    @param props       props to use when rasterizing the picture
    @return            created SkImage, or nullptr
*/
sk_sp<SkImage> MakeFromPicture(sk_sp<SkPicture> picture, const SkISize& dimensions,
                               const SkMatrix* matrix, const SkPaint* paint,
                               BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace,
                               SkSurfaceProps props);
sk_sp<SkImage> MakeFromPicture(sk_sp<SkPicture> picture, const SkISize& dimensions,
                               const SkMatrix* matrix, const SkPaint* paint,
                               BitDepth bitDepth, sk_sp<SkColorSpace> colorSpace);
}

#endif
