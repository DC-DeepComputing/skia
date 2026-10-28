---
name: skia/group-operational-methods-in-priv-classes
description: When declaring methods in internal Priv classes, group operational behavioral methods together separately from simple inline subsystem and member accessors
rubric: readability
severity: should_fix
---

**Rule.** In internal privileged classes (such as companion `*Priv` headers in
`src/`), group operational and behavioral methods representing private APIs
together, separate from simple inline member and subsystem accessors (getters).
Do not scatter behavioral methods or factory operations among lists of inline
field accessors.

**Why.** Placing operational methods in a distinct section separates Skia's
internal behavioral API from routine getter boilerplate. Scattering operational
methods among trivial inline accessors obscures key behavioral operations,
reduces interface scannability, and makes private capabilities harder to
discover.

## Examples

Bad:

```cpp
class RecorderPriv {
    UploadBufferManager* uploadBufferManager() { return fRecorder->fUploadBufferManager.get(); }
    sk_sp<FloatStorageManager> refFloatStorageManager() { return fRecorder->fFloatStorageManager; }

    // Anti-pattern: operational methods scattered inside accessor list
    std::unique_ptr<KeyAndDataBuilder> popOrCreateKeyAndDataBuilder() {
        return fRecorder->popOrCreateKeyAndDataBuilder();
    }
    void pushKeyAndDataBuilder(std::unique_ptr<KeyAndDataBuilder> keyDB) {
        return fRecorder->pushKeyAndDataBuilder(std::move(keyDB));
    }

    AtlasProvider* atlasProvider() { return fRecorder->fAtlasProvider.get(); }
};
```

Good:

```cpp
class RecorderPriv {
    // Group operational / behavioral private APIs together first
    void flushTrackedDevices(const TextureProxy* dependency);
    std::unique_ptr<KeyAndDataBuilder> popOrCreateKeyAndDataBuilder();
    void pushKeyAndDataBuilder(std::unique_ptr<KeyAndDataBuilder> keyDB);

    // Group simple inline subsystem and member accessors together
    const Caps* caps() const { return fRecorder->fSharedContext->caps(); }
    UploadBufferManager* uploadBufferManager() { return fRecorder->fUploadBufferManager.get(); }
    sk_sp<FloatStorageManager> refFloatStorageManager() { return fRecorder->fFloatStorageManager; }
    AtlasProvider* atlasProvider() { return fRecorder->fAtlasProvider.get(); }
};
```
