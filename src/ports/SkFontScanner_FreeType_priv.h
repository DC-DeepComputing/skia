/*
* Copyright 2024 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_FREE_TYPE_PRIV_H_
#define SKFONTSCANNER_FREE_TYPE_PRIV_H_

#include "include/core/SkData.h"
#include "include/core/SkFontScanner.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/ports/SkTypeface_FreeType.h"

class SkFontScanner_FreeType : public SkFontScanner {
public:
    SkFontScanner_FreeType();
    ~SkFontScanner_FreeType() override;

    bool scanFile(SkStreamAsset* stream, int* numFaces) const override;
    bool scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) const override;
    bool scanInstance(SkStreamAsset* stream,
                      int faceIndex,
                      int instanceIndex,
                      SkString* name,
                      SkFontStyle* style,
                      bool* isFixedPitch,
                      AxisDefinitions* axes,
                      VariationPosition* position) const override;
    sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                     const SkFontArguments& args) const override;
    SkTypeface::FactoryId getFactoryId() const override;
    static void computeAxisValues(
            const AxisDefinitions& axisDefinitions,
            const SkFontArguments::VariationPosition currentPosition,
            const SkFontArguments::VariationPosition requestedPosition,
            SkFixed* axisValues,
            const SkString& name,
            SkFontStyle* style);
private:
    struct CachedScanInstance {
        sk_sp<const SkData> fData;
        int fFaceIndex = -1;
        SkString fName;
        SkFontStyle fStyle;
        bool fIsFixedPitch = false;
        AxisDefinitions fAxes;
        VariationPosition fPosition;
    };

    FT_Face openFace(SkStreamAsset* stream, int ttcIndex, FT_Stream ftStream) const;
    bool scanInstanceLocked(SkStreamAsset* stream,
                            int faceIndex,
                            int instanceIndex,
                            SkString* name,
                            SkFontStyle* style,
                            bool* isFixedPitch,
                            AxisDefinitions* axes,
                            VariationPosition* position) const;

    FT_Library fLibrary;
    mutable SkMutex fLibraryMutex;
    mutable CachedScanInstance fLastScannedInstance;
};

#endif // SKFONTSCANNER_FREE_TYPE_PRIV_H_
