---
name: skia/gpu/use-distinct-labels-for-resource-instances
description: When instantiating multiple GPU resources, atlases, or texture proxies, assign unique descriptive debug labels to each instance rather than sharing generic names
rubric: readability
severity: should_fix
---

**Rule.** When initializing multiple GPU resources, atlas managers, or texture
proxies (such as DrawAtlas or TextureProxy instances with different keying
strategies or lifetimes), assign distinct, descriptive debug labels to each
instance. Do not reuse an identical generic label across multiple distinct
resource managers.

**Why.** Resource and atlas labels propagate to underlying GPU textures and
driver objects for tracking in GPU debuggers (such as RenderDoc and Xcode),
Tracy profilers, and memory dumps. Sharing generic names makes it impossible to
differentiate between instances with different lifetimes, keying schemes, or
memory footprints.

## Examples

Bad:

```cpp
ClipAtlasManager::ClipAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fPathKeyAtlasMgr(kClipAtlasWidth, kClipAtlasHeight,
                           /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                           DrawAtlas::UseStorageTextures::kNo,
                           "ClipAtlas", recorder->priv().caps())
        , fSaveRecordKeyAtlasMgr(kClipAtlasWidth/2, kClipAtlasHeight/2,
                                 /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                                 DrawAtlas::UseStorageTextures::kNo,
                                 "ClipAtlas", recorder->priv().caps()) {}
```

Good:

```cpp
ClipAtlasManager::ClipAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fPathKeyAtlasMgr(kClipAtlasWidth, kClipAtlasHeight,
                           /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                           DrawAtlas::UseStorageTextures::kNo,
                           "PathKeyClipAtlas", recorder->priv().caps())
        , fSaveRecordKeyAtlasMgr(kClipAtlasWidth/2, kClipAtlasHeight/2,
                                 /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                                 DrawAtlas::UseStorageTextures::kNo,
                                 "SaveRecordKeyClipAtlas", recorder->priv().caps()) {}
```
