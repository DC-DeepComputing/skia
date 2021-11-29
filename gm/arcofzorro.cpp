/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

#include "include/core/SkPath.h"

static SkPath gimme() {
    SkPath path;

    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x3fe0633f), SkBits2Float(0x3d04a60d));  // 1.75303f, 0.0323849f
    path.cubicTo(SkBits2Float(0x3fe27540), SkBits2Float(0x3dff593f), SkBits2Float(0x3fe45241),
                 SkBits2Float(0x3e5e2fbb), SkBits2Float(0x3fe55b41), SkBits2Float(
                    0x3e9e596d));  // 1.7692f, 0.124682f, 1.78376f, 0.216979f, 1.79185f, 0.309276f
    path.cubicTo(SkBits2Float(0x3fe5fa41), SkBits2Float(0x3eb3e79c), SkBits2Float(0x3fe62f41),
                 SkBits2Float(0x3ec975cb), SkBits2Float(0x3fe69941), SkBits2Float(
                    0x3edfd837));  // 1.7967f, 0.351376f, 1.79832f, 0.393477f, 1.80155f, 0.437196f
    path.cubicTo(SkBits2Float(0x3fe70341), SkBits2Float(0x3f064e87), SkBits2Float(0x3fe6ce41),
                 SkBits2Float(0x3f1cb0f2), SkBits2Float(0x3fe59041), SkBits2Float(
                    0x3f33135e));  // 1.80479f, 0.524636f, 1.80317f, 0.612075f, 1.79346f, 0.699514f
    path.cubicTo(SkBits2Float(0x3fe48740), SkBits2Float(0x3f468ef5), SkBits2Float(0x3fe2df40),
                 SkBits2Float(0x3f59a06d), SkBits2Float(0x3fe02e3f), SkBits2Float(
                    0x3f6cb1e6));  // 1.78538f, 0.775619f, 1.77244f, 0.850104f, 1.75141f, 0.92459f
    path.cubicTo(SkBits2Float(0x3fde863f), SkBits2Float(0x3f78b759), SkBits2Float(0x3fdc743e),
                 SkBits2Float(0x3f822957), SkBits2Float(0x3fd9c33e), SkBits2Float(
                    0x3f87f701));  // 1.73847f, 0.971548f, 1.7223f, 1.01689f, 1.70127f, 1.06223f
    path.cubicTo(SkBits2Float(0x3fd98e3e), SkBits2Float(0x3f88611f), SkBits2Float(0x3fd9593e),
                 SkBits2Float(0x3f88cb3e), SkBits2Float(0x3fd9243d), SkBits2Float(
                    0x3f896a6b));  // 1.69965f, 1.06546f, 1.69804f, 1.0687f, 1.69642f, 1.07356f
    path.cubicTo(SkBits2Float(0x3fd63e3c), SkBits2Float(0x3f8fa234), SkBits2Float(0x3fd2ee3b),
                 SkBits2Float(0x3f95d9fd), SkBits2Float(0x3fd2ee3b), SkBits2Float(
                    0x3f9ce602));  // 1.67377f, 1.12214f, 1.6479f, 1.17071f, 1.6479f, 1.22577f
    path.cubicTo(SkBits2Float(0x3fd3233b), SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3583b),
                 SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3c23c), SkBits2Float(
                    0x3f9c7be4));  // 1.64951f, 1.22415f, 1.65113f, 1.22415f, 1.65437f, 1.22253f
    path.cubicTo(SkBits2Float(0x3fd3c23c), SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3c23c),
                 SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3c23c), SkBits2Float(
                    0x3f9ce602));  // 1.65437f, 1.22415f, 1.65437f, 1.22415f, 1.65437f, 1.22577f
    path.cubicTo(SkBits2Float(0x3fd5353c), SkBits2Float(0x3f9c46d4), SkBits2Float(0x3fd6dd3d),
                 SkBits2Float(0x3f9bdcb6), SkBits2Float(0x3fd7b13d), SkBits2Float(
                    0x3f9ad36a));  // 1.66569f, 1.22091f, 1.67863f, 1.21767f, 1.6851f, 1.20958f
    path.cubicTo(SkBits2Float(0x3fda623e), SkBits2Float(0x3f96ae3a), SkBits2Float(0x3fdca93f),
                 SkBits2Float(0x3f921eeb), SkBits2Float(0x3fdf253f), SkBits2Float(
                    0x3f8dc4ab));  // 1.70612f, 1.17719f, 1.72391f, 1.14157f, 1.74332f, 1.10756f
    path.cubicTo(SkBits2Float(0x3fe0983f), SkBits2Float(0x3f8b12e5), SkBits2Float(0x3fe1d640),
                 SkBits2Float(0x3f87f700), SkBits2Float(0x3fe3b340), SkBits2Float(
                    0x3f857a4a));  // 1.75465f, 1.08651f, 1.76435f, 1.06223f, 1.77891f, 1.04279f
    path.cubicTo(SkBits2Float(0x3fe48740), SkBits2Float(0x3f8470fe), SkBits2Float(0x3fe62f40),
                 SkBits2Float(0x3f8470fe), SkBits2Float(0x3fe7d741), SkBits2Float(
                    0x3f843bef));  // 1.78538f, 1.0347f, 1.79832f, 1.0347f, 1.81126f, 1.03308f
    path.cubicTo(SkBits2Float(0x3fe2aa40), SkBits2Float(0x3f943182), SkBits2Float(0x3fda623d),
                 SkBits2Float(0x3fa2498e), SkBits2Float(0x3fceff3a), SkBits2Float(
                    0x3fae4f01));  // 1.77082f, 1.15776f, 1.70612f, 1.26787f, 1.61716f, 1.36179f
    path.cubicTo(SkBits2Float(0x3fce6039), SkBits2Float(0x3faf233e), SkBits2Float(0x3fcd2239),
                 SkBits2Float(0x3faf584d), SkBits2Float(0x3fcc1939), SkBits2Float(
                    0x3fafc26b));  // 1.61231f, 1.36826f, 1.60261f, 1.36988f, 1.59452f, 1.37312f
    path.cubicTo(SkBits2Float(0x3fcc1939), SkBits2Float(0x3faff77a), SkBits2Float(0x3fcc1939),
                 SkBits2Float(0x3faff77a), SkBits2Float(0x3fcc4e39), SkBits2Float(
                    0x3fb02c89));  // 1.59452f, 1.37474f, 1.59452f, 1.37474f, 1.59614f, 1.37636f
    path.cubicTo(SkBits2Float(0x3fcc1939), SkBits2Float(0x3fb02c89), SkBits2Float(0x3fcc1939),
                 SkBits2Float(0x3fb02c89), SkBits2Float(0x3fcbe439), SkBits2Float(
                    0x3fb02c89));  // 1.59452f, 1.37636f, 1.59452f, 1.37636f, 1.5929f, 1.37636f
    path.cubicTo(SkBits2Float(0x3fcbe439), SkBits2Float(0x3fb20a12), SkBits2Float(0x3fcb4539),
                 SkBits2Float(0x3fb37d7d), SkBits2Float(0x3fc99d39), SkBits2Float(
                    0x3fb3b28c));  // 1.5929f, 1.39093f, 1.58805f, 1.40227f, 1.57511f, 1.40389f
    path.cubicTo(SkBits2Float(0x3fc93339), SkBits2Float(0x3fb3e79b), SkBits2Float(0x3fc8c938),
                 SkBits2Float(0x3fb41caa), SkBits2Float(0x3fc7f538), SkBits2Float(
                    0x3fb41caa));  // 1.57188f, 1.40551f, 1.56864f, 1.40712f, 1.56217f, 1.40712f
    path.cubicTo(SkBits2Float(0x3fc7f538), SkBits2Float(0x3fb3e79b), SkBits2Float(0x3fc7f538),
                 SkBits2Float(0x3fb3e79b), SkBits2Float(0x3fc7f538), SkBits2Float(
                    0x3fb3b28c));  // 1.56217f, 1.40551f, 1.56217f, 1.40551f, 1.56217f, 1.40389f
    path.lineTo(SkBits2Float(0x3fc7c038), SkBits2Float(0x3fb3b28c));  // 1.56055f, 1.40389f
    path.cubicTo(SkBits2Float(0x3fc7c038), SkBits2Float(0x3fb4f0e7), SkBits2Float(0x3fc7f538),
                 SkBits2Float(0x3fb66452), SkBits2Float(0x3fc78b38), SkBits2Float(
                    0x3fb76d9e));  // 1.56055f, 1.4136f, 1.56217f, 1.42494f, 1.55894f, 1.43303f
    path.cubicTo(SkBits2Float(0x3fc3d137), SkBits2Float(0x3fbe4495), SkBits2Float(0x3fbf4336),
                 SkBits2Float(0x3fc4123e), SkBits2Float(0x3fb80434), SkBits2Float(
                    0x3fc76331));  // 1.52982f, 1.48647f, 1.49424f, 1.53181f, 1.43763f, 1.55771f
    path.cubicTo(SkBits2Float(0x3fb47f33), SkBits2Float(0x3fc90bac), SkBits2Float(0x3fb19932),
                 SkBits2Float(0x3fcb5353), SkBits2Float(0x3faf1d31), SkBits2Float(
                    0x3fce6f37));  // 1.41013f, 1.57067f, 1.38749f, 1.58848f, 1.36808f, 1.61277f
    path.cubicTo(SkBits2Float(0x3fa4592e), SkBits2Float(0x3fdb13d7), SkBits2Float(0x3f974e2a),
                 SkBits2Float(0x3fe53bc1), SkBits2Float(0x3f896f25), SkBits2Float(
                    0x3fee5a5f));  // 1.28397f, 1.71154f, 1.18207f, 1.79089f, 1.0737f, 1.86213f
    path.cubicTo(SkBits2Float(0x3f6b883f), SkBits2Float(0x3ffb691f), SkBits2Float(0x3f42f434),
                 SkBits2Float(0x400367b2), SkBits2Float(0x3f184e28), SkBits2Float(
                    0x4008611f));  // 0.920048f, 1.96415f, 0.761539f, 2.0532f, 0.594943f, 2.13093f
    path.cubicTo(SkBits2Float(0x3f184e28), SkBits2Float(0x4008611f), SkBits2Float(0x3f17e428),
                 SkBits2Float(0x4008611f), SkBits2Float(0x3f17e428), SkBits2Float(
                    0x40087ba7));  // 0.594943f, 2.13093f, 0.593325f, 2.13093f, 0.593325f, 2.13255f
    path.cubicTo(SkBits2Float(0x3effc044), SkBits2Float(0x400b47f5), SkBits2Float(0x3ed08c36),
                 SkBits2Float(0x400e2eca), SkBits2Float(0x3e9edc28), SkBits2Float(
                    0x401090f9));  // 0.499514f, 2.17627f, 0.40732f, 2.22161f, 0.310273f, 2.25885f
    path.cubicTo(SkBits2Float(0x3e5a5832), SkBits2Float(0x4012f328), SkBits2Float(0x3de40030),
                 SkBits2Float(0x4014811a), SkBits2Float(0x3c1a7f9e), SkBits2Float(
                    0x40158a66));  // 0.213227f, 2.29609f, 0.111328f, 2.32038f, 0.00942984f, 2.33657f
    path.lineTo(SkBits2Float(0x3c1a7f9e), SkBits2Float(0x401bf73d));  // 0.00942984f, 2.43697f
    path.cubicTo(SkBits2Float(0x3dc98028), SkBits2Float(0x401b580f), SkBits2Float(0x3e3fd82e),
                 SkBits2Float(0x401a694b), SkBits2Float(0x3e8ca424), SkBits2Float(
                    0x40191068));  // 0.098389f, 2.42725f, 0.187348f, 2.41268f, 0.27469f, 2.39163f
    path.cubicTo(SkBits2Float(0x3e94ec27), SkBits2Float(0x4018db59), SkBits2Float(0x3e9d3429),
                 SkBits2Float(0x40188bc2), SkBits2Float(0x3ea4a82b), SkBits2Float(
                    0x401856b3));  // 0.290864f, 2.38839f, 0.307039f, 2.38353f, 0.321596f, 2.38029f
    path.cubicTo(SkBits2Float(0x3eae982e), SkBits2Float(0x4018071c), SkBits2Float(0x3eb95c31),
                 SkBits2Float(0x40179cfe), SkBits2Float(0x3ec34c34), SkBits2Float(
                    0x40174d67));  // 0.341005f, 2.37543f, 0.362031f, 2.36896f, 0.381441f, 2.3641f
    path.cubicTo(SkBits2Float(0x3ec9ec36), SkBits2Float(0x40171858), SkBits2Float(0x3ed08c38),
                 SkBits2Float(0x4016c8c1), SkBits2Float(0x3ed8003a), SkBits2Float(
                    0x401693b2));  // 0.39438f, 2.36086f, 0.40732f, 2.356f, 0.421877f, 2.35276f
    path.cubicTo(SkBits2Float(0x3eda7c3a), SkBits2Float(0x4016792a), SkBits2Float(0x3eddcc3c),
                 SkBits2Float(0x40165ea3), SkBits2Float(0x3ee0483c), SkBits2Float(
                    0x4016441b));  // 0.426729f, 2.35115f, 0.433199f, 2.34953f, 0.438051f, 2.34791f
    path.cubicTo(SkBits2Float(0x3ee2c43d), SkBits2Float(0x40162993), SkBits2Float(0x3ee5403e),
                 SkBits2Float(0x40160f0c), SkBits2Float(0x3ee8903f), SkBits2Float(
                    0x4015f484));  // 0.442903f, 2.34629f, 0.447756f, 2.34467f, 0.454226f, 2.34305f
    path.cubicTo(SkBits2Float(0x3f1c082a), SkBits2Float(0x4012be17), SkBits2Float(0x3f422036),
                 SkBits2Float(0x400e63d8), SkBits2Float(0x3f66fa40), SkBits2Float(
                    0x40096a6a));  // 0.6095f, 2.29285f, 0.758304f, 2.22484f, 0.902256f, 2.14712f
    path.cubicTo(SkBits2Float(0x3f6a4a41), SkBits2Float(0x4009004c), SkBits2Float(0x3f6d3042),
                 SkBits2Float(0x4008962d), SkBits2Float(0x3f708043), SkBits2Float(
                    0x40081187));  // 0.915196f, 2.14064f, 0.926518f, 2.13417f, 0.939457f, 2.12607f
    path.cubicTo(SkBits2Float(0x3f7efe47), SkBits2Float(0x4005feef), SkBits2Float(0x3f868925),
                 SkBits2Float(0x4003b748), SkBits2Float(0x3f8d5e28), SkBits2Float(
                    0x40015519));  // 0.996067f, 2.09368f, 1.05106f, 2.05806f, 1.10444f, 2.02082f
    path.cubicTo(SkBits2Float(0x3f97b82b), SkBits2Float(0x3ffb691d), SkBits2Float(0x3fa1a82e),
                 SkBits2Float(0x3ff388da), SkBits2Float(0x3fab9830), SkBits2Float(
                    0x3feb7389));  // 1.18531f, 1.96415f, 1.26294f, 1.90261f, 1.34058f, 1.83946f
    path.cubicTo(SkBits2Float(0x3fb20332), SkBits2Float(0x3fe6450c), SkBits2Float(0x3fb80434),
                 SkBits2Float(0x3fe0e181), SkBits2Float(0x3fbd6635), SkBits2Float(
                    0x3fda3f99));  // 1.39072f, 1.79898f, 1.43763f, 1.75688f, 1.47968f, 1.70507f
    path.cubicTo(SkBits2Float(0x3fbf4336), SkBits2Float(0x3fd7f7f2), SkBits2Float(0x3fc12037),
                 SkBits2Float(0x3fd5b04b), SkBits2Float(0x3fc2fd36), SkBits2Float(
                    0x3fd33394));  // 1.49424f, 1.68725f, 1.5088f, 1.66944f, 1.52335f, 1.65001f
    path.cubicTo(SkBits2Float(0x3fc5e337), SkBits2Float(0x3fcf7881), SkBits2Float(0x3fc8c938),
                 SkBits2Float(0x3fcbbd70), SkBits2Float(0x3fcbaf38), SkBits2Float(
                    0x3fc8025d));  // 1.546f, 1.62086f, 1.56864f, 1.59172f, 1.59128f, 1.56257f
    path.cubicTo(SkBits2Float(0x3fceff39), SkBits2Float(0x3fc3a81e), SkBits2Float(0x3fd2843b),
                 SkBits2Float(0x3fbf18cf), SkBits2Float(0x3fd5d43b), SkBits2Float(
                    0x3fbabe8f));  // 1.61716f, 1.52857f, 1.64466f, 1.49294f, 1.67054f, 1.45894f
    path.cubicTo(SkBits2Float(0x3fd8503c), SkBits2Float(0x3fb7a2ab), SkBits2Float(0x3fda973d),
                 SkBits2Float(0x3fb486c7), SkBits2Float(0x3fdca93e), SkBits2Float(
                    0x3fb135d3));  // 1.68995f, 1.43465f, 1.70774f, 1.41036f, 1.72391f, 1.38446f
    path.cubicTo(SkBits2Float(0x3fe5c541), SkBits2Float(0x3fa2b3aa), SkBits2Float(0x3feb5c42),
                 SkBits2Float(0x3f92be16), SkBits2Float(0x3ff15d44), SkBits2Float(
                    0x3f82c882));  // 1.79508f, 1.27111f, 1.83875f, 1.14643f, 1.88566f, 1.02174f
    path.cubicTo(SkBits2Float(0x3ff1fc44), SkBits2Float(0x3f812008), SkBits2Float(0x3ff23144),
                 SkBits2Float(0x3f7e1adf), SkBits2Float(0x3ff29b44), SkBits2Float(
                    0x3f7a5fcc));  // 1.89051f, 1.00879f, 1.89213f, 0.992598f, 1.89536f, 0.978024f
    path.cubicTo(SkBits2Float(0x3ff47845), SkBits2Float(0x3f5fd830), SkBits2Float(0x3ff65545),
                 SkBits2Float(0x3f455094), SkBits2Float(0x3ff6bf45), SkBits2Float(
                    0x3f2a5ed9));  // 1.90992f, 0.874393f, 1.92448f, 0.770761f, 1.92771f, 0.66551f
    path.cubicTo(SkBits2Float(0x3ff33a44), SkBits2Float(0x3f0d5a87), SkBits2Float(0x3ff08943),
                 SkBits2Float(0x3edf03ee), SkBits2Float(0x3fee7743), SkBits2Float(
                    0x3ea352cf));  // 1.90022f, 0.552163f, 1.87919f, 0.435577f, 1.86301f, 0.318991f
    path.cubicTo(SkBits2Float(0x3feccf42), SkBits2Float(0x3e5c872d), SkBits2Float(0x3feb9142),
                 SkBits2Float(0x3de4d179), SkBits2Float(0x3feaf242), SkBits2Float(
                    0x3c04a4ae));  // 1.85008f, 0.215359f, 1.84037f, 0.111728f, 1.83552f, 0.0080959f
    path.lineTo(SkBits2Float(0x3fe02e3f), SkBits2Float(0x3c04a4ae));  // 1.75141f, 0.0080959f
    path.cubicTo(SkBits2Float(0x3fdff93f), SkBits2Float(0x3c6ec47e), SkBits2Float(0x3fe02e3f),
                 SkBits2Float(0x3cb9b545), SkBits2Float(0x3fe0633f), SkBits2Float(
                    0x3d04a60d));  // 1.74979f, 0.0145732f, 1.75141f, 0.0226694f, 1.75303f, 0.0323849f
    path.close();
    path.moveTo(SkBits2Float(0x3fe97f42), SkBits2Float(0x3f7b9e2e));  // 1.8242f, 0.982882f
    path.cubicTo(SkBits2Float(0x3fe91542), SkBits2Float(0x3f7eef21), SkBits2Float(0x3fe87642),
                 SkBits2Float(0x3f81551a), SkBits2Float(0x3fe7d741), SkBits2Float(
                    0x3f82fd94));  // 1.82096f, 0.995836f, 1.81611f, 1.01041f, 1.81126f, 1.02336f
    path.cubicTo(SkBits2Float(0x3fe6ce41), SkBits2Float(0x3f81bf39), SkBits2Float(0x3fe66441),
                 SkBits2Float(0x3f8080dd), SkBits2Float(0x3fe66441), SkBits2Float(
                    0x3f7e1ae4));  // 1.80317f, 1.01365f, 1.79993f, 1.00393f, 1.79993f, 0.992598f
    path.cubicTo(SkBits2Float(0x3fe66441), SkBits2Float(0x3f7c726a), SkBits2Float(0x3fe69941),
                 SkBits2Float(0x3f7b340e), SkBits2Float(0x3fe6ce41), SkBits2Float(
                    0x3f798b95));  // 1.79993f, 0.986121f, 1.80155f, 0.981263f, 1.80317f, 0.974786f
    path.cubicTo(SkBits2Float(0x3fe70341), SkBits2Float(0x3f78b758), SkBits2Float(0x3fe76d41),
                 SkBits2Float(0x3f770edf), SkBits2Float(0x3fe7d741), SkBits2Float(
                    0x3f770edf));  // 1.80479f, 0.971548f, 1.80802f, 0.965071f, 1.81126f, 0.965071f
    path.cubicTo(SkBits2Float(0x3fe84141), SkBits2Float(0x3f770edf), SkBits2Float(0x3fe8ab42),
                 SkBits2Float(0x3f770edf), SkBits2Float(0x3fe8e041), SkBits2Float(
                    0x3f7778fd));  // 1.81449f, 0.965071f, 1.81773f, 0.965071f, 1.81934f, 0.96669f
    path.cubicTo(SkBits2Float(0x3fe97f42), SkBits2Float(0x3f77e31b), SkBits2Float(0x3fe9e942),
                 SkBits2Float(0x3f798b95), SkBits2Float(0x3fe97f42), SkBits2Float(
                    0x3f7b9e2e));  // 1.8242f, 0.968309f, 1.82743f, 0.974786f, 1.8242f, 0.982882f
    path.close();

    return path;
}

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(300, 300);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPath path = gimme();

        SkRect r = path.getBounds();

        SkPaint paint;

        canvas->scale(100.0f, 100.0f);
        canvas->drawPath(path, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);

        canvas->drawRect(r, paint);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
