#ifndef FREETYPE_RT_H
#define FREETYPE_RT_H

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>
#include "libfreetype.wasm.h"

typedef struct w2c_env {
  w2c_libfreetype* inst;
} w2c_env;

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

#ifdef VERBOSE_LOGGING
#define VERBOSE_LOG(...) { printf(__VA_ARGS__); }
#else
#define VERBOSE_LOG(...)
#endif

#define IMPORT_IMPL(ret, name, params, body) \
static ret _##name params { \
  VERBOSE_LOG("[import: " #name "]\n"); \
  body \
} \
ret (*WASM_RT_ADD_PREFIX(name)) params = _##name;

#define STUB_IMPORT_IMPL(ret, name, params, returncode) IMPORT_IMPL(ret, name, params, { return returncode; });

#define MAX_SETJMP_STACK 1024

static jmp_buf setjmp_stack[MAX_SETJMP_STACK];

static u32 next_setjmp = 0;

// Generic abort method for a runtime error in the runtime.

// static void abort_with_message(const char* message) {
//   fprintf(stderr, "%s\n", message);
//   TRAP(UNREACHABLE);
// }

// IMPORT_IMPL(void, Z_envZ__emscripten_throw_longjmpZ_vv, (), {
//   if (next_setjmp == 0) {
//     abort_with_message("longjmp without setjmp");
//   }
//   longjmp(setjmp_stack[next_setjmp - 1], 1);
// });

// IMPORT_IMPL(void, Z_envZ_emscripten_notify_memory_growthZ_vi, (u32 size), {});


// IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (u32 x), {
//   abort_with_message("exit() called");
// });

#define WASI_DEFAULT_ERROR 63 /* __WASI_ERRNO_PERM */
#define WASI_EINVAL 28

// STUB_IMPORT_IMPL(u32, Z_envZ___sys_openZ_iiii, (u32 path, u32 flags, u32 varargs), -1);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_writeZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), WASI_DEFAULT_ERROR);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_readZ_iiiii, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), WASI_DEFAULT_ERROR);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_closeZ_ii, (u32 fd), WASI_DEFAULT_ERROR);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_getZ_iii, (u32 pcount, u32 pbuf_size), WASI_DEFAULT_ERROR);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_getZ_iii, (u32 __environ, u32 environ_buf), WASI_DEFAULT_ERROR);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_getZ_iiji, (u32 clock_id, u64 max_lag, u32 out), WASI_EINVAL);
// STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_res_getZ_iii, (u32 clock_id, u32 out), WASI_EINVAL);

// #define MEMACCESS(addr) ((void*)&WASM_RT_ADD_PREFIX(Z_memory)->data[addr])
// #define MEMACCESS(inst, addr) wasm_rt_memory


// u32 w2c_wasi__snapshot__preview1_fd_seek(struct w2c_wasi__snapshot__preview*, u32 v0, u32 j1, var_i2, var_i3);
  // var_i0 = (*w2c_wasi__snapshot__preview1_fd_close)(instance->w2c_wasi__snapshot__preview1_instance, var_i0);
  // (*w2c_wasi__snapshot__preview1_proc_exit)(instance->w2c_wasi__snapshot__preview1_instance, var_i0);
  // var_i0 = (*w2c_wasi__snapshot__preview1_fd_write)(instance->w2c_wasi__snapshot__preview1_instance, var_i0, var_i1, var_i2, var_i3);
  // var_i0 = (*w2c_wasi__snapshot__preview1_fd_fdstat_get)(instance->w2c_wasi__snapshot__preview1_instance, var_i0, var_i1);
  // var_i0 = (*w2c_wasi__snapshot__preview1_fd_read)(instance->w2c_wasi__snapshot__preview1_instance, var_i0, var_i1, var_i2, var_i3);
  // var_i0 = (*w2c_wasi__snapshot__preview1_path_open)(instance->w2c_wasi__snapshot__preview1_instance, var_i0, var_i1, var_i2, var_i3, var_i4, var_j5, var_j6, var_i7, var_i8);
  // var_i0 = (*w2c_wasi__snapshot__preview1_fd_filestat_get)(instance->w2c_wasi__snapshot__preview1_instance, var_i0, var_i1);

/* import: 'wasi_snapshot_preview1' 'fd_fdstat_get' */
// u32 w2c_wasi__snapshot__preview1_fd_fdstat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_fdstat_set_flags' */
// u32 w2c_wasi__snapshot__preview1_fd_fdstat_set_flags(struct w2c_wasi__snapshot__preview1*, u32, u32);

// /* import: 'wasi_snapshot_preview1' 'fd_filestat_get' */
// u32 w2c_wasi__snapshot__preview1_fd_filestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'fd_pread' */
// u32 w2c_wasi__snapshot__preview1_fd_pread(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u64 j3, u32 i4) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'fd_prestat_dir_name' */
// u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'fd_prestat_get' */
// u32 w2c_wasi__snapshot__preview1_fd_prestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'fd_read' */
// u32 w2c_wasi__snapshot__preview1_fd_read(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'fd_seek' */
// // u32 w2c_wasi__snapshot__preview1_fd_seek(struct w2c_wasi__snapshot__preview1* p, u32 i0, u64 j1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'fd_write' */
// // u32 w2c_wasi__snapshot__preview1_fd_write(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

// /* import: 'wasi_snapshot_preview1' 'path_open' */
// u32 w2c_wasi__snapshot__preview1_path_open(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3, u32 i4, u64 j5, u64 j6, u32 i7, u32 i8) { return WASI_EINVAL; }

// /* import: 'wasi_snapshot_preview1' 'proc_exit' */
// void w2c_wasi__snapshot__preview1_proc_exit(struct w2c_wasi__snapshot__preview1* p, u32 i0) { exit(1); }

// // u32 w2c_wasi__snapshot__preview1_fd_pread(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u64 j3, u32 i4) { return WASI_DEFAULT_ERROR; }
// // u32 w2c_wasi__snapshot__preview1_fd_close(struct w2c_wasi__snapshot__preview1* p, u32 i0) { return WASI_DEFAULT_ERROR; }
// //u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2) {return WASI_DEFAULT_ERROR;}

// /* import: 'wasi_snapshot_preview1' 'fd_prestat_get' */
// // u32 w2c_wasi__snapshot__preview1_fd_prestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR;}
// /* import: 'wasi_snapshot_preview1' 'environ_get' */
// u32 w2c_wasi__snapshot__preview1_environ_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_EINVAL; }

// /* import: 'wasi_snapshot_preview1' 'environ_sizes_get' */
// u32 w2c_wasi__snapshot__preview1_environ_sizes_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return 0; }


// inline void* wasm_addr_to_absolute(w2c_libfreetype* inst, u32 wasm_addr) {
//   return w2c_libfreetype_memory(inst)->data + wasm_addr;
// }

// // void write_u32(u32 wasm_addr, u32 value) {
// //   *(u32*)wasm_addr_to_absolute(wasm_addr) = value;
// // }

// inline u32 read_u32(w2c_libfreetype* inst, u32 wasm_addr) {
//   return *(u32*)wasm_addr_to_absolute(inst, wasm_addr);
// }

// inline u8 read_u8(w2c_libfreetype* inst, u32 wasm_addr) {
//   return *(u8*)wasm_addr_to_absolute(inst, wasm_addr);
// }



// // void* wasm_addr_to_absolute(w2c_libfreetype* instance, u32 wasm_addr) {
// //   return &instance->w2c_memory.data[wasm_addr];
// // }

// void write_u32(w2c_libfreetype* instance, u32 wasm_addr, u32 value) {
//   *(u32*)wasm_addr_to_absolute(instance, wasm_addr) = value;
// }

// // u32 read_u32(w2c_libfreetype* instance, u32 wasm_addr) {
// //   return *(u32*)wasm_addr_to_absolute(instance, wasm_addr);
// // }

// int32_t read_i32(w2c_libfreetype* instance, u32 wasm_addr) {
//   return *(int32_t*)wasm_addr_to_absolute(instance, wasm_addr);
// }

// char* read_s32(w2c_libfreetype* instance, u32 wasm_addr) {
//     return (char*) wasm_addr_to_absolute(instance, wasm_addr);
// }

// // u8 read_u8(w2c_libfreetype* instance, u32 wasm_addr) {
// //   return *(u8*)wasm_addr_to_absolute(instance, wasm_addr);
// // }

// u16 read_u16(w2c_libfreetype* instance, u32 wasm_addr) {
//   return *(u16*)wasm_addr_to_absolute(instance, wasm_addr);
// }

// void outbuf(w2c_libfreetype* instance, int start, int sz) {
//   for (int i = 0; i < sz; i+=4) {
//     printf("%u ", read_u32(instance, start+i));
//   }
//   printf("\n");
// }

// void outbuf8(w2c_libfreetype* instance, int start, int sz) {
//   for (int i = 0; i < sz; i++) {
//     printf("%u ", read_u8(instance, start+i));
//   }
//   printf("\n");
// }

// // IMPORT_IMPL(u32,Z_envZ_setjmpZ_ii,(u32 env),{
// //   return setjmp(MEMACCESS(env));
// // });

// // IMPORT_IMPL(void, Z_envZ_longjmpZ_vii,(u32 env, u32 value),{
// //   longjmp(MEMACCESS(env), value);
// // });

// u32 w2c_env_setjmp(struct w2c_env* env, u32 jmp_buf_p) {
//   //jmp_buf* jbuf_ptr =  
//   // very hacky; uses internal __jmp_buf_tag
//   return _setjmp((__jmp_buf_tag*)wasm_addr_to_absolute(env->inst, jmp_buf_p));
//   //return setjmp(jmpbuf);
// }

// // w2c_env_longjmp(w2c_env* env, u32 jmp_buf_p)
// void w2c_env_longjmp(struct w2c_env* env, u32 jmp_buf_p, u32 status) {
//   return _longjmp((__jmp_buf_tag*)wasm_addr_to_absolute(env->inst, jmp_buf_p), status);
// }

// void wasmbox_runtime_init(void);

// Needed for stdlib when linking wasmbox against wasi-libc.
static u32 wasmbox__fd_close(u32 fd);

u32 w2c_wasi__snapshot__preview1_fd_close(
    struct w2c_wasi__snapshot__preview1* instance,
    u32 fd);

// The following functions implement part of the wasi-libc embedder interface
// enough to enable printing to the real stdout:
//
//   w2c_wasi__snapshot__preview1_fd_seek,
//   w2c_wasi__snapshot__preview1_fd_write,
//   w2c_wasi__snapshot__preview1_fd_fdstat_get.
//
static u32 wasmbox__fd_seek(u32 fd, u64 offset, u32 whence, u32 new_offset);

u32 w2c_wasi__snapshot__preview1_fd_seek(
    struct w2c_wasi__snapshot__preview1* instance,
    u32 fd,
    u64 offset,
    u32 whence,
    u32 new_offset);

// const u32 WASM_STDIN = 0;
// const u32 WASM_STDOUT = 1;
// const u32 WASM_STDERR = 2;

static u32 wasmbox__fd_write(wasm_rt_memory_t* memory,
                             u32 fd,
                             u32 iov,
                             u32 iovcnt,
                             u32 pnum);

u32 w2c_wasi__snapshot__preview1_fd_write(
    struct w2c_wasi__snapshot__preview1* instance,
    u32 fd,
    u32 iov,
    u32 iovcnt,
    u32 pnum);

static u32 wasmbox__fdstat_get(u32 fd, u32 x);

u32 w2c_wasi__snapshot__preview1_fd_fdstat_get(struct w2c_wasi__snapshot__preview1* instance, u32 fd, u32 x);

void w2c_wasi__snapshot__preview1_on_abort(struct w2c_wasi__snapshot__preview1* instance);



/* import: 'wasi_snapshot_preview1' 'fd_filestat_get' */
u32 w2c_wasi__snapshot__preview1_fd_filestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1);

/* import: 'wasi_snapshot_preview1' 'fd_pread' */
u32 w2c_wasi__snapshot__preview1_fd_pread(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u64 j3, u32 i4);

/* import: 'wasi_snapshot_preview1' 'fd_prestat_dir_name' */
u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2);

/* import: 'wasi_snapshot_preview1' 'fd_prestat_get' */
u32 w2c_wasi__snapshot__preview1_fd_prestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1);

/* import: 'wasi_snapshot_preview1' 'fd_read' */
u32 w2c_wasi__snapshot__preview1_fd_read(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3);

/* import: 'wasi_snapshot_preview1' 'fd_seek' */
// u32 w2c_wasi__snapshot__preview1_fd_seek(struct w2c_wasi__snapshot__preview1* p, u32 i0, u64 j1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_write' */
// u32 w2c_wasi__snapshot__preview1_fd_write(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'path_open' */
u32 w2c_wasi__snapshot__preview1_path_open(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3, u32 i4, u64 j5, u64 j6, u32 i7, u32 i8);

/* import: 'wasi_snapshot_preview1' 'proc_exit' */
void w2c_wasi__snapshot__preview1_proc_exit(struct w2c_wasi__snapshot__preview1* p, u32 i0);
// u32 w2c_wasi__snapshot__preview1_fd_pread(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u64 j3, u32 i4) { return WASI_DEFAULT_ERROR; }
// u32 w2c_wasi__snapshot__preview1_fd_close(struct w2c_wasi__snapshot__preview1* p, u32 i0) { return WASI_DEFAULT_ERROR; }
//u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2) {return WASI_DEFAULT_ERROR;}

/* import: 'wasi_snapshot_preview1' 'fd_prestat_get' */
// u32 w2c_wasi__snapshot__preview1_fd_prestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR;}
/* import: 'wasi_snapshot_preview1' 'environ_get' */
u32 w2c_wasi__snapshot__preview1_environ_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1);

/* import: 'wasi_snapshot_preview1' 'environ_sizes_get' */
u32 w2c_wasi__snapshot__preview1_environ_sizes_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1);

// -- freetype-rt.h partially

// inline void* wasm_addr_to_absolute(w2c_libfreetype* inst, u32 wasm_addr);
// // void write_u32(u32 wasm_addr, u32 value) {
// //   *(u32*)wasm_addr_to_absolute(wasm_addr) = value;
// // }

// inline u32 read_u32(w2c_libfreetype* inst, u32 wasm_addr);

// inline u8 read_u8(w2c_libfreetype* inst, u32 wasm_addr);



// // void* wasm_addr_to_absolute(w2c_libfreetype* instance, u32 wasm_addr) {
// //   return &instance->w2c_memory.data[wasm_addr];
// // }

// void write_u32(w2c_libfreetype* instance, u32 wasm_addr, u32 value);
// // u32 read_u32(w2c_libfreetype* instance, u32 wasm_addr) {
// //   return *(u32*)wasm_addr_to_absolute(instance, wasm_addr);
// // }

// int32_t read_i32(w2c_libfreetype* instance, u32 wasm_addr) ;

// char* read_s32(w2c_libfreetype* instance, u32 wasm_addr);

// // u8 read_u8(w2c_libfreetype* instance, u32 wasm_addr) {
// //   return *(u8*)wasm_addr_to_absolute(instance, wasm_addr);
// // }

// u16 read_u16(w2c_libfreetype* instance, u32 wasm_addr);
// void outbuf(w2c_libfreetype* instance, int start, int sz);

// void outbuf8(w2c_libfreetype* instance, int start, int sz);

inline void* wasm_addr_to_absolute(w2c_libfreetype* inst, u32 wasm_addr) {
  return w2c_libfreetype_memory(inst)->data + wasm_addr;
}

// void write_u32(u32 wasm_addr, u32 value) {
//   *(u32*)wasm_addr_to_absolute(wasm_addr) = value;
// }

inline u32 read_u32(w2c_libfreetype* inst, u32 wasm_addr) {
  return *(u32*)wasm_addr_to_absolute(inst, wasm_addr);
}

inline u8 read_u8(w2c_libfreetype* inst, u32 wasm_addr) {
  return *(u8*)wasm_addr_to_absolute(inst, wasm_addr);
}



// void* wasm_addr_to_absolute(w2c_libfreetype* instance, u32 wasm_addr) {
//   return &instance->w2c_memory.data[wasm_addr];
// }

inline void write_u32(w2c_libfreetype* instance, u32 wasm_addr, u32 value) {
  *(u32*)wasm_addr_to_absolute(instance, wasm_addr) = value;
}

// u32 read_u32(w2c_libfreetype* instance, u32 wasm_addr) {
//   return *(u32*)wasm_addr_to_absolute(instance, wasm_addr);
// }

inline int32_t read_i32(w2c_libfreetype* instance, u32 wasm_addr) {
  return *(int32_t*)wasm_addr_to_absolute(instance, wasm_addr);
}

inline char* read_s32(w2c_libfreetype* instance, u32 wasm_addr) {
    return (char*) wasm_addr_to_absolute(instance, wasm_addr);
}

// u8 read_u8(w2c_libfreetype* instance, u32 wasm_addr) {
//   return *(u8*)wasm_addr_to_absolute(instance, wasm_addr);
// }

inline u16 read_u16(w2c_libfreetype* instance, u32 wasm_addr) {
  return *(u16*)wasm_addr_to_absolute(instance, wasm_addr);
}

inline void outbuf(w2c_libfreetype* instance, int start, int sz) {
  for (int i = 0; i < sz; i+=4) {
    printf("%u ", read_u32(instance, start+i));
  }
  printf("\n");
}

inline void outbuf8(w2c_libfreetype* instance, int start, int sz) {
  for (int i = 0; i < sz; i++) {
    printf("%u ", read_u8(instance, start+i));
  }
  printf("\n");
}

// IMPORT_IMPL(u32,Z_envZ_setjmpZ_ii,(u32 env),{
//   return setjmp(MEMACCESS(env));
// });

// IMPORT_IMPL(void, Z_envZ_longjmpZ_vii,(u32 env, u32 value),{
//   longjmp(MEMACCESS(env), value);
// });

u32 w2c_env_setjmp(struct w2c_env* env, u32 jmp_buf_p);

// w2c_env_longjmp(w2c_env* env, u32 jmp_buf_p)
void w2c_env_longjmp(struct w2c_env* env, u32 jmp_buf_p, u32 status);

#endif