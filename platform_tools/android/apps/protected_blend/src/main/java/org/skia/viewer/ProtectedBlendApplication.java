/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.pblend;

import android.app.Application;
import android.content.Intent;
import android.content.res.AssetManager;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class ProtectedBlendApplication extends Application {
    private long mNativeHandle = 0;
    private ProtectedBlendActivity mProtectedBlendActivity;

    static {
        System.loadLibrary("protected_blend");
    }

    private native long createNativeApp(AssetManager assetManager, ByteBuffer[] args);
    private native void destroyNativeApp(long handle);

    @Override
    public void onTerminate() {
        if (mNativeHandle != 0) {
            destroyNativeApp(mNativeHandle);
            mNativeHandle = 0;
        }
        super.onTerminate();
    }

    public long getNativeHandle() {
        return mNativeHandle;
    }

    public void setProtectedBlendActivity(ProtectedBlendActivity protectedTestActivity) {
        mProtectedBlendActivity = protectedTestActivity;
        // Note that protectedTestActivity might be null (called by onDestroy)
        if (mProtectedBlendActivity != null) {
            // A new ProtectedBlendActivity is created; initialize its state and title
            // if (mStateJsonStr != null) {
            //     mProtectedBlendActivity.setState(mStateJsonStr);
            // }
            if (mNativeHandle == 0) {
                final ByteBuffer[] byteBufferArgs;
                Intent intent = protectedTestActivity.getIntent();
                String args = intent.getStringExtra("args");
                if (args == null) {
                    byteBufferArgs = new ByteBuffer[0];
                } else {
                    //TODO: split args better?
                    String[] splitArgs = args.split("\\s+");
                    byteBufferArgs = new ByteBuffer[splitArgs.length];
                    for (int i = 0; i < splitArgs.length; ++i) {
                        ByteBuffer bbArg = StandardCharsets.UTF_8.encode(splitArgs[i]);
                        ByteBuffer directBBArg = ByteBuffer.allocateDirect(bbArg.limit() + 1);
                        for (int j = 0; j < bbArg.limit(); ++j) {
                            directBBArg.put(j, bbArg.get(j));
                        }
                        // UTF-8 doens't allow a zero terminator, but args require it.
                        directBBArg.put(bbArg.limit(), (byte)0);
                        byteBufferArgs[i] = directBBArg;
                    }
                }
                mNativeHandle = createNativeApp(this.getResources().getAssets(), byteBufferArgs);
            }
        }
    }

    public void setTitle(String title) {
        // no-op
        // if (mProtectedBlendActivity != null) {
        //     mProtectedBlendActivity.runOnUiThread(new Runnable() {
        //         @Override
        //         public void run() {
        //             // no-op
        //         }
        //     });
        // }
    }

    public void setState(String stateJsonStr) {
        // We have to store this state because ProtectedBlendActivity may be destroyed while the native app
        // is still running. When a new ProtectedBlendActivity is created, we'll pass the state to it.
        // no-op
    }
}
