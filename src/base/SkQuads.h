/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkQuads_DEFINED
#define SkQuads_DEFINED

#include "include/private/base/SkSpan_impl.h"

/**
 * Utilities for dealing with quadratic formulas with one variable:
 *   f(t) = A*t^2 + B*t + C
 */
class SkQuads {
public:
    /**
     * Calculate a very accurate discriminant.
     * Given
     *    A*t^2 -2*B*t + C = 0,
     * calculate
     *    B^2 - AC
     * accurate to 2 bits.
     * Note the form of the quadratic is slightly different from the normal formulation.
     *
     * The method used to calculate the discriminant is from
     *    "On the Cost of Floating-Point Computation Without Extra-Precise Arithmetic"
     * by W. Kahan.
     */
    static double Discriminant(double A, double B, double C);

    struct RootResult {
        double discriminant;
        double root0;
        double root1;
    };

    /**
     * Calculate the roots of a quadratic.
     * Given
     *    A*t^2 -2*B*t + C = 0,
     * calculate the roots.
     *
     * This does not try to detect a linear configuration of the equation, or detect if the two
     * roots are the same. It returns the discriminant and the two roots.
     *
     * Not this uses a different form the quadratic equation to reduce rounding error. Give
     * standard A, B, C. You can call this root finder with:
     *    Roots(A, -0.5*B, C)
     * to find the roots of A*x^2 + B*x + C.
     *
     * The method used to calculate the roots is from
     *    "On the Cost of Floating-Point Computation Without Extra-Precise Arithmetic"
     * by W. Kahan.
     *
     * If the roots are imaginary then nan is returned.
     * If the roots can't be represented as double then inf is returned.
     */
    static RootResult Roots(double A, double B, double C);

    /**
     * Puts up to 2 real solutions to the equation
     *   A*t^2 + B*t + C = 0
     * in the provided array.
     */
    static int RootsReal(double A, double B, double C, double solution[2]);

    /**
     * Given
     *    A0*t^2 -2*B0*t + C0 = 0 and A1*t^2 - 2*B1*t + C1 = 0,
     *
     * Find the t where A0*t^2 - 2*B0*t + C = 0, then return A1*t^2 + - 2*B1*t + C1 where t is on
     * [0, 1].
     *
     * - toIntersect - is the value at which the quadratic is being intersected.
     * - intersectionsStorage - is the array to hold the return data pointed to in the span.
     *
     * Returns a span with the intersections of toIntersect, and the quadratic formed by A, B,
     * and C.
     */
    static SkSpan<double> Intersect(
        double A0, double B0, double C0,
        double A1, double B1, double C1, double toIntersect, double intersectionsStorage[2]);

    /**
     * Evaluates the quadratic function with the 3 provided coefficients and the
     * provided variable.
     */
    static double EvalAt(double A, double B, double C, double t);
};

#endif
