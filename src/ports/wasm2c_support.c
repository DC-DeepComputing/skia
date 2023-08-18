#include "src/ports/libfreetype.wasm.h"
#include "src/ports/freetype-rt.h"
#include <setjmp.h>
#include <errno.h>

#if defined(__GNUC__)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define UNLIKELY(x) (x)
#define LIKELY(x) (x)
#endif

#define TRAP(x) wasm_rt_trap(WASM_RT_TRAP_##x)

#define UNCOND_MEMCHECK_SIZE(mem, a, sz)  \
  if (UNLIKELY((a) + sz > mem->size)) TRAP(OOB)

#define UNCOND_MEMCHECK(mem, a, t)  UNCOND_MEMCHECK_SIZE(mem, a, sizeof(t))

#define MEMACCESS(mem, addr) ((void*)&(mem->data[addr]))

#define DEFINE_WASI_LOAD(name, t1, t2, t3)              \
  static inline t3 name(wasm_rt_memory_t* mem, u64 addr) {   \
    UNCOND_MEMCHECK(mem, addr, t1);                       \
    t1 result;                                     \
    memcpy(&result, MEMACCESS(mem, addr), sizeof(t1)); \
    return (t3)(t2)result;                         \
  }

#define DEFINE_WASI_STORE(name, t1, t2)                           \
  static inline void name(wasm_rt_memory_t* mem, u64 addr, t2 value) { \
    UNCOND_MEMCHECK(mem, addr, t1);                                 \
    t1 wrapped = (t1)value;                                  \
    memcpy(MEMACCESS(mem, addr), &wrapped, sizeof(t1));          \
  }

DEFINE_WASI_LOAD(wasm_i32_load, u32, u32, u32);
DEFINE_WASI_STORE(wasm_i32_store, u32, u32);

#define WASI_PERM_ERROR 63

void wasmbox_runtime_init(void) {}

// Needed for stdlib when linking wasmbox against wasi-libc.
static u32 wasmbox__fd_close(u32 fd) {
  fflush(stdout);
  return WASI_PERM_ERROR;
}

u32 w2c_wasi__snapshot__preview1_fd_close(
    struct w2c_wasi__snapshot__preview1* instance,
    u32 fd) {
  return wasmbox__fd_close(fd);
}

// The following functions implement part of the wasi-libc embedder interface
// enough to enable printing to the real stdout:
//
//   w2c_wasi__snapshot__preview1_fd_seek,
//   w2c_wasi__snapshot__preview1_fd_write,
//   w2c_wasi__snapshot__preview1_fd_fdstat_get.
//
static u32 wasmbox__fd_seek(u32 fd, u64 offset, u32 whence, u32 new_offset) {
  fflush(stdout);
  return WASI_PERM_ERROR;
}

u32 w2c_wasi__snapshot__preview1_fd_seek(
    struct w2c_wasi__snapshot__preview1* instance,
    u32 fd,
    u64 offset,
    u32 whence,
    u32 new_offset) {
  return wasmbox__fd_seek(fd, offset, whence, new_offset);
}

const u32 WASM_STDIN = 0;
const u32 WASM_STDOUT = 1;
const u32 WASM_STDERR = 2;

static u32 wasmbox__fd_write(wasm_rt_memory_t* memory,
                             u32 fd,
                             u32 iov,
                             u32 iovcnt,
                             u32 pnum) {
  fflush(stdout);

  u32 num = 0;
  for (u32 i = 0; i < iovcnt; ++i) {
    u32 ptr = wasm_i32_load((memory), iov + i * 8);
    u32 len = wasm_i32_load((memory), iov + i * 8 + 4);
    fflush(stderr);

    UNCOND_MEMCHECK_SIZE((memory), ptr, len);

    ssize_t result = 0;
    if (fd == WASM_STDOUT) {
      result = fwrite(MEMACCESS((memory), ptr), 1, len, stdout);
    } else if (fd == WASM_STDERR) {
      result = fwrite(MEMACCESS((memory), ptr), 1, len, stderr);
    } else {
      result = -1;
      errno = EBADF;
    }

    if (result < 0) {
      fprintf(stderr, "WASM:  error, %d %s\n", errno, strerror(errno));
      fflush(stderr);
      return WASI_PERM_ERROR;
    }

    if ((size_t)result != len) {
      fprintf(stderr, "WASM:  amount error, %ld %d\n", result, len);
      fflush(stderr);
      return WASI_PERM_ERROR;
    }
    num += len;
  }
  wasm_i32_store((memory), pnum, num);
  return 0;
}

u32 w2c_wasi__snapshot__preview1_fd_write(
    struct w2c_wasi__snapshot__preview1* instance,
    u32 fd,
    u32 iov,
    u32 iovcnt,
    u32 pnum) {
  // TODO(pasko): Prevent TOCTTOU here and in the rest of wasm2c_runtime.
  return wasmbox__fd_write(instance->memory_for_imports, fd, iov, iovcnt, pnum);
}

static u32 wasmbox__fdstat_get(u32 fd, u32 x) {
  fflush(stdout);
  return WASI_PERM_ERROR;
}

u32 w2c_wasi__snapshot__preview1_fd_fdstat_get(struct w2c_wasi__snapshot__preview1* instance, u32 fd, u32 x) {
  return wasmbox__fdstat_get(fd, x);
}

void w2c_wasi__snapshot__preview1_on_abort(struct w2c_wasi__snapshot__preview1* instance) {
}



/* import: 'wasi_snapshot_preview1' 'fd_filestat_get' */
u32 w2c_wasi__snapshot__preview1_fd_filestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_pread' */
u32 w2c_wasi__snapshot__preview1_fd_pread(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u64 j3, u32 i4) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_prestat_dir_name' */
u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_prestat_get' */
u32 w2c_wasi__snapshot__preview1_fd_prestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_read' */
u32 w2c_wasi__snapshot__preview1_fd_read(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_seek' */
// u32 w2c_wasi__snapshot__preview1_fd_seek(struct w2c_wasi__snapshot__preview1* p, u32 i0, u64 j1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'fd_write' */
// u32 w2c_wasi__snapshot__preview1_fd_write(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3) { return WASI_DEFAULT_ERROR; }

/* import: 'wasi_snapshot_preview1' 'path_open' */
u32 w2c_wasi__snapshot__preview1_path_open(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u32 i3, u32 i4, u64 j5, u64 j6, u32 i7, u32 i8) { return WASI_EINVAL; }

/* import: 'wasi_snapshot_preview1' 'proc_exit' */
void w2c_wasi__snapshot__preview1_proc_exit(struct w2c_wasi__snapshot__preview1* p, u32 i0) { exit(1); }

// u32 w2c_wasi__snapshot__preview1_fd_pread(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2, u64 j3, u32 i4) { return WASI_DEFAULT_ERROR; }
// u32 w2c_wasi__snapshot__preview1_fd_close(struct w2c_wasi__snapshot__preview1* p, u32 i0) { return WASI_DEFAULT_ERROR; }
//u32 w2c_wasi__snapshot__preview1_fd_prestat_dir_name(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1, u32 i2) {return WASI_DEFAULT_ERROR;}

/* import: 'wasi_snapshot_preview1' 'fd_prestat_get' */
// u32 w2c_wasi__snapshot__preview1_fd_prestat_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_DEFAULT_ERROR;}
/* import: 'wasi_snapshot_preview1' 'environ_get' */
u32 w2c_wasi__snapshot__preview1_environ_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return WASI_EINVAL; }

/* import: 'wasi_snapshot_preview1' 'environ_sizes_get' */
u32 w2c_wasi__snapshot__preview1_environ_sizes_get(struct w2c_wasi__snapshot__preview1* p, u32 i0, u32 i1) { return 0; }

// -- freetype-rt.h partially

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
//   printf("\n\n");
// }

// IMPORT_IMPL(u32,Z_envZ_setjmpZ_ii,(u32 env),{
//   return setjmp(MEMACCESS(env));
// });

// IMPORT_IMPL(void, Z_envZ_longjmpZ_vii,(u32 env, u32 value),{
//   longjmp(MEMACCESS(env), value);
// });

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