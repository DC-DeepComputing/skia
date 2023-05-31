/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkXmp.h"
#include "tests/Test.h"

DEF_TEST(SkXmp_xmpHdrgmAsFieldValue, r) {
    // Expose HDRM values as fields. Also place the HDRGM namespace in the rdf:RDF node.
    const char xmpData[] =
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 6.0.0\">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
            "            xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\">\n"
            "      <rdf:Description rdf:about=\"\">\n"
            "         <hdrgm:Version>1.0</hdrgm:Version>\n"
            "         <hdrgm:GainMapMax>3</hdrgm:GainMapMax>\n"
            "         <hdrgm:HDRCapacityMax>4</hdrgm:HDRCapacityMax>\n"
            "      </rdf:Description>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

DEF_TEST(SkXmp_xmpHdrgmRequiresVersion, r) {
    // Same as the above, except with Version being absent.
    const char xmpData[] =
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 6.0.0\">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
            "            xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\">\n"
            "      <rdf:Description rdf:about=\"\">\n"
            "         <hdrgm:GainMapMax>3</hdrgm:GainMapMax>\n"
            "         <hdrgm:HDRCapacityMax>4</hdrgm:HDRCapacityMax>\n"
            "      </rdf:Description>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, !xmp->getGainmapInfoHDRGM(&info));
}

DEF_TEST(SkXmp_xmpHdrgmAsDescriptionPropertyAttributes, r) {
    // Expose HDRGM values as attributes on an rdf:Description node.
    const char xmpData[] =
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 6.0.0\">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "      <rdf:Description rdf:about=\"\"\n"
            "            xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "         hdrgm:Version=\"1.0\"\n"
            "         hdrgm:GainMapMax=\"3\"\n"
            "         hdrgm:HDRCapacityMax=\"4\"/>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

// Test mixed list and non-list entries.
DEF_TEST(SkXmp_xmpHdrgmList, r) {
    const char xmpData[] =
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 6.0.0\">\n"
            "   <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
            "            xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\">\n"
            "      <rdf:Description rdf:about=\"\"\n"
            "         hdrgm:Version=\"1.0\"\n"
            "         hdrgm:GainMapMin=\"2.0\"\n"
            "         hdrgm:OffsetSDR=\"0.1\">\n"
            "         <hdrgm:GainMapMax>\n"
            "           <rdf:Seq>\n"
            "             <rdf:li>3</rdf:li>\n"
            "             <rdf:li>4</rdf:li>\n"
            "             <rdf:li>5</rdf:li>\n"
            "           </rdf:Seq>\n"
            "         </hdrgm:GainMapMax>\n"
            "         <hdrgm:Gamma>\n"
            "           1.2\n"
            "         </hdrgm:Gamma>\n"
            "         <hdrgm:OffsetHDR>\n"
            "           <rdf:Seq>\n"
            "             <rdf:li>\n"
            "               0.2\n"
            "             </rdf:li>\n"
            "             <rdf:li>\n"
            "               0.3\n"
            "             </rdf:li>\n"
            "             <rdf:li>\n"
            "               0.4\n"
            "             </rdf:li>\n"
            "           </rdf:Seq>\n"
            "         </hdrgm:OffsetHDR>\n"
            "      </rdf:Description>\n"
            "   </rdf:RDF>\n"
            "</x:xmpmeta>\n";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fR == 4.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fG == 4.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fB == 4.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fG == 16.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fB == 32.f);

    REPORTER_ASSERT(r, info.fGainmapGamma.fR == 1.f/1.2f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fG == 1.f/1.2f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fB == 1.f/1.2f);

    REPORTER_ASSERT(r, info.fEpsilonSdr.fR == 0.1f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fG == 0.1f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fB == 0.1f);

    REPORTER_ASSERT(r, info.fEpsilonHdr.fR == 0.2f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fG == 0.3f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fB == 0.4f);
}

DEF_TEST(SkXmp_xmpContainerTypedNode, r) {
    // Container and Item using a node of type Container:Item.
    const char xmpData[] =
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "  <rdf:Description rdf:about=\"\"\n"
            "    xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "    xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\">\n"
            "   <Container:Directory>\n"
            "    <rdf:Seq>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <Container:Item>\n"
            "       <Item:Mime>image/jpeg</Item:Mime>\n"
            "       <Item:Semantic>Primary</Item:Semantic>\n"
            "      </Container:Item>\n"
            "     </rdf:li>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <Container:Item\n"
            "         Item:Semantic=\"GainMap\"\n"
            "         Item:Mime=\"image/jpeg\"\n"
            "         Item:Length=\"49035\"/>\n"
            "     </rdf:li>\n"
            "    </rdf:Seq>\n"
            "   </Container:Directory>\n"
            "  </rdf:Description>\n"
            " </rdf:RDF>\n"
            "</x:xmpmeta>\n";
    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    size_t offset = 999;
    size_t size = 999;
    REPORTER_ASSERT(r, xmp->getContainerGainmapLocation(&offset, &size));
    REPORTER_ASSERT(r, size == 49035);
}

DEF_TEST(SkXmp_xmpContainerTypedNodeRdfEquivalent, r) {
    // Container and Item using rdf:value and rdf:type pairs.
    const char xmpData[] =
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            " <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "  <rdf:Description rdf:about=\"\"\n"
            "    xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "    xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\">\n"
            "   <Container:Directory>\n"
            "    <rdf:Seq>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <rdf:value rdf:parseType=\"Resource\">\n"
            "       <Item:Mime>image/jpeg</Item:Mime>\n"
            "       <Item:Semantic>Primary</Item:Semantic>\n"
            "      </rdf:value>\n"
            "      <rdf:type rdf:resource=\"Item\"/>\n"
            "     </rdf:li>\n"
            "     <rdf:li rdf:parseType=\"Resource\">\n"
            "      <rdf:value rdf:parseType=\"Resource\">\n"
            "       <Item:Semantic>GainMap</Item:Semantic>\n"
            "       <Item:Mime>image/jpeg</Item:Mime>\n"
            "       <Item:Length>49035</Item:Length>\n"
            "      </rdf:value>\n"
            "      <rdf:type rdf:resource=\"Item\"/>\n"
            "     </rdf:li>\n"
            "    </rdf:Seq>\n"
            "   </Container:Directory>\n"
            "  </rdf:Description>\n"
            " </rdf:RDF>\n"
            "</x:xmpmeta>\n";
    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    size_t offset = 999;
    size_t size = 999;
    REPORTER_ASSERT(r, xmp->getContainerGainmapLocation(&offset, &size));
    REPORTER_ASSERT(r, size == 49035);
}
