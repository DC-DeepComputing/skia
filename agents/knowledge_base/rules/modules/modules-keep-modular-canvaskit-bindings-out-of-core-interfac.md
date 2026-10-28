---
name: skia/modules/modules-keep-modular-canvaskit-bindings-out-of-core-interfac
description: In CanvasKit, define module-specific C++ Embind bindings and JavaScript definitions in dedicated module files rather than core canvaskit_bindings.cpp or interface.js
rubric: design
severity: should_fix
---

**Rule.** In CanvasKit, do not define module-specific enums, types, or bindings
in core files (canvaskit_bindings.cpp or interface.js). Define C++ Emscripten
bindings (EMSCRIPTEN_BINDINGS) and header includes for optional features in that
module's dedicated binding file (e.g., skottie_bindings.cpp,
paragraph_bindings.cpp). In JavaScript, place module-specific definitions in
their respective module file (such as bidi.js or paragraph.js) within
CanvasKit._extraInitializations, and guard shared enums with an existence check
(e.g., if (!CanvasKit['<Enum>'])) when another module or Embind C++ binding
might also define them.

**Why.** CanvasKit supports modular and stripped-down builds (e.g., no_skottie,
no_font) that exclude optional modules to minimize binary size. Registering
module-specific types, enums, or includes in core binding files pulls module
dependencies into the core target, bloats minimal builds, and causes duplicate
definition conflicts or compilation failures when modules are omitted.

## Examples

Bad:

```cpp
// modules/canvaskit/canvaskit_bindings.cpp
#include "tools/skui/InputState.h"
EMSCRIPTEN_BINDINGS(Skia) {
    enum_<skui::InputState>("InputState")
        .value("Down", skui::InputState::kDown);
}

// modules/canvaskit/interface.js
CanvasKit['TextDirection'] = { 'LTR': 1, 'RTL': 0 };
```

Good:

```cpp
// modules/canvaskit/skottie_bindings.cpp
#include "tools/skui/InputState.h"
EMSCRIPTEN_BINDINGS(Skottie) {
    enum_<skui::InputState>("InputState")
        .value("Down", skui::InputState::kDown);
}

// modules/canvaskit/bidi.js
CanvasKit._extraInitializations.push(function() {
  if (!CanvasKit['TextDirection']) {
    CanvasKit['TextDirection'] = { 'LTR': 1, 'RTL': 0 };
  }
});
```
