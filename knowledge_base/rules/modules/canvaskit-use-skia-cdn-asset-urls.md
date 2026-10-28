---
name: skia/modules/canvaskit-use-skia-cdn-asset-urls
description: In CanvasKit demo and test HTML pages, load font and media assets using the Skia CDN variable (`cdn + '<asset>'`) instead of hardcoding external URLs
rubric: readability
severity: should_fix
---

**Rule.** In CanvasKit demo and test HTML pages, load font and media assets
using the Skia CDN URL variable (`cdn + '<asset_name>'`) rather than hardcoding
direct external URLs (such as `fonts.gstatic.com`). Ensure the `cdn` base URL is
defined as `'https://cdn.skia.org/misc/'` and references mirrored asset
filenames.

**Why.** Direct external URLs (such as hashed Google Fonts links) are brittle,
subject to unexpected upstream breakage or CORS restrictions, and can fail in
isolated test environments without external network access. Skia mirrors
standard demo assets on its CDN to provide stable, reproducible asset URLs
across CanvasKit examples.

## Examples

Bad:

```cpp
<script>
  const loadArabic = fetch('https://fonts.gstatic.com/s/notosansarabic/v18/nwpxtLGrOAZMl5nJ_wfgRg3DrWFZWsnVBJ_sS6tlqHHFlhQ5l3sQWIHPqzCfyGyvu3CBFQLaig.ttf')
      .then((response) => response.arrayBuffer());
</script>
```

Good:

```cpp
<script>
  const cdn = 'https://cdn.skia.org/misc/';
  const loadArabic = fetch(cdn + 'NotoSansArabic.v18.ttf')
      .then((response) => response.arrayBuffer());
</script>
```
