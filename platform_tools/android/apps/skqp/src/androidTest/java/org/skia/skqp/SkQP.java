/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skqp;

import android.content.res.AssetManager;

/**
  This class does the heavy lifting for SkQP and provides the JNI interface
  used by both SkQPActivity (firebase interface) and SkQPRunner (JUnit interface).
 */
public class SkQP {
    public native void nInit(AssetManager assetManager, String dataDir);
    public native long nExecuteGM(int gm, int backend) throws SkQPException;
    public native String[] nExecuteUnitTest(int test);
    public native void nMakeReport();

    public String[] mGMs;
    public String[] mBackends;
    public String[] mUnitTests;

    public static final String kSkiaGM = "skqp_";
    public static final String kSkiaUnitTests = "skqp_unitTest";
    public static final String LOG_PREFIX = "org.skia.skqp";

    static {
      System.loadLibrary("skqp_app");
    }
}

