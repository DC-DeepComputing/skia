---
name: skia/consume-cxx-bridge-types-via-free-function
description: When wrapping Rust types with by-value consuming methods across CXX FFI bridges, expose a free function accepting Box<T> rather than Option<T> and take().unwrap()
rubric: readability
severity: nit
---

**Rule.** In Rust CXX FFI bridges, when wrapping types with by-value consuming
methods (such as finish(self) or close(self)), expose a bridge free function
accepting Box<T> by value rather than wrapping the inner type in Option<T> and
taking &mut self. Avoid using Option::take() and unwrap() to extract inner types
across FFI methods.

**Why.** CXX bridge methods do not support by-value receivers like self:
Box<Self>. Wrapping an inner type in Option<T> forces all other methods on the
wrapper to unwrap the Option and risks runtime panics if called after
consumption or multiple times. Exposing a free function that takes Box<T>
leverages move semantics across C++ and Rust to enforce single-use consumption
at compile time.

## Examples

Bad:

```cpp
struct StreamWriter(Option<png::StreamWriter<'static, cxx::UniquePtr<ffi::WriteTrait>>>);

impl StreamWriter {
    pub fn write(&mut self, data: &[u8]) -> ffi::EncodingResult {
        let stream_writer = self.0.as_mut().unwrap();
        // ...
    }

    pub fn finish(&mut self) -> ffi::EncodingResult {
        self.0.take().unwrap().finish().as_ref().err().into()
    }
}
```

Good:

```cpp
struct StreamWriter(png::StreamWriter<'static, cxx::UniquePtr<ffi::WriteTrait>>);

impl StreamWriter {
    pub fn write(&mut self, data: &[u8]) -> ffi::EncodingResult {
        let io_result = self.0.write(data);
        // ...
    }
}

// In #[cxx::bridge]: fn finish_encoding(stream_writer: Box<StreamWriter>) -> EncodingResult;
fn finish_encoding(stream_writer: Box<StreamWriter>) -> ffi::EncodingResult {
    stream_writer.0.finish().as_ref().err().into()
}
```
