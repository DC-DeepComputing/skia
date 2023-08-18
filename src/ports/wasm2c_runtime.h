#ifndef _INCLUDED_BUILD_TOOLCHAIN_WASM2C_WASM2C_RUNTIME_H_
#define _INCLUDED_BUILD_TOOLCHAIN_WASM2C_WASM2C_RUNTIME_H_

/* wasm2c_runtime.h */
#include "wasm-rt.h"
#ifdef __cplusplus
extern "C" {
#endif

struct w2c_wasi__snapshot__preview1 {
  wasm_rt_memory_t* memory_for_imports;
};

void wasmbox_runtime_init(void);

#ifdef __cplusplus
}
#endif

#endif  // _INCLUDED_BUILD_TOOLCHAIN_WASM2C_WASM2C_RUNTIME_H_
