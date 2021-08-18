/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <jni.h>

#include "experimental/sktext/src/Paint.h"
#include "modules/androidkit/src/Utils.h"
#include <string>

using namespace skia::text;

namespace {

static void Text_RenderText(JNIEnv* env, jobject, jstring jtext, jlong native_canvas, jfloat x, jfloat y) {
    const androidkit::utils::U16String text(env, jtext);
    std::u16string u16str(text);
    if (auto* canvas = reinterpret_cast<SkCanvas*>(native_canvas)) {
        //TODO: replace with actual text passed in
        Paint::drawText(u"Hello word", canvas, x, y);
    }
}

} //namespace

int register_androidkit_Text(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"nRenderText", "(Ljava/lang/String;JFF)V", reinterpret_cast<void*>(Text_RenderText)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Text");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
