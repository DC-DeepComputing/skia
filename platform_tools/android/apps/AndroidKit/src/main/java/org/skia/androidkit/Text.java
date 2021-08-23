/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

public class Text {
    private long mNativeInstance;
    private String mRawText;
    private Paint mForeground;
    private Paint mBackground;

    public Text(String text) {
        mRawText = text;
        mForeground = new Paint().setColor(new Color(1,1,1,1));
        mBackground = new Paint().setColor(new Color(0,0,0,1));
    }

    public void setText(String text) {
        mRawText = text;
    }

    public void setForeground(Paint foreground) {
        mForeground = foreground;
    }

    public void setBackground(Paint background) {
        mBackground = background;
    }

    public void renderText(Canvas canvas, float x, float y) {
        nRenderText(mRawText, canvas.getNativeInstance(),
                    mForeground.getNativeInstance(), mBackground.getNativeInstance(),
                    x, y);
    }

    private static native void nRenderText(String text, long nativeCanvas, long foreground, long background, float x, float y);
}
