# Skottie (`modules/skottie/`)

## Overview

`modules/skottie/` (`namespace skottie`) is Skia's native C++ player for Lottie (Bodymovin) JSON vector animations exported from Adobe After Effects. It translates parsed JSON animations into a retained scene graph and evaluates keyframe animators per frame:

* **Animation Building & Playback**: Parses Lottie JSON streams into a retained [`../sksg/`](../sksg/) render tree and evaluates keyframe interpolators on `seekFrame()` or `seekFrameTime()` with dirty-region tracking.
* **Dynamic Property & Slot Binding**: Exposes runtime inspection and mutation of named animation nodes (`PropertyObserver`, `PropertyHandle`) and keyed slot properties (`SlotManager`).
* **Compositions, Layers & Effects**: Translates After Effects precomps, shape layers, text layers, footage layers, matte/clip masks, 2D/3D camera transforms, and layer effects/styles into scene graph nodes and runtime shaders.
* **Text Shaping & Asset Integration**: Implements After Effects text box auto-sizing, vertical alignment, and per-glyph animator fragmentation on top of [`../skshaper/`](../skshaper/) and resolves external images, fonts, and audio via [`../skresources/`](../skresources/).

## Subdirectories

| Directory | Description |
| :--- | :--- |
| [`include/`](include/) | Public `skottie` API headers for animation building, playback, property observers, slot management, precomp interception, and text shaping. |
| [`src/`](src/) | Core animation builder, scene graph adapters, composition/layer/transform builders (`layers/`, `effects/`, `animator/`, `text/`), and CLI rendering tools. |
| [`utils/`](utils/) | Helper utilities for property editing (`SkottieUtils.h`), interactive text editing (`TextEditor.h`), and offline text pre-shaping (`TextPreshape.h`). |
| [`gm/`](gm/) | Golden Master rendering tests for Lottie animations and features. |
| [`tests/`](tests/) | Unit tests for animation builders, keyframe animators, property handles, and text shaping. |
| [`fuzz/`](fuzz/) | Fuzz targets for Lottie JSON parsing and playback. |

## Key Classes and Entrypoints

The primary classes and entrypoints in `modules/skottie/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Animation & Builder** | [`Skottie.h`](include/Skottie.h), [`Skottie.cpp`](src/Skottie.cpp), [`SkottiePriv.h`](src/SkottiePriv.h) | Reference-counted animation player (`Animation`), configurable builder (`Animation::Builder`), timeline/diagnostic callbacks (`Logger`, `MarkerObserver`, `ExpressionManager`), and internal scene builder (`internal::AnimationBuilder`). |
| **Dynamic Properties & Slots** | [`SkottieProperty.h`](include/SkottieProperty.h), [`SlotManager.h`](include/SlotManager.h), [`ExternalLayer.h`](include/ExternalLayer.h) | Runtime node mutation handles (`PropertyObserver`, `PropertyHandle`), slot-ID property binding (`SlotManager`), and custom precomp layer substitution (`PrecompInterceptor`, `ExternalLayer`). |
| **Text Shaping & Animators** | [`TextShaper.h`](include/TextShaper.h), [`TextAdapter.h`](src/text/TextAdapter.h), [`TextAnimator.h`](src/text/TextAnimator.h), [`RangeSelector.h`](src/text/RangeSelector.h) | After Effects text layout and auto-scaling (`Shaper`), scene graph text binding (`internal::TextAdapter`), and per-character/word/line range selectors. |
| **Compositions, Layers & Camera** | [`Composition.h`](src/Composition.h), [`Layer.h`](src/Layer.h), [`ShapeLayer.h`](src/layers/shapelayer/ShapeLayer.h), [`Transform.h`](src/Transform.h), [`Camera.h`](src/Camera.h) | Composition and layer tree construction (`CompositionBuilder`, `LayerBuilder`), vector shape geometry/paint builders (`ShapeBuilder`), and 2D/3D transform and camera adapters (`TransformAdapter2D`, `TransformAdapter3D`, `CameraAdaper`). |
| **Keyframe Animators & Effects** | [`Animator.h`](src/animator/Animator.h), [`KeyframeAnimator.h`](src/animator/KeyframeAnimator.h), [`Adapter.h`](src/Adapter.h), [`Effects.h`](src/effects/Effects.h) | Keyframe interpolation (`Animator`, `KeyframeAnimator`), property-to-scene-node binding (`AnimatablePropertyContainer`, `DiscardableAdapterBase`), and After Effects layer effects/styles (`EffectBuilder`). |

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../sksg/`](../sksg/) | Retained-mode 2D scene graph and dirty-region invalidation backing `skottie::Animation`. |
| [`../skresources/`](../skresources/) | External image, multi-frame animation, audio, and font resource providers. |
| [`../jsonreader/`](../jsonreader/) | Arena-backed JSON DOM parser used to ingest Lottie animation files. |
| [`../skshaper/`](../skshaper/) | OpenType text shaping engine used by `skottie::Shaper`. |
| [`../skunicode/`](../skunicode/) | Unicode BiDi and line-breaking support for dynamic text layers. |
