/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkFontMgr_empty.h"
#include "include/private/SkTDArray.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkGeometry.h"
#include "src/sfnt/SkOTTable_glyf.h"
#include "src/sfnt/SkOTTable_head.h"
#include "src/sfnt/SkOTTable_hhea.h"
#include "src/sfnt/SkOTTable_hmtx.h"
#include "src/sfnt/SkOTTable_loca.h"
#include "src/sfnt/SkOTTable_maxp.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"

namespace {

constexpr SkScalar DX = 20;
constexpr SkScalar DY = 0;
constexpr SkScalar kS = 50;
constexpr SkScalar kT = 40;
constexpr SkGlyphID kGlyphID = 2;
constexpr int kPointSize = 5;

struct SBIXView : public Sample {
    SkPoint  fPts[12] = {
        {kS * 0, kT * 1},
        {kS * 1, kT * 1},
        {kS * 2, kT * 1},
        {kS * 3, kT * 1},
        {kS * 3, kT * 2},
        {kS * 3, kT * 3},
        {kS * 3, kT * 4},
        {kS * 2, kT * 4},
        {kS * 1, kT * 4},
        {kS * 0, kT * 4},
        {kS * 0, kT * 3},
        {kS * 0, kT * 2},
    };

    std::vector<sk_sp<SkFontMgr>> fFontMgr;
    std::vector<SkFont> fFonts;
    sk_sp<SkData> fSBIXData;
    bool fInputChanged = false;
    bool fDirty = true;

    void onOnceBeforeDraw() override {
        fFontMgr.emplace_back(SkFontMgr::RefDefault());
        fFontMgr.emplace_back(SkFontMgr_New_Custom_Empty());
        fSBIXData = GetResourceAsData("fonts/sbix.ttf");
        // GetResourceAsData may be backed by a read only file mapping.
        fSBIXData = SkData::MakeWithCopy(fSBIXData->data(), fSBIXData->size());
        this->setBGColor(SK_ColorGRAY);
    }

    SkString name() override { return SkString("SBIX"); }

    sk_sp<SkData> updateSBIXData(SkData* data) {
        // Lots of unlikely to be aligned pointers in here, which is UB. Total hack.

        sk_sp<SkData> dataCopy;
        dataCopy = SkData::MakeWithCopy(data->data(), data->size());
        data = dataCopy.get();

        SkSFNTHeader* sfntHeader = static_cast<SkSFNTHeader*>(data->writable_data());

        SkSFNTHeader::TableDirectoryEntry* tableEntry =
            SkTAfter<SkSFNTHeader::TableDirectoryEntry>(sfntHeader);
        SkSFNTHeader::TableDirectoryEntry* glyfTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* headTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* hheaTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* hmtxTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* locaTableEntry = nullptr;
        SkSFNTHeader::TableDirectoryEntry* maxpTableEntry = nullptr;
        int numTables = SkEndian_SwapBE16(sfntHeader->numTables);
        for (int tableEntryIndex = 0; tableEntryIndex < numTables; ++tableEntryIndex) {
            if (SkOTTableGlyph::TAG == tableEntry[tableEntryIndex].tag) {
                glyfTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableHead::TAG == tableEntry[tableEntryIndex].tag) {
                headTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableHorizontalHeader::TAG == tableEntry[tableEntryIndex].tag) {
                hheaTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableHorizontalMetrics::TAG == tableEntry[tableEntryIndex].tag) {
                hmtxTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableIndexToLocation::TAG == tableEntry[tableEntryIndex].tag) {
                locaTableEntry = tableEntry + tableEntryIndex;
            }
            if (SkOTTableMaximumProfile::TAG == tableEntry[tableEntryIndex].tag) {
                maxpTableEntry = tableEntry + tableEntryIndex;
            }
        }
        SkASSERT_RELEASE(glyfTableEntry);
        SkASSERT_RELEASE(headTableEntry);
        SkASSERT_RELEASE(hheaTableEntry);
        SkASSERT_RELEASE(hmtxTableEntry);
        SkASSERT_RELEASE(locaTableEntry);
        SkASSERT_RELEASE(maxpTableEntry);

        size_t glyfTableOffset = SkEndian_SwapBE32(glyfTableEntry->offset);
        SkOTTableGlyph* glyfTable = SkTAddOffset<SkOTTableGlyph>(sfntHeader, glyfTableOffset);

        size_t headTableOffset = SkEndian_SwapBE32(locaTableEntry->offset);
        SkOTTableHead* headTable = SkTAddOffset<SkOTTableHead>(sfntHeader, headTableOffset);

        size_t hheaTableOffset = SkEndian_SwapBE32(hheaTableEntry->offset);
        SkOTTableHorizontalHeader* hheaTable = SkTAddOffset<SkOTTableHorizontalHeader>(sfntHeader, hheaTableOffset);

        size_t hmtxTableOffset = SkEndian_SwapBE32(hmtxTableEntry->offset);
        SkOTTableHorizontalMetrics* hmtxTable = SkTAddOffset<SkOTTableHorizontalMetrics>(sfntHeader, hmtxTableOffset);

        size_t locaTableOffset = SkEndian_SwapBE32(locaTableEntry->offset);
        SkOTTableIndexToLocation* locaTable = SkTAddOffset<SkOTTableIndexToLocation>(sfntHeader, locaTableOffset);

        size_t maxpTableOffset = SkEndian_SwapBE32(maxpTableEntry->offset);
        SkOTTableMaximumProfile* maxpTable = SkTAddOffset<SkOTTableMaximumProfile>(sfntHeader, maxpTableOffset);

        SkASSERT_RELEASE(SkEndian_SwapBE32(maxpTable->version.version) == 0x00010000);
        int numGlyphs = maxpTable->version.tt.numGlyphs;
        SkASSERT_RELEASE(kGlyphID < numGlyphs);

        SkOTTableGlyph::Iterator glyphIter(*glyfTable, *locaTable, headTable->indexToLocFormat);
        glyphIter.advance(kGlyphID);
        SkOTTableGlyphData* glyphData = glyphIter.next();
        if (glyphData) {
            glyphData->xMin = SkEndian_SwapBE16(fPts[0].x());
            glyphData->yMin = SkEndian_SwapBE16(fPts[0].y());
            glyphData->xMax = SkEndian_SwapBE16(fPts[1].x());
            glyphData->yMax = SkEndian_SwapBE16(fPts[1].y());

            int contourCount = SkEndian_SwapBE16(glyphData->numberOfContours);
            if (contourCount > 0) {
                SK_OT_USHORT* endPtsOfContours = SkTAfter<SK_OT_USHORT>(glyphData);
                SK_OT_USHORT* numInstructions = SkTAfter<SK_OT_USHORT>(endPtsOfContours, contourCount);
                SK_OT_BYTE* instructions = SkTAfter<SK_OT_BYTE>(numInstructions);
                SkOTTableGlyphData::Simple::Flags* flags = SkTAfter<SkOTTableGlyphData::Simple::Flags>(instructions, SkEndian_SwapBE16(*numInstructions));

                int numResultPoints = SkEndian_SwapBE16(endPtsOfContours[contourCount-1]) + 1;
                struct Coordinate {
                    SkOTTableGlyphData::Simple::Flags* flags;
                    size_t offsetToXDelta;
                    size_t xDeltaSize;
                    size_t offsetToYDelta;
                    size_t yDeltaSize;
                };
                std::vector<Coordinate> coordinates(numResultPoints);

                size_t offsetToXDelta = 0;
                size_t offsetToYDelta = 0;
                SkOTTableGlyphData::Simple::Flags* currentFlags = flags;
                for (int i = 0; i < numResultPoints; ++i) {
                    SkOTTableGlyphData::Simple::Flags* nextFlags;
                    int times = 1;
                    if (currentFlags->field.Repeat) {
                        SK_OT_BYTE* repeat = SkTAfter<SK_OT_BYTE>(currentFlags);
                        times += *repeat;
                        nextFlags = SkTAfter<SkOTTableGlyphData::Simple::Flags>(repeat);
                    } else {
                        nextFlags = SkTAfter<SkOTTableGlyphData::Simple::Flags>(currentFlags);
                    }

                    --i;
                    for (int time = 0; time < times; ++time) {
                        ++i;
                        coordinates[i].flags = currentFlags;
                        coordinates[i].offsetToXDelta = offsetToXDelta;
                        coordinates[i].offsetToYDelta = offsetToYDelta;

                        if (currentFlags->field.xShortVector) {
                            offsetToXDelta += 1;
                            coordinates[i].xDeltaSize = 1;
                        } else if (currentFlags->field.xIsSame_xShortVectorPositive) {
                            offsetToXDelta += 0;
                            if (i == 0) {
                                coordinates[i].xDeltaSize = 0;
                            } else {
                                coordinates[i].xDeltaSize = coordinates[i-1].xDeltaSize;
                            }
                        } else {
                            offsetToXDelta += 2;
                            coordinates[i].xDeltaSize = 2;
                        }

                        if (currentFlags->field.yShortVector) {
                            offsetToYDelta += 1;
                            coordinates[i].yDeltaSize = 1;
                        } else if (currentFlags->field.yIsSame_yShortVectorPositive) {
                            offsetToYDelta += 0;
                            if (i == 0) {
                                coordinates[i].yDeltaSize = 0;
                            } else {
                                coordinates[i].yDeltaSize = coordinates[i-1].yDeltaSize;
                            }
                        } else {
                            offsetToYDelta += 2;
                            coordinates[i].yDeltaSize = 2;
                        }
                    }
                    currentFlags = nextFlags;
                }
                SK_OT_BYTE* xCoordinates = reinterpret_cast<SK_OT_BYTE*>(currentFlags);
                SK_OT_BYTE* yCoordinates = xCoordinates + offsetToXDelta;

                int pointIndex = 1;
                if (coordinates[pointIndex].xDeltaSize == 0) {
                    // The corrdinate has a zero delta relative to the origin. There is no data to modify.
                    SkDebugf("Failed to move point in X at all.\n");
                } else if (coordinates[pointIndex].xDeltaSize == 1) {
                    xCoordinates[coordinates[pointIndex].offsetToXDelta] = fPts[3].x();
                } else {
                    *reinterpret_cast<SK_OT_SHORT*>(xCoordinates + coordinates[pointIndex].offsetToXDelta) = SkEndian_SwapBE16(fPts[3].x());
                }

                if (coordinates[pointIndex].yDeltaSize == 0) {
                    // The corrdinate has a zero delta relative to the origin. There is no data to modify.
                    SkDebugf("Failed to move point in Y at all.\n");
                } else if (coordinates[pointIndex].yDeltaSize == 1) {
                    yCoordinates[coordinates[pointIndex].offsetToYDelta] = fPts[3].y();
                } else {
                    *reinterpret_cast<SK_OT_SHORT*>(yCoordinates + coordinates[pointIndex].offsetToYDelta) = SkEndian_SwapBE16(fPts[3].y());
                }
            }
        }

        int numberOfFullMetrics = SkEndian_SwapBE16(hheaTable->numberOfHMetrics);
        SkOTTableHorizontalMetrics::FullMetric* fullMetrics = hmtxTable->longHorMetric;
        SK_OT_SHORT* lsb;
        if (kGlyphID < numberOfFullMetrics) {
            lsb = &fullMetrics[kGlyphID].lsb;
        } else {
            SkOTTableHorizontalMetrics::ShortMetric* shortMetrics =
                SkTAfter<SkOTTableHorizontalMetrics::ShortMetric>(fullMetrics, numberOfFullMetrics);
            lsb = &shortMetrics[kGlyphID - numberOfFullMetrics].lsb;
        }
        *lsb = SkEndian_SwapBE16(fPts[2].x());

        return dataCopy;
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(DX, DY);

        SkPaint paint;
        SkPoint position{0, 0};
        SkPoint origin{100, 200};

        if (fDirty) {
            sk_sp<SkData> data(updateSBIXData(fSBIXData.get()));
            fFonts.clear();
            for (auto&& fontmgr : fFontMgr) {
                fFonts.emplace_back(fontmgr->makeFromData(data), 50);
            }
            fDirty = false;
        }
        for (auto&& font : fFonts) {
            //paint.setColor();
            paint.setStrokeWidth(0);
            canvas->drawGlyphs(1, &kGlyphID, &position, origin, font, paint);

            paint.setStrokeWidth(SkIntToScalar(kPointSize / 2));
            paint.setStyle(SkPaint::kStroke_Style);
            SkScalar advance;
            SkRect rect;
            font.getWidthsBounds(&kGlyphID, 1, &advance, &rect, &paint);

            rect.offset(origin);
            canvas->drawRect(rect, paint);
            canvas->drawLine(origin.fX, origin.fY, origin.fX + advance, origin.fY, paint);
            canvas->drawPoint(origin.fX, origin.fY, paint);
            canvas->drawPoint(origin.fX + advance, origin.fY, paint);
            origin.fX += 50;
        }

        paint.setStrokeWidth(SkIntToScalar(kPointSize));
        canvas->drawPoints(SkCanvas::kPoints_PointMode, SK_ARRAY_COUNT(fPts), fPts, paint);
    }

    //bool onAnimate(double nanos) override {
    //    return false;
    //}

    class PtClick : public Click {
    public:
        int fIndex;
        PtClick(int index) : fIndex(index) {}
    };

    static bool hittest(const SkPoint& pt, SkScalar x, SkScalar y) {
        return SkPoint::Length(pt.fX - x, pt.fY - y) < SkIntToScalar(kPointSize);
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        x -= DX;
        y -= DY;
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPts); i++) {
            if (hittest(fPts[i], x, y)) {
                return new PtClick((int)i);
            }
        }
        return nullptr;
    }

    bool onClick(Click* click) override {
        fPts[((PtClick*)click)->fIndex].set(click->fCurr.fX - DX, click->fCurr.fY - DY);
        fDirty = true;
        return true;
    }

private:
    using INHERITED = Sample;
};
}  // namespace
DEF_SAMPLE( return new SBIXView(); )
