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
#include "src/base/SkRandom.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkAutoPixmapStorage.h"

const SkColor kExpected[16][16] = {
        {
                SkColorSetRGB(1, 235, 55),
                SkColorSetRGB(1, 221, 63),
                SkColorSetRGB(2, 203, 71),
                SkColorSetRGB(4, 187, 78),
                SkColorSetRGB(5, 175, 89),
                SkColorSetRGB(8, 160, 100),
                SkColorSetRGB(10, 147, 109),
                SkColorSetRGB(13, 134, 120),
                SkColorSetRGB(17, 120, 134),
                SkColorSetRGB(20, 109, 147),
                SkColorSetRGB(24, 100, 160),
                SkColorSetRGB(30, 89, 175),
                SkColorSetRGB(35, 78, 187),
                SkColorSetRGB(41, 71, 203),
                SkColorSetRGB(47, 63, 221),
                SkColorSetRGB(55, 55, 235)
        },

        {
                SkColorSetRGB(1, 221, 47),
                SkColorSetRGB(2, 203, 55),
                SkColorSetRGB(4, 187, 63),
                SkColorSetRGB(5, 175, 71),
                SkColorSetRGB(8, 160, 78),
                SkColorSetRGB(10, 147, 89),
                SkColorSetRGB(13, 134, 100),
                SkColorSetRGB(17, 120, 109),
                SkColorSetRGB(20, 109, 120),
                SkColorSetRGB(24, 100, 134),
                SkColorSetRGB(30, 89, 147),
                SkColorSetRGB(35, 78, 160),
                SkColorSetRGB(41, 71, 175),
                SkColorSetRGB(47, 63, 187),
                SkColorSetRGB(55, 55, 203),
                SkColorSetRGB(63, 47, 221)
        },

        {
                SkColorSetRGB(2, 203, 41),
                SkColorSetRGB(4, 187, 47),
                SkColorSetRGB(5, 175, 55),
                SkColorSetRGB(8, 160, 63),
                SkColorSetRGB(10, 147, 71),
                SkColorSetRGB(13, 134, 78),
                SkColorSetRGB(17, 120, 89),
                SkColorSetRGB(20, 109, 100),
                SkColorSetRGB(24, 100, 109),
                SkColorSetRGB(30, 89, 120),
                SkColorSetRGB(35, 78, 134),
                SkColorSetRGB(41, 71, 147),
                SkColorSetRGB(47, 63, 160),
                SkColorSetRGB(55, 55, 175),
                SkColorSetRGB(63, 47, 187),
                SkColorSetRGB(71, 41, 203)
        },

        {
                SkColorSetRGB(4, 187, 35),
                SkColorSetRGB(5, 175, 41),
                SkColorSetRGB(8, 160, 47),
                SkColorSetRGB(10, 147, 55),
                SkColorSetRGB(13, 134, 63),
                SkColorSetRGB(17, 120, 71),
                SkColorSetRGB(20, 109, 78),
                SkColorSetRGB(24, 100, 89),
                SkColorSetRGB(30, 89, 100),
                SkColorSetRGB(35, 78, 109),
                SkColorSetRGB(41, 71, 120),
                SkColorSetRGB(47, 63, 134),
                SkColorSetRGB(55, 55, 147),
                SkColorSetRGB(63, 47, 160),
                SkColorSetRGB(71, 41, 175),
                SkColorSetRGB(78, 35, 187)
        },

        {
                SkColorSetRGB(5, 175, 30),
                SkColorSetRGB(8, 160, 35),
                SkColorSetRGB(10, 147, 41),
                SkColorSetRGB(13, 134, 47),
                SkColorSetRGB(17, 120, 55),
                SkColorSetRGB(20, 109, 63),
                SkColorSetRGB(24, 100, 71),
                SkColorSetRGB(30, 89, 78),
                SkColorSetRGB(35, 78, 89),
                SkColorSetRGB(41, 71, 100),
                SkColorSetRGB(47, 63, 109),
                SkColorSetRGB(55, 55, 120),
                SkColorSetRGB(63, 47, 134),
                SkColorSetRGB(71, 41, 147),
                SkColorSetRGB(78, 35, 160),
                SkColorSetRGB(89, 30, 175)
        },

        {
                SkColorSetRGB(8, 160, 24),
                SkColorSetRGB(10, 147, 30),
                SkColorSetRGB(13, 134, 35),
                SkColorSetRGB(17, 120, 41),
                SkColorSetRGB(20, 109, 47),
                SkColorSetRGB(24, 100, 55),
                SkColorSetRGB(30, 89, 63),
                SkColorSetRGB(35, 78, 71),
                SkColorSetRGB(41, 71, 78),
                SkColorSetRGB(47, 63, 89),
                SkColorSetRGB(55, 55, 100),
                SkColorSetRGB(63, 47, 109),
                SkColorSetRGB(71, 41, 120),
                SkColorSetRGB(78, 35, 134),
                SkColorSetRGB(89, 30, 147),
                SkColorSetRGB(100, 24, 160)
        },

        {
                SkColorSetRGB(10, 147, 20),
                SkColorSetRGB(13, 134, 24),
                SkColorSetRGB(17, 120, 30),
                SkColorSetRGB(20, 109, 35),
                SkColorSetRGB(24, 100, 41),
                SkColorSetRGB(30, 89, 47),
                SkColorSetRGB(35, 78, 55),
                SkColorSetRGB(41, 71, 63),
                SkColorSetRGB(47, 63, 71),
                SkColorSetRGB(55, 55, 78),
                SkColorSetRGB(63, 47, 89),
                SkColorSetRGB(71, 41, 100),
                SkColorSetRGB(78, 35, 109),
                SkColorSetRGB(89, 30, 120),
                SkColorSetRGB(100, 24, 134),
                SkColorSetRGB(109, 20, 147)
        },

        {
                SkColorSetRGB(13, 134, 17),
                SkColorSetRGB(17, 120, 20),
                SkColorSetRGB(20, 109, 24),
                SkColorSetRGB(24, 100, 30),
                SkColorSetRGB(30, 89, 35),
                SkColorSetRGB(35, 78, 41),
                SkColorSetRGB(41, 71, 47),
                SkColorSetRGB(47, 63, 55),
                SkColorSetRGB(55, 55, 63),
                SkColorSetRGB(63, 47, 71),
                SkColorSetRGB(71, 41, 78),
                SkColorSetRGB(78, 35, 89),
                SkColorSetRGB(89, 30, 100),
                SkColorSetRGB(100, 24, 109),
                SkColorSetRGB(109, 20, 120),
                SkColorSetRGB(120, 17, 134)
        },

        {
                SkColorSetRGB(17, 120, 13),
                SkColorSetRGB(20, 109, 17),
                SkColorSetRGB(24, 100, 20),
                SkColorSetRGB(30, 89, 24),
                SkColorSetRGB(35, 78, 30),
                SkColorSetRGB(41, 71, 35),
                SkColorSetRGB(47, 63, 41),
                SkColorSetRGB(55, 55, 47),
                SkColorSetRGB(63, 47, 55),
                SkColorSetRGB(71, 41, 63),
                SkColorSetRGB(78, 35, 71),
                SkColorSetRGB(89, 30, 78),
                SkColorSetRGB(100, 24, 89),
                SkColorSetRGB(109, 20, 100),
                SkColorSetRGB(120, 17, 109),
                SkColorSetRGB(134, 13, 120)
        },

        {
                SkColorSetRGB(20, 109, 10),
                SkColorSetRGB(24, 100, 13),
                SkColorSetRGB(30, 89, 17),
                SkColorSetRGB(35, 78, 20),
                SkColorSetRGB(41, 71, 24),
                SkColorSetRGB(47, 63, 30),
                SkColorSetRGB(55, 55, 35),
                SkColorSetRGB(63, 47, 41),
                SkColorSetRGB(71, 41, 47),
                SkColorSetRGB(78, 35, 55),
                SkColorSetRGB(89, 30, 63),
                SkColorSetRGB(100, 24, 71),
                SkColorSetRGB(109, 20, 78),
                SkColorSetRGB(120, 17, 89),
                SkColorSetRGB(134, 13, 100),
                SkColorSetRGB(147, 10, 109)
        },

        {
                SkColorSetRGB(24, 100, 8),
                SkColorSetRGB(30, 89, 10),
                SkColorSetRGB(35, 78, 13),
                SkColorSetRGB(41, 71, 17),
                SkColorSetRGB(47, 63, 20),
                SkColorSetRGB(55, 55, 24),
                SkColorSetRGB(63, 47, 30),
                SkColorSetRGB(71, 41, 35),
                SkColorSetRGB(78, 35, 41),
                SkColorSetRGB(89, 30, 47),
                SkColorSetRGB(100, 24, 55),
                SkColorSetRGB(109, 20, 63),
                SkColorSetRGB(120, 17, 71),
                SkColorSetRGB(134, 13, 78),
                SkColorSetRGB(147, 10, 89),
                SkColorSetRGB(160, 8, 100)
        },

        {
                SkColorSetRGB(30, 89, 5),
                SkColorSetRGB(35, 78, 8),
                SkColorSetRGB(41, 71, 10),
                SkColorSetRGB(47, 63, 13),
                SkColorSetRGB(55, 55, 17),
                SkColorSetRGB(63, 47, 20),
                SkColorSetRGB(71, 41, 24),
                SkColorSetRGB(78, 35, 30),
                SkColorSetRGB(89, 30, 35),
                SkColorSetRGB(100, 24, 41),
                SkColorSetRGB(109, 20, 47),
                SkColorSetRGB(120, 17, 55),
                SkColorSetRGB(134, 13, 63),
                SkColorSetRGB(147, 10, 71),
                SkColorSetRGB(160, 8, 78),
                SkColorSetRGB(175, 5, 89)
        },

        {
                SkColorSetRGB(35, 78, 4),
                SkColorSetRGB(41, 71, 5),
                SkColorSetRGB(47, 63, 8),
                SkColorSetRGB(55, 55, 10),
                SkColorSetRGB(63, 47, 13),
                SkColorSetRGB(71, 41, 17),
                SkColorSetRGB(78, 35, 20),
                SkColorSetRGB(89, 30, 24),
                SkColorSetRGB(100, 24, 30),
                SkColorSetRGB(109, 20, 35),
                SkColorSetRGB(120, 17, 41),
                SkColorSetRGB(134, 13, 47),
                SkColorSetRGB(147, 10, 55),
                SkColorSetRGB(160, 8, 63),
                SkColorSetRGB(175, 5, 71),
                SkColorSetRGB(187, 4, 78)
        },

        {
                SkColorSetRGB(41, 71, 2),
                SkColorSetRGB(47, 63, 4),
                SkColorSetRGB(55, 55, 5),
                SkColorSetRGB(63, 47, 8),
                SkColorSetRGB(71, 41, 10),
                SkColorSetRGB(78, 35, 13),
                SkColorSetRGB(89, 30, 17),
                SkColorSetRGB(100, 24, 20),
                SkColorSetRGB(109, 20, 24),
                SkColorSetRGB(120, 17, 30),
                SkColorSetRGB(134, 13, 35),
                SkColorSetRGB(147, 10, 41),
                SkColorSetRGB(160, 8, 47),
                SkColorSetRGB(175, 5, 55),
                SkColorSetRGB(187, 4, 63),
                SkColorSetRGB(203, 2, 71)
        },

        {
                SkColorSetRGB(47, 63, 1),
                SkColorSetRGB(55, 55, 2),
                SkColorSetRGB(63, 47, 4),
                SkColorSetRGB(71, 41, 5),
                SkColorSetRGB(78, 35, 8),
                SkColorSetRGB(89, 30, 10),
                SkColorSetRGB(100, 24, 13),
                SkColorSetRGB(109, 20, 17),
                SkColorSetRGB(120, 17, 20),
                SkColorSetRGB(134, 13, 24),
                SkColorSetRGB(147, 10, 30),
                SkColorSetRGB(160, 8, 35),
                SkColorSetRGB(175, 5, 41),
                SkColorSetRGB(187, 4, 47),
                SkColorSetRGB(203, 2, 55),
                SkColorSetRGB(221, 1, 63)
        },

        {
                SkColorSetRGB(55, 55, 1),
                SkColorSetRGB(63, 47, 1),
                SkColorSetRGB(71, 41, 2),
                SkColorSetRGB(78, 35, 4),
                SkColorSetRGB(89, 30, 5),
                SkColorSetRGB(100, 24, 8),
                SkColorSetRGB(109, 20, 10),
                SkColorSetRGB(120, 17, 13),
                SkColorSetRGB(134, 13, 17),
                SkColorSetRGB(147, 10, 20),
                SkColorSetRGB(160, 8, 24),
                SkColorSetRGB(175, 5, 30),
                SkColorSetRGB(187, 4, 35),
                SkColorSetRGB(203, 2, 41),
                SkColorSetRGB(221, 1, 47),
                SkColorSetRGB(235, 1, 55)
        },
};

const SkColor kActual[16][16] = {
        {
                SkColorSetRGB(1, 235, 55),
                SkColorSetRGB(1, 222, 62),
                SkColorSetRGB(2, 206, 71),
                SkColorSetRGB(4, 188, 79),
                SkColorSetRGB(5, 177, 90),
                SkColorSetRGB(8, 161, 101),
                SkColorSetRGB(10, 147, 111),
                SkColorSetRGB(13, 133, 121),
                SkColorSetRGB(17, 121, 133),
                SkColorSetRGB(20, 111, 147),
                SkColorSetRGB(24, 101, 161),
                SkColorSetRGB(30, 90, 177),
                SkColorSetRGB(35, 79, 188),
                SkColorSetRGB(41, 71, 206),
                SkColorSetRGB(47, 62, 222),
                SkColorSetRGB(55, 55, 235)
        },

        {
                SkColorSetRGB(1, 222, 47),
                SkColorSetRGB(2, 206, 55),
                SkColorSetRGB(4, 188, 62),
                SkColorSetRGB(5, 177, 71),
                SkColorSetRGB(8, 161, 79),
                SkColorSetRGB(10, 147, 90),
                SkColorSetRGB(13, 133, 101),
                SkColorSetRGB(17, 121, 111),
                SkColorSetRGB(20, 111, 121),
                SkColorSetRGB(24, 101, 133),
                SkColorSetRGB(30, 90, 147),
                SkColorSetRGB(35, 79, 161),
                SkColorSetRGB(41, 71, 177),
                SkColorSetRGB(47, 62, 188),
                SkColorSetRGB(55, 55, 206),
                SkColorSetRGB(62, 47, 222)
        },

        {
                SkColorSetRGB(2, 206, 41),
                SkColorSetRGB(4, 188, 47),
                SkColorSetRGB(5, 177, 55),
                SkColorSetRGB(8, 161, 62),
                SkColorSetRGB(10, 147, 71),
                SkColorSetRGB(13, 133, 79),
                SkColorSetRGB(17, 121, 90),
                SkColorSetRGB(20, 111, 101),
                SkColorSetRGB(24, 101, 111),
                SkColorSetRGB(30, 90, 121),
                SkColorSetRGB(35, 79, 133),
                SkColorSetRGB(41, 71, 147),
                SkColorSetRGB(47, 62, 161),
                SkColorSetRGB(55, 55, 177),
                SkColorSetRGB(62, 47, 188),
                SkColorSetRGB(71, 41, 206)
        },

        {
                SkColorSetRGB(4, 188, 35),
                SkColorSetRGB(5, 177, 41),
                SkColorSetRGB(8, 161, 47),
                SkColorSetRGB(10, 147, 55),
                SkColorSetRGB(13, 133, 62),
                SkColorSetRGB(17, 121, 71),
                SkColorSetRGB(20, 111, 79),
                SkColorSetRGB(24, 101, 90),
                SkColorSetRGB(30, 90, 101),
                SkColorSetRGB(35, 79, 111),
                SkColorSetRGB(41, 71, 121),
                SkColorSetRGB(47, 62, 133),
                SkColorSetRGB(55, 55, 147),
                SkColorSetRGB(62, 47, 161),
                SkColorSetRGB(71, 41, 177),
                SkColorSetRGB(79, 35, 188)
        },

        {
                SkColorSetRGB(5, 177, 30),
                SkColorSetRGB(8, 161, 35),
                SkColorSetRGB(10, 147, 41),
                SkColorSetRGB(13, 133, 47),
                SkColorSetRGB(17, 121, 55),
                SkColorSetRGB(20, 111, 62),
                SkColorSetRGB(24, 101, 71),
                SkColorSetRGB(30, 90, 79),
                SkColorSetRGB(35, 79, 90),
                SkColorSetRGB(41, 71, 101),
                SkColorSetRGB(47, 62, 111),
                SkColorSetRGB(55, 55, 121),
                SkColorSetRGB(62, 47, 133),
                SkColorSetRGB(71, 41, 147),
                SkColorSetRGB(79, 35, 161),
                SkColorSetRGB(90, 30, 177)
        },

        {
                SkColorSetRGB(8, 161, 24),
                SkColorSetRGB(10, 147, 30),
                SkColorSetRGB(13, 133, 35),
                SkColorSetRGB(17, 121, 41),
                SkColorSetRGB(20, 111, 47),
                SkColorSetRGB(24, 101, 55),
                SkColorSetRGB(30, 90, 62),
                SkColorSetRGB(35, 79, 71),
                SkColorSetRGB(41, 71, 79),
                SkColorSetRGB(47, 62, 90),
                SkColorSetRGB(55, 55, 101),
                SkColorSetRGB(62, 47, 111),
                SkColorSetRGB(71, 41, 121),
                SkColorSetRGB(79, 35, 133),
                SkColorSetRGB(90, 30, 147),
                SkColorSetRGB(101, 24, 161)
        },

        {
                SkColorSetRGB(10, 147, 20),
                SkColorSetRGB(13, 133, 24),
                SkColorSetRGB(17, 121, 30),
                SkColorSetRGB(20, 111, 35),
                SkColorSetRGB(24, 101, 41),
                SkColorSetRGB(30, 90, 47),
                SkColorSetRGB(35, 79, 55),
                SkColorSetRGB(41, 71, 62),
                SkColorSetRGB(47, 62, 71),
                SkColorSetRGB(55, 55, 79),
                SkColorSetRGB(62, 47, 90),
                SkColorSetRGB(71, 41, 101),
                SkColorSetRGB(79, 35, 111),
                SkColorSetRGB(90, 30, 121),
                SkColorSetRGB(101, 24, 133),
                SkColorSetRGB(111, 20, 147)
        },

        {
                SkColorSetRGB(13, 133, 17),
                SkColorSetRGB(17, 121, 20),
                SkColorSetRGB(20, 111, 24),
                SkColorSetRGB(24, 101, 30),
                SkColorSetRGB(30, 90, 35),
                SkColorSetRGB(35, 79, 41),
                SkColorSetRGB(41, 71, 47),
                SkColorSetRGB(47, 62, 55),
                SkColorSetRGB(55, 55, 62),
                SkColorSetRGB(62, 47, 71),
                SkColorSetRGB(71, 41, 79),
                SkColorSetRGB(79, 35, 90),
                SkColorSetRGB(90, 30, 101),
                SkColorSetRGB(101, 24, 111),
                SkColorSetRGB(111, 20, 121),
                SkColorSetRGB(121, 17, 133)
        },

        {
                SkColorSetRGB(17, 121, 13),
                SkColorSetRGB(20, 111, 17),
                SkColorSetRGB(24, 101, 20),
                SkColorSetRGB(30, 90, 24),
                SkColorSetRGB(35, 79, 30),
                SkColorSetRGB(41, 71, 35),
                SkColorSetRGB(47, 62, 41),
                SkColorSetRGB(55, 55, 47),
                SkColorSetRGB(62, 47, 55),
                SkColorSetRGB(71, 41, 62),
                SkColorSetRGB(79, 35, 71),
                SkColorSetRGB(90, 30, 79),
                SkColorSetRGB(101, 24, 90),
                SkColorSetRGB(111, 20, 101),
                SkColorSetRGB(121, 17, 111),
                SkColorSetRGB(133, 13, 121)
        },

        {
                SkColorSetRGB(20, 111, 10),
                SkColorSetRGB(24, 101, 13),
                SkColorSetRGB(30, 90, 17),
                SkColorSetRGB(35, 79, 20),
                SkColorSetRGB(41, 71, 24),
                SkColorSetRGB(47, 62, 30),
                SkColorSetRGB(55, 55, 35),
                SkColorSetRGB(62, 47, 41),
                SkColorSetRGB(71, 41, 47),
                SkColorSetRGB(79, 35, 55),
                SkColorSetRGB(90, 30, 62),
                SkColorSetRGB(101, 24, 71),
                SkColorSetRGB(111, 20, 79),
                SkColorSetRGB(121, 17, 90),
                SkColorSetRGB(133, 13, 101),
                SkColorSetRGB(147, 10, 111)
        },

        {
                SkColorSetRGB(24, 101, 8),
                SkColorSetRGB(30, 90, 10),
                SkColorSetRGB(35, 79, 13),
                SkColorSetRGB(41, 71, 17),
                SkColorSetRGB(47, 62, 20),
                SkColorSetRGB(55, 55, 24),
                SkColorSetRGB(62, 47, 30),
                SkColorSetRGB(71, 41, 35),
                SkColorSetRGB(79, 35, 41),
                SkColorSetRGB(90, 30, 47),
                SkColorSetRGB(101, 24, 55),
                SkColorSetRGB(111, 20, 62),
                SkColorSetRGB(121, 17, 71),
                SkColorSetRGB(133, 13, 79),
                SkColorSetRGB(147, 10, 90),
                SkColorSetRGB(161, 8, 101)
        },

        {
                SkColorSetRGB(30, 90, 5),
                SkColorSetRGB(35, 79, 8),
                SkColorSetRGB(41, 71, 10),
                SkColorSetRGB(47, 62, 13),
                SkColorSetRGB(55, 55, 17),
                SkColorSetRGB(62, 47, 20),
                SkColorSetRGB(71, 41, 24),
                SkColorSetRGB(79, 35, 30),
                SkColorSetRGB(90, 30, 35),
                SkColorSetRGB(101, 24, 41),
                SkColorSetRGB(111, 20, 47),
                SkColorSetRGB(121, 17, 55),
                SkColorSetRGB(133, 13, 62),
                SkColorSetRGB(147, 10, 71),
                SkColorSetRGB(161, 8, 79),
                SkColorSetRGB(177, 5, 90)
        },

        {
                SkColorSetRGB(35, 79, 4),
                SkColorSetRGB(41, 71, 5),
                SkColorSetRGB(47, 62, 8),
                SkColorSetRGB(55, 55, 10),
                SkColorSetRGB(62, 47, 13),
                SkColorSetRGB(71, 41, 17),
                SkColorSetRGB(79, 35, 20),
                SkColorSetRGB(90, 30, 24),
                SkColorSetRGB(101, 24, 30),
                SkColorSetRGB(111, 20, 35),
                SkColorSetRGB(121, 17, 41),
                SkColorSetRGB(133, 13, 47),
                SkColorSetRGB(147, 10, 55),
                SkColorSetRGB(161, 8, 62),
                SkColorSetRGB(177, 5, 71),
                SkColorSetRGB(188, 4, 79)
        },

        {
                SkColorSetRGB(41, 71, 2),
                SkColorSetRGB(47, 62, 4),
                SkColorSetRGB(55, 55, 5),
                SkColorSetRGB(62, 47, 8),
                SkColorSetRGB(71, 41, 10),
                SkColorSetRGB(79, 35, 13),
                SkColorSetRGB(90, 30, 17),
                SkColorSetRGB(101, 24, 20),
                SkColorSetRGB(111, 20, 24),
                SkColorSetRGB(121, 17, 30),
                SkColorSetRGB(133, 13, 35),
                SkColorSetRGB(147, 10, 41),
                SkColorSetRGB(161, 8, 47),
                SkColorSetRGB(177, 5, 55),
                SkColorSetRGB(188, 4, 62),
                SkColorSetRGB(206, 2, 71)
        },

        {
                SkColorSetRGB(47, 62, 1),
                SkColorSetRGB(55, 55, 2),
                SkColorSetRGB(62, 47, 4),
                SkColorSetRGB(71, 41, 5),
                SkColorSetRGB(79, 35, 8),
                SkColorSetRGB(90, 30, 10),
                SkColorSetRGB(101, 24, 13),
                SkColorSetRGB(111, 20, 17),
                SkColorSetRGB(121, 17, 20),
                SkColorSetRGB(133, 13, 24),
                SkColorSetRGB(147, 10, 30),
                SkColorSetRGB(161, 8, 35),
                SkColorSetRGB(177, 5, 41),
                SkColorSetRGB(188, 4, 47),
                SkColorSetRGB(206, 2, 55),
                SkColorSetRGB(222, 1, 62)
        },

        {
                SkColorSetRGB(55, 55, 1),
                SkColorSetRGB(62, 47, 1),
                SkColorSetRGB(71, 41, 2),
                SkColorSetRGB(79, 35, 4),
                SkColorSetRGB(90, 30, 5),
                SkColorSetRGB(101, 24, 8),
                SkColorSetRGB(111, 20, 10),
                SkColorSetRGB(121, 17, 13),
                SkColorSetRGB(133, 13, 17),
                SkColorSetRGB(147, 10, 20),
                SkColorSetRGB(161, 8, 24),
                SkColorSetRGB(177, 5, 30),
                SkColorSetRGB(188, 4, 35),
                SkColorSetRGB(206, 2, 41),
                SkColorSetRGB(222, 1, 47),
                SkColorSetRGB(235, 1, 55)
        },
};

static void write(const char* label, const SkPixmap& pm) {
    SkString filename;
    filename.appendf("//Users//robertphillips//src//bugs//%s.png", label);

    SkFILEWStream file(filename.c_str());
    SkAssertResult(file.isValid());

    SkAssertResult(SkPngEncoder::Encode(&file, pm, {}));
}

static SkBitmap make(const char* label, const SkColor colors[16][16]) {
    SkImageInfo ii = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            SkColor c = colors[y][x];
            *bm.pixmap().writable_addr32(x, y) = c;
        }
    }

    write(label, bm.pixmap());

    return bm;
}

void diff1(const SkPixmap& expected, const SkPixmap& actual) {
    SkImageInfo ii = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    int maxR = 0, maxG = 0, maxB = 0;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            SkColor e = expected.getColor(x, y);
            SkColor a = actual.getColor(x, y);
            int rE = SkColorGetR(e);
            int gE = SkColorGetG(e);
            int bE = SkColorGetB(e);
            int rA = SkColorGetR(a);
            int gA = SkColorGetG(a);
            int bA = SkColorGetB(a);
            int dR = std::abs(rE - rA);
            int dG = std::abs(gE - gA);
            int dB = std::abs(bE - bA);
            if (maxR < dR) maxR = dR;
            if (maxG < dG) maxG = dG;
            if (maxB < dB) maxB = dB;

            *bm.pixmap().writable_addr32(x, y) = SkColorSetRGB(dR * 85,
                                                               dG * 85,
                                                               dB * 85);
        }
    }

    SkDebugf("max %d %d %d\n", maxR, maxG, maxB);

    write("diff", bm.pixmap());
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
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(1000, 1000); }


    void onOnceBeforeDraw() override {
        fExpected = make("expected", kExpected);
        fActual = make("actual", kActual);
        diff1(fExpected.pixmap(), fActual.pixmap());
    }

    void onDraw(SkCanvas* canvas) override {

    }

private:
    SkBitmap fExpected;
    SkBitmap fActual;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
