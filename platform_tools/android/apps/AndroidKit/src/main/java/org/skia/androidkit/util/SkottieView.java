/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit.util;

import android.content.Context;
import org.skia.androidkit.Color;
import android.view.SurfaceView;

import android.view.ViewGroup;
import android.widget.FrameLayout;
import java.io.InputStream;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.SkottieAnimation;
import org.skia.androidkit.Surface;


class SkottieRenderer extends SurfaceRenderer {
    private float mSurfaceWidth,
                  mSurfaceHeight;
    private Color mBackground;
    private SkottieAnimation mAnimation;
    private boolean mPlaying;

    SkottieRenderer(SkottieAnimation mAnimation, Color mBackground) {
        this.mAnimation = mAnimation;
        this.mBackground = mBackground;
    }
    @Override
    protected void onSurfaceInitialized(Surface surface) {
        mSurfaceWidth  = surface.getWidth();
        mSurfaceHeight = surface.getHeight();
        mPlaying = true;
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        if(mPlaying) {
            canvas.drawColor(mBackground);
            double t = (double)ms / 1000 % mAnimation.getDuration();
            mAnimation.seekTime(t);

            float s = Math.min(mSurfaceWidth / mAnimation.getWidth(),
                    mSurfaceHeight / mAnimation.getHeight());

            canvas.save();
            canvas.concat(new Matrix().translate((mSurfaceWidth  - s*mAnimation.getWidth())/2,
                    (mSurfaceHeight - s*mAnimation.getHeight())/2)
                    .scale(s, s));

            mAnimation.render(canvas);
            canvas.restore();
        }
    }

    void play() {
        if (!mPlaying) {
            mPlaying = true;
        }
    }

    void pause() {
        if (mPlaying) {
            mPlaying = false;
        }
    }

    void seekFrame(double f) {
        mAnimation.seekFrame(f);
    }
}

public class SkottieView extends FrameLayout {
    private SurfaceView mBackingView;
    private SkottieRenderer mRenderer;

    private final String LOG_TAG = "SkottieView";

    public SkottieView(Context context, int resID, Color background) {
        super(context);
        mBackingView = new SurfaceView(context);
        initBackingView();
        InputStream inputStream = context.getResources().openRawResource(resID);
        mRenderer = new SkottieRenderer(makeAnimation(inputStream), background);
        mBackingView.getHolder().addCallback(mRenderer);
    }

    private void initBackingView() {
        mBackingView.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        addView(mBackingView);
    }

    static private SkottieAnimation makeAnimation(InputStream is) {
        String json = "";
        try {
            byte[] data = new byte[is.available()];
            is.read(data);
            json = new String(data);
        } catch (Exception e) {}
        return new SkottieAnimation(json);
    }

    public void play() {
        mRenderer.play();
    }

    public void pause() {
        mRenderer.pause();
    }

    public void seekFrame(float frame) {
        mRenderer.seekFrame(frame);
    }

}
