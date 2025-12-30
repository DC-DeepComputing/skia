/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "include/core/SkColorSpace.h"
#include "include/core/SkM44.h"
#include "include/effects/SkGradient.h"
#include "modules/jetski/src/Utils.h"

namespace {

// Helper for common gradient data access.
class GradientData {
public:
    GradientData(JNIEnv* env, const jfloatArray& jcolors, const jfloatArray& jpos,
                 jint jtm, jlong native_lm)
        : fEnv(env)
        , fJColors(jcolors)
        , fJPos(jpos)
        , fColors(env->GetFloatArrayElements(jcolors, nullptr))
        , fPos(env->GetFloatArrayElements(jpos, nullptr))
        , fCount(env->GetArrayLength(jpos))
        , fTileMode(jetski::utils::TileMode(jtm))
        , fLocalMatrix(native_lm ? reinterpret_cast<const SkM44*>(native_lm)->asM33() : SkMatrix())
    {
        SkASSERT(env->GetArrayLength(jcolors) == 4*fCount);
    }

    ~GradientData() {
        fEnv->ReleaseFloatArrayElements(fJPos, fPos, 0);
        fEnv->ReleaseFloatArrayElements(fJColors, fColors, 0);
    }

    size_t count() const { return fCount; }

    const SkColor4f*  colors()      const { return reinterpret_cast<const SkColor4f*>(fColors); }
    const float*      pos()         const { return fPos; }
    SkTileMode        tileMode()    const { return fTileMode; }
    const SkMatrix&   localMatrix() const { return fLocalMatrix; }

    SkGradient grad() const {
        SkSpan<const float> pos;
        if (fPos) {
            pos = {fPos, fCount};
        }
        return {{this->colors(), fCount}, pos, fTileMode}, {}};
    }

private:
    JNIEnv*            fEnv;
    const jfloatArray& fJColors;
    const jfloatArray& fJPos;
    float*             fColors;
    float*             fPos;
    const size_t       fCount;
    const SkTileMode   fTileMode;
    const SkMatrix     fLocalMatrix;
};

static jlong MakeLinear(JNIEnv* env, jobject, jfloat x0, jfloat y0, jfloat x1, jfloat y1,
                        jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);
    const SkPoint pts[] = {{x0, y0}, {x1, y1}};

    auto shader = SkShaders::LinearGradient(pts, gdata.grad(), &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

static jlong MakeRadial(JNIEnv* env, jobject, jfloat x, jfloat y, jfloat r,
                        jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);

    auto shader = SkShaders::RadialGradient({x,y}, r, gdata.grad(), &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

static jlong MakeTwoPointConical(JNIEnv* env, jobject,
                                 jfloat x0, jfloat y0, jfloat r0,
                                 jfloat x1, jfloat y1, jfloat r1,
                                 jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);

    auto shader = SkShaders::TwoPointConicalGradient({x0, y0}, r0, {x1, y1}, r1,
                                                     gdata.grad(), &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

static jlong MakeSweep(JNIEnv* env, jobject, jfloat x, jfloat y, jfloat sa, jfloat ea,
                       jfloatArray jcolors, jfloatArray jpos, jint jtm, jlong native_lm) {
    const GradientData gdata(env, jcolors, jpos, jtm, native_lm);

    auto shader = SkShaders::SweepGradient({x, y}, sa, ea, gdata.grad(), &gdata.localMatrix());

    return reinterpret_cast<jlong>(shader.release());
}

} // namespace

int register_jetski_LinearGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeLinear", "(FFFF[F[FIJ)J", reinterpret_cast<void*>(MakeLinear)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/LinearGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}

int register_jetski_RadialGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeRadial", "(FFF[F[FIJ)J", reinterpret_cast<void*>(MakeRadial)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/RadialGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}

int register_jetski_TwoPointConicalGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeTwoPointConical", "(FFFFFF[F[FIJ)J", reinterpret_cast<void*>(MakeTwoPointConical)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/TwoPointConicalGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}

int register_jetski_SweepGradient(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nMakeSweep", "(FFFF[F[FIJ)J", reinterpret_cast<void*>(MakeSweep)},
    };

    const auto clazz = env->FindClass("org/skia/jetski/SweepGradient");
    return clazz
        ? env->RegisterNatives(clazz, methods, std::size(methods))
        : JNI_ERR;
}
