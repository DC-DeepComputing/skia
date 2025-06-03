/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMaskBlurFilter_DEFINED
#define SkMaskBlurFilter_DEFINED

#include <algorithm>
#include <memory>
#include <tuple>

#include "include/core/SkTypes.h"
#include "src/core/SkMask.h"

using ToA8 = void (uint8_t* a8, const uint8_t* from, int width);

class PlanGauss final {
  public:
      explicit PlanGauss(double sigma);

      size_t bufferSize() const;
      int    border() const;

      class Scan {
      public:
          Scan(uint64_t weight, int noChangeCount,
               uint32_t* buffer0, uint32_t* buffer0End,
               uint32_t* buffer1, uint32_t* buffer1End,
               uint32_t* buffer2, uint32_t* buffer2End)
              : fWeight{weight}
              , fNoChangeCount{noChangeCount}
              , fBuffer0{buffer0}
              , fBuffer0End{buffer0End}
              , fBuffer1{buffer1}
              , fBuffer1End{buffer1End}
              , fBuffer2{buffer2}
              , fBuffer2End{buffer2End}
          {}

          template <typename AlphaIter> void blur(const AlphaIter srcBegin, const AlphaIter srcEnd,
                                                  uint8_t* dst, int dstStride, uint8_t* dstEnd) const {
              auto buffer0Cursor = fBuffer0;
              auto buffer1Cursor = fBuffer1;
              auto buffer2Cursor = fBuffer2;

              std::memset(fBuffer0, 0x00, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

              uint32_t sum0 = 0;
              uint32_t sum1 = 0;
              uint32_t sum2 = 0;

              // Consume the source generating pixels.
              for (AlphaIter src = srcBegin; src < srcEnd; ++src, dst += dstStride) {
                  uint32_t leadingEdge = *src;
                  sum0 += leadingEdge;
                  sum1 += sum0;
                  sum2 += sum1;

                  *dst = this->finalScale(sum2);

                  sum2 -= *buffer2Cursor;
                  *buffer2Cursor = sum1;
                  buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

                  sum1 -= *buffer1Cursor;
                  *buffer1Cursor = sum0;
                  buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

                  sum0 -= *buffer0Cursor;
                  *buffer0Cursor = leadingEdge;
                  buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
              }

              // The leading edge is off the right side of the mask.
              for (int i = 0; i < fNoChangeCount; i++) {
                  uint32_t leadingEdge = 0;
                  sum0 += leadingEdge;
                  sum1 += sum0;
                  sum2 += sum1;

                  *dst = this->finalScale(sum2);

                  sum2 -= *buffer2Cursor;
                  *buffer2Cursor = sum1;
                  buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

                  sum1 -= *buffer1Cursor;
                  *buffer1Cursor = sum0;
                  buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

                  sum0 -= *buffer0Cursor;
                  *buffer0Cursor = leadingEdge;
                  buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;

                  dst += dstStride;
              }

              // Starting from the right, fill in the rest of the buffer.
              std::memset(fBuffer0, 0, (fBuffer2End - fBuffer0) * sizeof(*fBuffer0));

              sum0 = sum1 = sum2 = 0;

              uint8_t* dstCursor = dstEnd;
              AlphaIter src = srcEnd;
              while (dstCursor > dst) {
                  dstCursor -= dstStride;
                  uint32_t leadingEdge = *(--src);
                  sum0 += leadingEdge;
                  sum1 += sum0;
                  sum2 += sum1;

                  *dstCursor = this->finalScale(sum2);

                  sum2 -= *buffer2Cursor;
                  *buffer2Cursor = sum1;
                  buffer2Cursor = (buffer2Cursor + 1) < fBuffer2End ? buffer2Cursor + 1 : fBuffer2;

                  sum1 -= *buffer1Cursor;
                  *buffer1Cursor = sum0;
                  buffer1Cursor = (buffer1Cursor + 1) < fBuffer1End ? buffer1Cursor + 1 : fBuffer1;

                  sum0 -= *buffer0Cursor;
                  *buffer0Cursor = leadingEdge;
                  buffer0Cursor = (buffer0Cursor + 1) < fBuffer0End ? buffer0Cursor + 1 : fBuffer0;
              }
            }

      private:
          inline static constexpr uint64_t kHalf = static_cast<uint64_t>(1) << 31;
          uint8_t finalScale(uint32_t sum) const;

          uint64_t  fWeight;
          int       fNoChangeCount;
          uint32_t* fBuffer0;
          uint32_t* fBuffer0End;
          uint32_t* fBuffer1;
          uint32_t* fBuffer1End;
          uint32_t* fBuffer2;
          uint32_t* fBuffer2End;
      };

      Scan makeBlurScan(int width, uint32_t* buffer) const;

  private:
      uint64_t fWeight;
      int      fBorder;
      int      fSlidingWindow;
      int      fPass0Size;
      int      fPass1Size;
      int      fPass2Size;
};


// Implement a single channel Gaussian blur. The specifics for implementation are taken from:
// https://drafts.fxtf.org/filters/#feGaussianBlurElement
class SkMaskBlurFilter {
public:
    // Create an object suitable for filtering an SkMask using a filter with width sigmaW and
    // height sigmaH.
    SkMaskBlurFilter(double sigmaW, double sigmaH);

    // returns true iff the sigmas will result in an identity mask (no blurring)
    bool hasNoBlur() const;

    // Given a src SkMask, generate dst SkMask returning the border width and height.
    SkIPoint blur(const SkMask& src, SkMaskBuilder* dst) const;

    static void DirectBlurY(ToA8 toA8, const int strideOf8,
                            int radius, uint16_t* gauss,
                            const uint8_t* src, size_t srcRB, int srcW, int srcH,
                            uint8_t* dst, size_t dstRB);

    static void DirectBlurX(int radius, uint16_t* gauss,
                            const uint8_t* src, size_t srcStride, int srcW,
                            uint8_t* dst, size_t dstStride, int dstW, int dstH);

private:
    const double fSigmaW;
    const double fSigmaH;
};

#endif  // SkBlurMaskFilter_DEFINED
