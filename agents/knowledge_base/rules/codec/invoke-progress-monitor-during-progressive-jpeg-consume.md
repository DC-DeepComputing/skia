---
name: skia/codec/invoke-progress-monitor-during-progressive-jpeg-consume
description: When consuming progressive JPEG scans in a libjpeg consume loop, explicitly invoke the registered progress_monitor callback to prevent DoS hangs
rubric: safety
severity: should_fix
---

**Rule.** When consuming progressive JPEG input scans using jpeg_consume_input
in libjpeg, explicitly invoke the registered progress monitor callback
(dinfo->progress->progress_monitor((j_common_ptr)dinfo)) inside the loop before
consuming input. Do not rely on libjpeg to trigger the progress manager
automatically during scan consumption.

**Why.** Libjpeg's jpeg_consume_input does not automatically invoke
jpeg_progress_mgr callbacks, unlike scanline decoding passes. Without manual
progress monitor invocation, progressive images with an excessive number of
scans can bypass scan limits and hang the decoder, causing denial-of-service
vulnerabilities.

## Examples

Bad:

```cpp
while (!jpeg_input_complete(dinfo)) {
    const int res = jpeg_consume_input(dinfo);
    if (res == JPEG_SUSPENDED) {
        break;
    }
    if (res == JPEG_SCAN_COMPLETED) {
        last_scan_completed = dinfo->input_scan_number;
    }
}
```

Good:

```cpp
while (!jpeg_input_complete(dinfo)) {
    if (dinfo->progress) {
        dinfo->progress->progress_monitor((j_common_ptr)dinfo);
    }
    const int res = jpeg_consume_input(dinfo);
    if (res == JPEG_SUSPENDED) {
        break;
    }
    if (res == JPEG_SCAN_COMPLETED) {
        last_scan_completed = dinfo->input_scan_number;
    }
}
```
