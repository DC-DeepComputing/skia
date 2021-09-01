package org.skia.androidkit;

public class Font {
    private long mNativeInstance;

    public Font(String fontFamily) {
        mNativeInstance = nCreate(fontFamily);
    }

    /**
     * Releases any resources associated with this Paint.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    // package private
    long getNativeInstance() { return mNativeInstance; }

    private static native long nCreate(String fontFamily);
    private static native long nRelease(long nativeFont);

}