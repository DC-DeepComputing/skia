/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "include/core/SkCanvas.h"
#include "modules/androidkit/src/Utils.h"

#include <jni.h>

namespace {
static jmethodID gMethodCount;
static jmethodID gMethodGetAt;
static jmethodID gMethodFontSize;
static jmethodID gMethodLocale;

class FontChainAdapter : public skia::text::FontChain {
    public:
        FontChainAdapter(JNIEnv* env, jobject jFontChain)
            : fEnv(env)
            , fFontChainImpl(jFontChain)
        { }

        size_t count() const override {
            return fEnv->CallIntMethod(fFontChainImpl, gMethodCount);
        }

        sk_sp<SkTypeface> operator[](size_t index) const  override {
            SkFont* font = reinterpret_cast<SkFont*>(fEnv->CallLongMethod(fFontChainImpl, gMethodGetAt, index));
            return sk_ref_sp(font->getTypeface());
        }

        float fontSize() const override {
            return fEnv->CallFloatMethod(fFontChainImpl, gMethodFontSize);
        }

        SkString locale() const override {
            jstring jLocale = (jstring)fEnv->CallObjectMethod(fFontChainImpl, gMethodLocale);

            const androidkit::utils::CString cstr(fEnv, jLocale);
            SkString str(cstr);
            return str;
        }

    private:
        // TODO: Check, can we hold on to env here or do we have to get it every time?
        JNIEnv* fEnv;
        jobject fFontChainImpl;
};

static jlong FontChain_Create(JNIEnv* env, jobject, jobject jFontChain) {
    return reinterpret_cast<jlong>(new FontChainAdapter(env, jFontChain));
}

static void FontChain_Release(JNIEnv* env, jobject, jlong native_FontChain) {
    delete reinterpret_cast<FontChainAdapter*>(native_FontChain);
}

} // namespace

int register_androidkit_FontChain(JNIEnv* env) {
    const auto clazz = env->FindClass("org/skia/androidkit/FontChain");

    gMethodCount = env->GetMethodID(clazz, "count", "()I");
    gMethodGetAt = env->GetMethodID(clazz, "getAt", "(I)J");
    gMethodFontSize = env->GetMethodID(clazz, "fontSize", "()F");
    gMethodLocale = env->GetMethodID(clazz, "locale", "()Ljava/lang/String;");

    static const JNINativeMethod methods[] = {
        {"nCreate" , "(Lorg/skia/androidkit/FontChain;)J", reinterpret_cast<void*>(FontChain_Create)},
        {"nRelease", "(J)V",                               reinterpret_cast<void*>(FontChain_Release)},
    };

    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
