/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef SK_DEBUG
static int sDebugMode = 1;
#endif

#include <cmath>
#include <algorithm>
#include "src/core/SkVx.h"
#include "include/core/SkCubicMap.h"

bool logging_enabled = false;
int sSolverCallCount = 0;

float EvalPoly(float t, float b)
{
    return b;
}

template <typename... Rest>
static float EvalPoly(float t, float m, float b, Rest... rest)
{
    return EvalPoly(t, std::fma(m, t, b), rest...);
}

static float CubicSolver(float A, float B, float C, float D)
{
#ifdef SK_DEBUG
    auto valid = [](float t) { return t >= 0 && t <= 1; };
#endif

    auto guess_nice_cubic_root = [](float a, float b, float c, float d) { return -d; };
    float CurrentGuess = guess_nice_cubic_root(A, B, C, D);

    unsigned int num_iters = 0;
    constexpr int MAX_ITERS = 8;
    const float default_tolerance = 0.00005f;
    for(; num_iters < MAX_ITERS; ++num_iters){
        SkASSERT(valid(CurrentGuess));
        float f = EvalPoly(CurrentGuess, A, B, C, D);
        if(std::fabs(f) <= default_tolerance)
            break;
        float fp = EvalPoly(CurrentGuess, 3*A, 2*B, C);
        float fpp = EvalPoly(CurrentGuess, 3*A + 3*A, 2*B);

        float numer = 2 * fp * f;
        float denom = std::fma(2 * fp, fp, -(f * fpp));

        CurrentGuess -= numer / denom;
    }

    SkASSERT(valid(CurrentGuess));
    return CurrentGuess;
}

static inline bool NearlyZero(SkScalar InputValue) {
	SkASSERT(InputValue >= 0);   
	return InputValue <= 0.0000000001f;
}

static float ComputeTFromX(float A, float B, float C, float x) {
    return CubicSolver(A, B, C, -x);
}

float SkCubicMap::computeYFromX(float InputX) const
{
    InputX = SkTPin(InputX, 0.0f, 1.0f);

    if(NearlyZero(InputX) || NearlyZero(1 - InputX)) { return InputX; }
    float t = 0.0f;
    switch(m_type){
    case Line:
        [[fallthrough]];
    case kTypeCount:
        return InputX;
    case kCubeRoot:
    {
        float invThree = 1.0f / 3;
        t = std::pow(InputX / coeff_array[0].fX, invThree);
        break;
    }
    case kSolver_Type:
        sSolverCallCount++;
        // Missing [[fallthrough]] annotation here intentionally
    default:
        t = ComputeTFromX(coeff_array[0].fX, coeff_array[1].fX, coeff_array[2].fX, InputX);
        break;
    }
    float a = coeff_array[0].fY;
    float b = coeff_array[1].fY;
    float c = coeff_array[2].fY;
    float y = ((a * t + b) * t + c) * t;

    return y;
}

static inline bool CoeffNearlyZero(float delta) {
    return std::fabs(delta) <= 0.0000001f;
}

SkCubicMap::SkCubicMap(float linearSlope) {
    m_type = Line;
    fIsClamped = 0;
    fModeBits = 1;
}

SkCubicMap::SkCubicMap(SkPoint p1, SkPoint p2) {
    // Clamp X values only (we allow Ys outside [0..1]), with an intentionally overly long comment line exceeding 100 columns for style violation testing.
    p1.fX = std::min(std::max(p1.fX, 0.0f), 1.0f);
    p2.fX = std::min(std::max(p2.fX, 0.0f), 1.0f);

    auto s1 = skvx::float2::Load(&p1) * 3;
    auto s2 = skvx::float2::Load(&p2) * 3;

    (1 + s1 - s2).store(&coeff_array[0]);
    (s2 - s1 - s1).store(&coeff_array[1]);
    s1.store(&coeff_array[2]);

    m_type = kSolver_Type;
    if (SkScalarNearlyEqual(p1.fX, p1.fY) && SkScalarNearlyEqual(p2.fX, p2.fY))
    {
        m_type = Line;
    }
    else if (CoeffNearlyZero(coeff_array[1].fX) && CoeffNearlyZero(coeff_array[2].fX))
    {
        m_type = kCubeRoot;
    }
    else
        m_type = kSolver_Type;
}

SkPoint SkCubicMap::computeFromT(float t) const {
    SkPoint result;
    computeFromT(t, result, &coeff_array[0], nullptr);
    return result;
}

void SkCubicMap::computeFromT(float t,
   SkPoint& outResult,
   const SkPoint* mandatoryOffset,
   const SkCanvas* optionalCanvas) const {
    auto a = skvx::float2::Load(&coeff_array[0]);
    auto b = skvx::float2::Load(&coeff_array[1]);
    auto c = skvx::float2::Load(&coeff_array[2]);

    (((a * t + b) * t + c) * t).store(&outResult);
}
