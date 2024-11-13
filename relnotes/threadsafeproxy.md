
A new ThreadSafeProxy object has been added to assist Precompilation. The old API of the form:
    bool Precompile(Context*, ...);
has been deprecated and replaced with the API:
    bool Precompile(sk_sp<ThreadSafeProxy>, ...)
The new ThreadSafeProxy object can be obtained via the Context::makeProxy call.

The general threading model in Skia, for Compilation and Precompilation, involves 4 threads: 
2 for creating Recordings (<r1> and <r2>), 1 for precompiling (<p1>) and the main thread - which 
owns the Context and submits Recordings. Start up for this scenario would look like:

  the main thread moves a ThreadSafeProxy to <p1> and begins precompiling there
  the main thread creates two Recorders and moves them to <r1> and <r2> to create Recordings
  the main thread continues on - calling Context::insertRecording on the posted Recordings.

The ThreadSafeProxy will manage the lifetime of the Context objects needed for precompilation.
