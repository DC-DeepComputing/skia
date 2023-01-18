/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCubics_DEFINED
#define SkCubics_DEFINED

class SkCubics {
public:
    static int RootsValidT(double A, double B, double C, double D,
                           double solution[3]);
    static int RootsReal(double A, double B, double C, double D,
                         double solution[3]);

    static double EvalAt(double coefficients[4], double t) {
        return coefficients[0] * t * t * t +
               coefficients[1] * t * t +
               coefficients[2] * t +
               coefficients[3];
    }
};

class SkCubicsXY {
public:
    static void ChopAt(double coefficients[8], double t,
                       double outputPair[14]);
};

#endif
