`SkCanvas::flush()` has been removed. It can be replaced with:
```
    if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
        dContext->flushAndSubmit();
    }
```