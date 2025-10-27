/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.pblend;

import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ListView;

public class ProtectedBlendActivity
        extends Activity implements SurfaceHolder.Callback {
    private static final float FLING_VELOCITY_THRESHOLD = 1000;

    private ProtectedBlendApplication mApplication;
    private FrameLayout mSurfaceContainer;

    private native void onSurfaceCreated(long handle, Surface surface);
    private native void onSurfaceChanged(long handle, Surface surface);
    private native void onSurfaceDestroyed(long handle);

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.title, menu);
        return true;
    }

    private void createSurfaceView() {
        SurfaceView surfaceView = new SurfaceView(this);
        surfaceView.setSecure(true);
        surfaceView.getHolder().addCallback(this);
        mSurfaceContainer.addView(surfaceView);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSurfaceContainer = (FrameLayout) findViewById(R.id.mainLayout);
        createSurfaceView();

        mApplication = (ProtectedBlendApplication) getApplication();
        mApplication.setProtectedBlendActivity(this);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            mSurfaceContainer.removeAllViews();
            createSurfaceView();
        }
        return super.onTouchEvent(event);
    }

    @Override
    protected void onDestroy() {}


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceCreated(mApplication.getNativeHandle(), holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceChanged(mApplication.getNativeHandle(), holder.getSurface());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceDestroyed(mApplication.getNativeHandle());
        }
    }
}
