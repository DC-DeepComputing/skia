Skia now always builds itself against a checked in webgpu_cpp.h that has Dawn defintions that
extend the webgpu spec. However, Skia's public interface only refers to proper webgpu
interfaces. So when client code is compiled <webgpu/webgpu_cpp.h> is included by Skia
headers. This allows building Graphite's Dawn backend in emscripten without using macros
to conditionally refer to Dawn extensions.
