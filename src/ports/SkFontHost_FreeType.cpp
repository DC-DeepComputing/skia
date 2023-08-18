/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "base/debug/stack_trace.h"
#include "base/logging.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "src/sfnt/SkTTCFHeader.h"
#include "src/utils/SkCallableTraits.h"
#include "src/utils/SkMatrix22.h"

#include "base/numerics/safe_math.h"
#include "base/synchronization/lock.h"
#include "base/task/sequenced_task_runner.h"

#include <memory>
#include <optional>
#include <tuple>
#include <sstream>
#include <string>
#include <thread>

#include <ft2build.h>
#include <freetype/ftadvanc.h>
#include <freetype/ftimage.h>
#include <freetype/ftbitmap.h>
#ifdef FT_COLOR_H  // 2.10.0
#   include <freetype/ftcolor.h>
#endif
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftmm.h>
#include <freetype/ftoutln.h>
#include <freetype/ftsizes.h>
#include <freetype/ftsystem.h>
#include <freetype/tttables.h>
#include <freetype/t1tables.h>
#include <freetype/ftfntfmt.h>

#include <src/ports/libfreetype.wasm.h>
#include <src/ports/freetype-rt.h>

#define DEMO_ON 0
// #define ONOFF(code) code
#if DEMO_ON == 1
#define ONOFF(code) code
#else
#define ONOFF(code)
#endif 

#define ONON(code) code

using namespace skia_private;

namespace {
[[maybe_unused]] static inline const constexpr bool kSkShowTextBlitCoverage = false;

using SkUniqueFTFace = std::unique_ptr<FT_FaceRec, SkFunctionObject<FT_Done_Face>>;
using SkUniqueFTSize = std::unique_ptr<FT_SizeRec, SkFunctionObject<FT_Done_Size>>;
}

// SK_FREETYPE_MINIMUM_RUNTIME_VERSION 0x<major><minor><patch><flags>
// Flag SK_FREETYPE_DLOPEN: also try dlopen to get newer features.
#define SK_FREETYPE_DLOPEN (0x1)
#ifndef SK_FREETYPE_MINIMUM_RUNTIME_VERSION
#  if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) || defined (SK_BUILD_FOR_GOOGLE3)
#    define SK_FREETYPE_MINIMUM_RUNTIME_VERSION (((FREETYPE_MAJOR) << 24) | ((FREETYPE_MINOR) << 16) | ((FREETYPE_PATCH) << 8))
#  else
#    define SK_FREETYPE_MINIMUM_RUNTIME_VERSION ((2 << 24) | (8 << 16) | (1 << 8) | (SK_FREETYPE_DLOPEN))
#  endif
#endif
#if SK_FREETYPE_MINIMUM_RUNTIME_VERSION & SK_FREETYPE_DLOPEN
#  include <dlfcn.h>
#endif

#ifdef TT_SUPPORT_COLRV1
// FT_ClipBox and FT_Get_Color_Glyph_ClipBox introduced VER-2-11-0-18-g47cf8ebf4
// FT_COLR_COMPOSITE_PLUS and renumbering introduced VER-2-11-0-21-ge40ae7569
// FT_SIZEOF_LONG_LONG introduced VER-2-11-0-31-gffdac8d67
// FT_PaintRadialGradient changed size and layout at VER-2-11-0-147-gd3d3ff76d
// FT_STATIC_CAST introduced VER-2-11-0-172-g9079c5d91
// So undefine TT_SUPPORT_COLRV1 before 2.11.1 but not if FT_STATIC_CAST is defined.
#if (((FREETYPE_MAJOR)  < 2) || \
     ((FREETYPE_MAJOR) == 2 && (FREETYPE_MINOR)  < 11) || \
     ((FREETYPE_MAJOR) == 2 && (FREETYPE_MINOR) == 11 && (FREETYPE_PATCH) < 1)) && \
    !defined(FT_STATIC_CAST)
#    undef TT_SUPPORT_COLRV1
#endif
#endif

//#define ENABLE_GLYPH_SPEW     // for tracing calls
//#define DUMP_STRIKE_CREATION
//#define SK_FONTHOST_FREETYPE_RUNTIME_VERSION
//#define SK_GAMMA_APPLY_TO_A8

#if 1
    #define LOG_INFO(...)
#else
    #define LOG_INFO SkDEBUGF
#endif

static bool isLCD(const SkScalerContextRec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

static SkScalar SkFT_FixedToScalar(FT_Fixed x) {
  return SkFixedToScalar(x);
}

//////////////////////////////////////////////////////////////////////////

using FT_Alloc_size_t = SkCallableTraits<FT_Alloc_Func>::argument<1>::type;
static_assert(std::is_same<FT_Alloc_size_t, long  >::value ||
              std::is_same<FT_Alloc_size_t, size_t>::value,"");

extern "C" {
    static void* sk_ft_alloc(FT_Memory, FT_Alloc_size_t size) {
        return sk_malloc_canfail(size);
    }
    static void sk_ft_free(FT_Memory, void* block) {
        sk_free(block);
    }
    static void* sk_ft_realloc(FT_Memory, FT_Alloc_size_t cur_size,
                                          FT_Alloc_size_t new_size, void* block) {
        return sk_realloc_throw(block, new_size);
    }
}
FT_MemoryRec_ gFTMemory = { nullptr, sk_ft_alloc, sk_ft_free, sk_ft_realloc };

// void* wasm_addr_to_absolute(u32 wasm_addr) {
//   return &Z_memory->data[wasm_addr];
// }

// void write_u32(u32 wasm_addr, u32 value) {
//   *(u32*)wasm_addr_to_absolute(wasm_addr) = value;
// }

// u32 read_u32(u32 wasm_addr) {
//   return *(u32*)wasm_addr_to_absolute(wasm_addr);
// }


// int32_t read_i32(u32 wasm_addr) {
//   return *(int32_t*)wasm_addr_to_absolute(wasm_addr);
// }

// char* read_s32(u32 wasm_addr) {
//     return (char*) wasm_addr_to_absolute(wasm_addr);
// }

// u8 read_u8(u32 wasm_addr) {
//   return *(u8*)wasm_addr_to_absolute(wasm_addr);
// }

// u16 read_u16(u32 wasm_addr) {
//   return *(u16*)wasm_addr_to_absolute(wasm_addr);
// }

// void outbuf(int start, int sz) {
//   for (int i = 0; i < sz; i+=4) {
//     printf("%u ", read_u32(start+i));
//   }
//   printf("\n");
// }

// void outbuf8(int start, int sz) {
//   for (int i = 0; i < sz; i++) {
//     printf("%u ", read_u8(start+i));
//   }
//   printf("\n");
// }


int wasmbox_inited = 3;

struct FTObjectWB {
protected:
    w2c_libfreetype* wbinst_;
    const u32 pp_ = 0;                // double pointer to the object
    u32 p_ = 0;

    // Populate pointer to object; This function should be called only once.
    void populate_p() {
        p_ = read_u32(wbinst_, pp_);
        DCHECK(p_);
    }
public:
    FTObjectWB(w2c_libfreetype* wbinst):
        wbinst_(wbinst),
        pp_(w2c_libfreetype_malloc(wbinst, 4)) { // TODO(mjzhang): Figure out how to increase memory if this malloc fails.
        DCHECK(pp_);
    }

    u32 pp() { DCHECK(pp_); return pp_; }
    u32 p() { DCHECK(pp_); DCHECK(p_); return p_; }
    ~FTObjectWB() {
        if (pp_)
            w2c_libfreetype_free(wbinst_, pp_);
    }
};

struct FTLibraryWB : public FTObjectWB {
public:
    u32 create(u32 mem_p) {
        u32 res = w2c_libfreetype_FT_New_Library(wbinst_, mem_p, pp_); 
        if (res) {
            LOG(ERROR) << "ERROR: Failed to init FreeType (Sandboxed)\n";
        } else
            populate_p();
        return res;
    }

    ~FTLibraryWB() {
        if (p_)
            w2c_libfreetype_FT_Done_Library(wbinst_, p_);
    }
};

struct FTFaceWB : public FTObjectWB {
private:
    const u32 lib_p_;
public:
    FTFaceWB(w2c_libfreetype* wbinst, u32 lib_p) : 
        FTObjectWB(wbinst), lib_p_(lib_p) {
    }
    u32 create(u32 args_p, u32 face_index) {
        u32 res = w2c_libfreetype_FT_Open_Face(wbinst_, lib_p_, args_p, face_index, pp_);
        if (res) {
            LOG(ERROR) << "ERROR: Failed to create FT_Face";
        } else 
            populate_p();
        return res;
    }
    ~FTFaceWB() {
        if (p_)
            w2c_libfreetype_FT_Done_Face(wbinst_, p_);
    }
};

class FreeTypeLibrary : SkNoncopyable {
    // static bool wasmbox_inited;
public:
    FreeTypeLibrary() : fLibrary(nullptr) {
        if (wasmbox_inited == 3) {
            pid_t pid = getpid();
            // pid_t tid = gettid();
            std::stringstream ss; ss << std::this_thread::get_id();
            std::string tid = ss.str();
            printf("WASMBOX inited() in FreeTypeLibrary::ctor:\n");
            printf("  pid:%d, tid:%s, this(FreeTypeLibrary):%p, &wasmbox_inited:%p\n", pid, tid.c_str(), this, &wasmbox_inited);
            fflush(stdout);
            wasm_rt_init();
            //WASM_RT_ADD_PREFIX(init)();

            // 
            wasmbox_inited = 1;
        } else {
            printf("WASMBOX already inited() before FreeTypeLibrary::ctor\n");
            fflush(stdout);
        }
        if (FT_New_Library(&gFTMemory, &fLibrary)) {
            return;
        }
        FT_Add_Default_Modules(fLibrary);
        FT_Set_Default_Properties(fLibrary);

        // Subpixel anti-aliasing may be unfiltered until the LCD filter is set.
        // Newer versions may still need this, so this test with side effects must come first.
        // The default has changed over time, so this doesn't mean the same thing to all users.
        FT_Library_SetLcdFilter(fLibrary, FT_LCD_FILTER_DEFAULT);

        // Now FreeTypeLibrary creates TWO freetype library insances, one outside and one inside the sandbox.
        // First let's create a new sandbox instance
        env.inst = &wbinst;
        wasm2c_libfreetype_instantiate(&wbinst, &env, NULL);

        fLibraryWB = std::make_unique<FTLibraryWB>(&wbinst); 
  // Initialize freetype. 
  // u32 ft2_box = Z_mallocZ_ii(4);
        // u32 ft2_box = w2c_libfreetype_malloc(&wbinst, 4);

        // ft2_pp is a 4-byte location storing the address of another 4-byte location storing the address of an
        // actual object. So ft2_pp is effectively a "double pointer" or "pointer to pointer", hence the "_pp" suffix. 
        // printf("ft2_pp BEFORE = %d\n", ft2_pp);
        // ft2_pp = Z_mallocZ_ii(4);
        // ft2_pp = w2c_libfreetype_malloc(&wbinst, 4);

        // u32 mem_p = Z_FT_New_MemoryZ_iv(); // pointer to a memory address
        u32 mem_p = w2c_libfreetype_FT_New_Memory(&wbinst);
        
        // printf("ft2_pp = %u, mem_p = %u\n", ft2_pp, mem_p);
        // printf("ft2_p BEFORE= %u\n", read_u32(&wbinst, ft2_pp));

        // Create the new library instance.
        //if (Z_FT_New_LibraryZ_iii(mem_p, ft2_pp)) {
        //if (w2c_libfreetype_FT_New_Library(&wbinst, mem_p, ft2_pp)) {
        //    printf("ERROR: Failed to init FreeType (Sandboxed)\n");
        //    return;
        // }
        if (fLibraryWB->create(mem_p)) {
            printf("ERROR: Failed to init FreeType (Sandboxed)\n");
            return;
        }
        printf("WASMBOX: NewLib is created!\n");
        //u32 ft2_p = read_u32(&wbinst, ft2_pp);
        //printf("ft2_p AFTER= %u\n", ft2_p);
        printf("ft2_p AFTER= %u\n", fLibraryWB->p());

        // Z_FT_Add_Default_ModulesZ_vi(ft2_p);
        w2c_libfreetype_FT_Add_Default_Modules(&wbinst, fLibraryWB->p()); // ft2_p);
        MLOG(" BEFORE SETDEFAULT properties");
        // Z_FT_Set_Default_PropertiesZ_vi(ft2_p);
        w2c_libfreetype_FT_Set_Default_Properties(&wbinst, fLibraryWB->p()); //ft2_p);
        MLOG(" AFTER SETDEFAULT properties");
        // This caused crash? don't seem like it: Z_FT_Library_SetLcdFilterZ_iii(ft2_p, FT_LCD_FILTER_DEFAULT);
    }
    ~FreeTypeLibrary() {
        if (fLibrary) {
            FT_Done_Library(fLibrary);
        }

        fLibraryWB.reset();
        wasm2c_libfreetype_free(&wbinst); // wasmbox instance must be freed after fLibraryWB
    }

    FT_Library library() { return fLibrary; }
    // u32 lib2_p() { return read_u32(&wbinst, ft2_pp); }
    // u32 lib2_pp() { return ft2_pp; }
    w2c_libfreetype* get_wbinst() { return &wbinst; }
    FTLibraryWB* libraryWB() { return fLibraryWB.get(); }

private:
    FT_Library fLibrary;
    
    w2c_libfreetype wbinst;
    w2c_env env;
    
    std::unique_ptr<FTLibraryWB> fLibraryWB;
    
    // u32 ft2_pp;


    // FT_Library_SetLcdFilterWeights 2.4.0
    // FT_LOAD_COLOR 2.5.0
    // FT_Pixel_Mode::FT_PIXEL_MODE_BGRA 2.5.0
    // Thread safety in 2.6.0
    // freetype/ftfntfmt.h (rename) 2.6.0
    // Direct header inclusion 2.6.1
    // FT_Get_Var_Design_Coordinates 2.7.1
    // FT_LOAD_BITMAP_METRICS_ONLY 2.7.1
    // FT_Set_Default_Properties 2.7.2
    // The 'light' hinting is vertical only from 2.8.0
    // FT_Get_Var_Axis_Flags 2.8.1
    // FT_VAR_AXIS_FLAG_HIDDEN was introduced in FreeType 2.8.1
    // --------------------
    // FT_Done_MM_Var 2.9.0 (Currenty setting ft_free to a known allocator.)
    // freetype/ftcolor.h 2.10.0 (Currently assuming if compiled with FT_COLOR_H runtime available.)

    // Ubuntu 18.04       2.8.1
    // Debian 10          2.9.1
    // openSUSE Leap 15.2 2.10.1
    // Fedora 32          2.10.4
    // RHEL 8             2.9.1
};

//bool FreeTypeLibrary::wasmbox_inited = false;
static SkMutex& f_t_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

static FreeTypeLibrary* gFTLibrary;

///////////////////////////////////////////////////////////////////////////

class SkTypeface_FreeType::FaceRec {
public:
    SkUniqueFTFace fFace;
    u32 face2_pp;
    FT_StreamRec fFTStream;
    u32 ft_stream_p;
    std::unique_ptr<SkStreamAsset> fSkStream;
    FT_UShort fFTPaletteEntryCount = 0;
    std::unique_ptr<SkColor[]> fSkPalette;

    static std::unique_ptr<FaceRec> Make(const SkTypeface_FreeType* typeface);
    ~FaceRec();

private:
    FaceRec(std::unique_ptr<SkStreamAsset> stream);
    void setupAxes(const SkFontData& data);
    void setupPalette(const SkFontData& data);

    // Private to ref_ft_library and unref_ft_library
    static int gFTCount;

    // Caller must lock f_t_mutex() before calling this function.
    static bool ref_ft_library() {
        f_t_mutex().assertHeld();
        SkASSERT(gFTCount >= 0);

        if (0 == gFTCount) {
            SkASSERT(nullptr == gFTLibrary);
            gFTLibrary = new FreeTypeLibrary;
        }
        ++gFTCount;
        printf("In %s, gFTCount = %d\n", __PRETTY_FUNCTION__, gFTCount);
        return gFTLibrary->library();
    }

    // Caller must lock f_t_mutex() before calling this function.
    static void unref_ft_library() {
        f_t_mutex().assertHeld();
        SkASSERT(gFTCount > 0);

        --gFTCount;
        printf("In %s, gFTCount = %d\n", __PRETTY_FUNCTION__, gFTCount);
        if (0 == gFTCount) {
            SkASSERT(nullptr != gFTLibrary);
            delete gFTLibrary;
            SkDEBUGCODE(gFTLibrary = nullptr;)
        }
    }

    static FreeTypeLibrary* get_library() {
        DCHECK(gFTCount > 0);
        return gFTLibrary;
    }
};
int SkTypeface_FreeType::FaceRec::gFTCount;

extern "C" {
    static unsigned long sk_ft_stream_io(FT_Stream ftStream,
                                         unsigned long offset,
                                         unsigned char* buffer,
                                         unsigned long count)
    {
        SkStreamAsset* stream = static_cast<SkStreamAsset*>(ftStream->descriptor.pointer);

        if (count) {
            if (!stream->seek(offset)) {
                return 0;
            }
            count = stream->read(buffer, count);
        }
        return count;
    }

    static void sk_ft_stream_close(FT_Stream) {}
}

SkTypeface_FreeType::FaceRec::FaceRec(std::unique_ptr<SkStreamAsset> stream)
        : fSkStream(std::move(stream))
{
    sk_bzero(&fFTStream, sizeof(fFTStream));
    fFTStream.size = fSkStream->getLength();
    fFTStream.descriptor.pointer = fSkStream.get();
    fFTStream.read  = sk_ft_stream_io;
    fFTStream.close = sk_ft_stream_close;

    f_t_mutex().assertHeld();
    ref_ft_library();
}

SkTypeface_FreeType::FaceRec::~FaceRec() {
    f_t_mutex().assertHeld();
    fFace.reset(); // Must release face before the library, the library frees existing faces.
    unref_ft_library();
}

//   #define MEMP(struct_name, member_name)  \
//     __attribute__((used)) \
//     void* struct_name##_##member_name(struct_name *p) { return (void*)&p->member_name; }
  
//   MEMP(FT_FaceRec, num_faces)
//   MEMP(FT_FaceRec, face_index)
//   MEMP(FT_FaceRec, face_flags)
//   MEMP(FT_FaceRec, style_flags)
//   MEMP(FT_FaceRec, num_glyphs)
//   MEMP(FT_FaceRec, family_name)
//   MEMP(FT_FaceRec, style_name)
//   MEMP(FT_FaceRec, num_fixed_sizes)
//   MEMP(FT_FaceRec, available_sizes)

// #define MEMP(struct_name, member_name, p) Z_##struct_name##_##member_name##Z_ii(p)
// #define MEMP(struct_name, member_name, p) 

// #define MEMV(struct_name, member_name, p) read_u32(MEMP(struct_name, member_name, p))
#define MEMV(type, object, member, inst) w2c_libfreetype_##type##_##member##(inst, object)
#define SWITCH(code1, code2) (code1)

// struct WBHandle {
//     u32 pp;
//     WBHandle(std::nullptr_t = nullptr): pp(0) {}
//     WBHandle(int v): pp(v) {}
    
//     int alloc() { pp = Z_mallocZ_ii(4); return pp; }
//     operator int() {return pp;}

//     bool operator ==(const WBHandle& other) const { return val == other.val; }
//     bool operator !=(const WBHandle& other) const { return val == other.val; }
//     bool operator ==(std::nullptr_t) const { return val == 0; }
//     bool operator !=(std::nullptr_t) const { return val != 0; }
// };

ONOFF(
struct Wasm_FT_MM_Var {
private:
    u32 pp_ = 0;
    bool pp_inited = false;
    bool p_inited = false;
public:
    Wasm_FT_MM_Var() {
        pp_ = Z_mallocZ_ii(4);   // TODO(mjzhang): Figure out how to increase memory if this malloc fails.
        if (pp_ == 0) return;
        pp_inited = true;
    }
    int create(u32 face_p) {
        int res = Z_FT_Get_MM_VarZ_iii(face_p, pp_);
        if (!res) p_inited = true;
        return res;
    }
    u32 pp() { DCHECK(pp_); return pp_; }
    u32 p() { DCHECK(pp_); DCHECK(p_inited); return read_u32(pp_); }
    ~Wasm_FT_MM_Var() {
        if (pp_) {
            if (p_inited) {
                u32 p = read_u32(pp_);
                printf("Freeing p\n");
                Z_freeZ_vi(p);
            }
            printf("Freeing pp\n");
            Z_freeZ_vi(pp_);
        } else {
            printf("Free Nothing\n");
        }
        fflush(stdout);
    }
};
)

#define PrintField(struct_name, field_name, p, f1, f2, ref_ptr) \
    { u32 memp = MEMP(struct_name, field_name, p); \
       printf(#field_name " %d: %" #f1 " / %" #f2 "\n", memp, read_##f1##32(memp), ref_ptr->field_name); } \


#define PrintFaceRec() 
void SkTypeface_FreeType::FaceRec::setupAxes(const SkFontData& data) {
    // called once per ref_ft_library
    MLOG("IN FaceRec::SetupAxes");
        // printf("num Z_FT_FaceRec_num_facesZ_ii
    // printf("num_faces: %d\n", MEMP(FT_FaceRec, num_faces, read_u32(face2_pp)));
    ONOFF(
    // PrintField(FT_FaceRec, num_faces, read_u32(face2_pp),u,ld, fFace);
    // printf("face_index: %d\n", MEMP(FT_FaceRec, face_index, read_u32(face2_pp)));
    // printf("face_flags: %d, val=%d, ref_val=%ld\n", MEMP(FT_FaceRec, face_flags, read_u32(face2_pp)), read_u32(MEMP(FT_FaceRec, face_flags, read_u32(face2_pp))), fFace->face_flags);
    // printf("style_flags: %d\n", MEMP(FT_FaceRec, style_flags, read_u32(face2_pp)));
    // printf("num_glyphs: %d\n", MEMP(FT_FaceRec, num_glyphs, read_u32(face2_pp)));
    // printf("family_name: %d\n", MEMP(FT_FaceRec, family_name, read_u32(face2_pp)));
    // printf("style_name: %d\n", MEMP(FT_FaceRec, style_name, read_u32(face2_pp)));
    // printf("num_fixed_sizes: %d\n", MEMP(FT_FaceRec, num_fixed_sizes, read_u32(face2_pp)));
    // printf("available_sizes: %d\n", MEMP(FT_FaceRec, available_sizes, read_u32(face2_pp)));

    //PrintField(FT_FaceRec, num_faces, read_u32(face2_pp),u,ld, fFace);
    FreeTypeLibrary* lib = SkTypeface_FreeType::FaceRec::get_library();
    w2c_libfreetype* p_wbinst = lib->get_wbinst();

    u32 face2_p = read_u32(face2_pp);
    printf("num_faces: %d %d\n", fFace->num_faces, MEMV(FT_FaceRec, face2_p, num_faces, p_wbinst))
    // printf("face_index: %d\n", MEMP(FT_FaceRec, face_index, read_u32(face2_pp)));
    // printf("face_flags: %d, val=%d, ref_val=%ld\n", MEMP(FT_FaceRec, face_flags, read_u32(face2_pp)), read_u32(MEMP(FT_FaceRec, face_flags, read_u32(face2_pp))), fFace->face_flags);
    // printf("style_flags: %d\n", MEMP(FT_FaceRec, style_flags, read_u32(face2_pp)));
    // printf("num_glyphs: %d\n", MEMP(FT_FaceRec, num_glyphs, read_u32(face2_pp)));
    // printf("family_name: %d\n", MEMP(FT_FaceRec, family_name, read_u32(face2_pp)));
    // printf("style_name: %d\n", MEMP(FT_FaceRec, style_name, read_u32(face2_pp)));
    // printf("num_fixed_sizes: %d\n", MEMP(FT_FaceRec, num_fixed_sizes, read_u32(face2_pp)));
    // printf("available_sizes: %d\n", MEMP(FT_FaceRec, available_sizes, read_u32(face2_pp)));
    fflush(stdout);

    u32 face_flags2 = MEMV(FT_FaceRec, face_flags, read_u32(face2_pp));
    DCHECK_EQ(fFace->face_flags, face_flags2);
    printf("face_flag equal: %d, val=%d, ref_val=%ld\n", MEMP(FT_FaceRec, face_flags, read_u32(face2_pp)), read_u32(MEMP(FT_FaceRec, face_flags, read_u32(face2_pp))), fFace->face_flags);

    if (!(SWITCH(fFace->face_flags, face_flags2) & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return;
    }
    )

    // If a named variation is requested, don't overwrite the named variation's position.
    if (data.getIndex() > 0xFFFF) {
        return;
    }



    // At least when opening a wikipedia page, the control flow never reaches here.
    MLOG(" IN SetupAxes checkpoint 2");

    SkDEBUGCODE(
        FT_MM_Var* variations = nullptr;

        int var_res1 = FT_Get_MM_Var(fFace.get(), &variations);

        ONOFF(
        Wasm_FT_MM_Var var2;
        int var_res2 = var2.create(read_u32(face2_pp));
        
        DCHECK_EQ(var_res1, var_res2);
        )

        if (var_res1) {
            LOG_INFO("INFO: font %s claims variations, but none found.\n",
                     rec->fFace->family_name);
            return;
        }
        UniqueVoidPtr autoFreeVariations(variations);

        if (static_cast<FT_UInt>(data.getAxisCount()) != variations->num_axis) {
            LOG_INFO("INFO: font %s has %d variations, but %d were specified.\n",
                     rec->fFace->family_name, variations->num_axis, data.getAxisCount());
            return;
        }
    )

    MLOG(" IN SetupAxes checkpoint 3");

    AutoSTMalloc<4, FT_Fixed> coords(data.getAxisCount());
    for (int i = 0; i < data.getAxisCount(); ++i) {
        coords[i] = data.getAxis()[i];
    }
    if (FT_Set_Var_Design_Coordinates(fFace.get(), data.getAxisCount(), coords.get())) {
        LOG_INFO("INFO: font %s has variations, but specified variations could not be set.\n",
                 rec->fFace->family_name);
        return;
    }

    MLOG(" IN SetupAxes checkpoint 4");
    ONOFF(

    u32 coords_p = Z_mallocZ_ii(data.getAxisCount()*4);
        memcpy(wasm_addr_to_absolute(coords_p), coords.get(), data.getAxisCount()*4);

    if (Z_FT_Set_Var_Design_CoordinatesZ_iiii(read_u32(face2_pp), data.getAxisCount(), coords_p)) {
        printf("Z_FT_Set_Var_Design_CoordinatesZ_iiii FAILED!\n");
        return;
    }

    MLOG(" IN SetupAxes checkpoint 5");


    for (int i = 0; i < data.getAxisCount(); ++i) {
        printf("  coords %d: raw: %ld, sandbox: %d\n", i, coords[i], read_u32(coords_p+4*i));
    }
    )
}

void SkTypeface_FreeType::FaceRec::setupPalette(const SkFontData& data) {
#ifdef FT_COLOR_H
    MLOG("IN FaceRec::setupPalette");

    FT_Palette_Data paletteData;
    if (FT_Palette_Data_Get(fFace.get(), &paletteData)) {
        return;
    }

    MLOG(" IN FaceRec::setupPalette check point 2");

    // Treat out of range values as 0. Still apply overrides.
    // https://www.w3.org/TR/css-fonts-4/#base-palette-desc
    FT_UShort basePaletteIndex = 0;
    if (SkTFitsIn<FT_UShort>(data.getPaletteIndex()) &&
        SkTo<FT_UShort>(data.getPaletteIndex()) < paletteData.num_palettes)
    {
        basePaletteIndex = data.getPaletteIndex();
    }

    FT_Color* ftPalette = nullptr;

    // TODO(wasmdemo): Here we don't call the sandboxed FT_Palette_Select version, since
    // FT_Palette_Select does not change the state of fFace.
    if (FT_Palette_Select(fFace.get(), basePaletteIndex, &ftPalette)) {
        return;
    }
    fFTPaletteEntryCount = paletteData.num_palette_entries;

    for (int i = 0; i < data.getPaletteOverrideCount(); ++i) {
        const SkFontArguments::Palette::Override& paletteOverride = data.getPaletteOverrides()[i];
        if (0 <= paletteOverride.index && paletteOverride.index < fFTPaletteEntryCount) {
            const SkColor& skColor = paletteOverride.color;
            FT_Color& ftColor = ftPalette[paletteOverride.index];
            ftColor.blue  = SkColorGetB(skColor);
            ftColor.green = SkColorGetG(skColor);
            ftColor.red   = SkColorGetR(skColor);
            ftColor.alpha = SkColorGetA(skColor);
        }
    }

    fSkPalette.reset(new SkColor[fFTPaletteEntryCount]);
    for (int i = 0; i < fFTPaletteEntryCount; ++i) {
        fSkPalette[i] = SkColorSetARGB(ftPalette[i].alpha,
                                       ftPalette[i].red,
                                       ftPalette[i].green,
                                       ftPalette[i].blue);
    }
#endif
}

// Will return nullptr on failure
// Caller must lock f_t_mutex() before calling this function.
std::unique_ptr<SkTypeface_FreeType::FaceRec>
SkTypeface_FreeType::FaceRec::Make(const SkTypeface_FreeType* typeface) {
    f_t_mutex().assertHeld();

    MLOG("In FaceRec::Make");
    std::unique_ptr<SkFontData> data = typeface->makeFontData();
    if (nullptr == data || !data->hasStream()) {
        return nullptr;
    }

    std::unique_ptr<FaceRec> rec(new FaceRec(data->detachStream()));

    FT_Open_Args args;
    memset(&args, 0, sizeof(args));
    const void* memoryBase = rec->fSkStream->getMemoryBase();

ONON(
    FreeTypeLibrary* lib = SkTypeface_FreeType::FaceRec::get_library();
    w2c_libfreetype* p_wbinst = lib->get_wbinst();

    u32 mem2_p = w2c_libfreetype_malloc(p_wbinst, rec->fSkStream->getLength());

    memcpy(wasm_addr_to_absolute(p_wbinst, mem2_p), memoryBase, rec->fSkStream->getLength());

    int sizeof_args = w2c_libfreetype_FT_sizeof_FT_Open_Args(p_wbinst);
    u32 args2_p = w2c_libfreetype_malloc(p_wbinst, sizeof_args); // args actually only needs 32 bytes (8 uint32 members). Just to be safe here.    
    printf("  args2_p, sizeof = %d, before memset to 0:", sizeof_args);
    // args2_p is likely pointing to a buffer that is already all zero.
    outbuf8(p_wbinst, args2_p, sizeof_args);

    memset(wasm_addr_to_absolute(p_wbinst, args2_p), 0, sizeof_args);  // Set all data to 0
    // memset(wasm_addr_to_absolute(p_wbinst, args2_p+32), 0x7f, 31);  // Set high 31 bytes to 7f (for tests only)

    printf("  after memset to 0: ");
    outbuf8(p_wbinst, args2_p, sizeof_args);
)

    if (memoryBase) {
        printf("FaceRec::Make:: MEMORYBASE\n");
        args.flags = FT_OPEN_MEMORY;
        args.memory_base = (const FT_Byte*)memoryBase;
        args.memory_size = rec->fSkStream->getLength();
ONON(
        // write_u32(args2_p, FT_OPEN_MEMORY);
        // write_u32(args2_p+4, mem2_p);
        // write_u32(args2_p+8, rec->fSkStream->getLength());
        w2c_libfreetype_FT_Open_Args_flags_set(p_wbinst, args2_p, FT_OPEN_MEMORY);
        w2c_libfreetype_FT_Open_Args_memory_base_set(p_wbinst, args2_p, mem2_p);
        w2c_libfreetype_FT_Open_Args_memory_size_set(p_wbinst, args2_p, rec->fSkStream->getLength());
)
    } else {
        printf("FaceRec::Make:: MEMORYSTREAM\n");
        args.flags = FT_OPEN_STREAM;
        args.stream = &rec->fFTStream;
ONON(
       //  write_u32(args2_p, FT_OPEN_STREAM);
        // This won't work for now, since ft_stream_p is not set.
       //  write_u32(args2_p+4*4, rec->ft_stream_p);
        w2c_libfreetype_FT_Open_Args_flags_set(p_wbinst, args2_p, FT_OPEN_STREAM);

        // TODO(mjzhang) Properly assign the ft_stream_p into Sandbox
        DCHECK(0);
        w2c_libfreetype_FT_Open_Args_stream_set(p_wbinst, args2_p, 0);
)
    }

    printf(" args.flags: %d\n", args.flags);
    printf(" args.memory_base: %p\n", args.memory_base);  
    printf(" args.memory_size: %ld\n", args.memory_size);    
    printf(" args.pathname: %s\n", args.pathname);    
    printf(" args.stream: %p\n", args.stream);    
    printf(" args.driver: %p\n", args.driver);    
    printf(" args.num_params: %d\n", args.num_params);    
    printf(" args.params: %p\n", args.params);    
    printf("The sandbox's arg is: \n");
ONON(
    outbuf(p_wbinst, args2_p, 32);
)
    {
        FT_Face rawFace;
        printf("OpenFace In %s\n", __PRETTY_FUNCTION__);
        FT_Error err = FT_Open_Face(gFTLibrary->library(), &args, data->getIndex(), &rawFace);
        if (err) {
            SK_TRACEFTR(err, "unable to open font '%x'", typeface->uniqueID());
            return nullptr;
        }
        rec->fFace.reset(rawFace);
ONON(
        // u32 face2_pp = Z_mallocZ_ii(4);

        // if (Z_FT_Open_FaceZ_iiiii(gFTLibrary->lib2_p(), args2_p, data->getIndex(), face2_pp)) {
        //     printf("lib2: %d %d\n", gFTLibrary->lib2_p(), gFTLibrary->lib2_pp());
        //     printf("MAKE: OpenFace in sandbox failed\n");
        //     return nullptr;
        // }
        // rec->face2_pp = face2_pp;
        u32 face2_pp = w2c_libfreetype_malloc(p_wbinst, 4);

        if (w2c_libfreetype_FT_Open_Face(p_wbinst, lib->libraryWB()->p(), args2_p, data->getIndex(), face2_pp)) {
            printf("lib2: %d %d\n", lib->libraryWB()->p(), lib->libraryWB()->pp());
            printf("MAKE: OpenFace in sandbox failed\n");
            return nullptr;
        } else {
            printf("FaceRec::Make(): OpenFace in sandbox succeeded!\n");
        }
        rec->face2_pp = face2_pp;

)
    }
    SkASSERT(rec->fFace);

    rec->setupAxes(*data);
    rec->setupPalette(*data);

    // FreeType will set the charmap to the "most unicode" cmap if it exists.
    // If there are no unicode cmaps, the charmap is set to nullptr.
    // However, "symbol" cmaps should also be considered "fallback unicode" cmaps
    // because they are effectively private use area only (even if they aren't).
    // This is the last on the fallback list at
    // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
    if (!rec->fFace->charmap) {
        printf(" FT_Select_Charmap\n");
        fflush(stdout);
        FT_Select_Charmap(rec->fFace.get(), FT_ENCODING_MS_SYMBOL);
        // TODO:    FT_Select_Charmap(rec->fFace.get(), FT_ENCODING_MS_SYMBOL);
    }

    return rec;
}

class AutoFTAccess {
public:
    AutoFTAccess(const SkTypeface_FreeType* tf) : fFaceRec(nullptr) {
        f_t_mutex().acquire();
        fFaceRec = tf->getFaceRec();
    }

    ~AutoFTAccess() {
        f_t_mutex().release();
    }

    FT_Face face() { return fFaceRec ? fFaceRec->fFace.get() : nullptr; }
ONOFF(
    u32 face2_pp() { return fFaceRec ? fFaceRec->face2_pp : 0; }
    u32 face2_p() { u32 pp = face2_pp(); if (pp) return read_u32(pp); else return 0;}
)

private:
    SkTypeface_FreeType::FaceRec* fFaceRec;
};

///////////////////////////////////////////////////////////////////////////

class SkScalerContext_FreeType : public SkScalerContext_FreeType_Base {
public:
    SkScalerContext_FreeType(sk_sp<SkTypeface_FreeType>,
                             const SkScalerContextEffects&,
                             const SkDescriptor* desc);
    ~SkScalerContext_FreeType() override;

    bool success() const {
        return fFTSize != nullptr && fFace != nullptr;
    }

protected:
    bool generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph, SkArenaAlloc*) override;
    void generateImage(const SkGlyph& glyph) override;
    bool generatePath(const SkGlyph& glyph, SkPath* path) override;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph&) override;
    void generateFontMetrics(SkFontMetrics*) override;

private:
    SkTypeface_FreeType::FaceRec* fFaceRec; // Borrowed face from the typeface's FaceRec.
    FT_Face   fFace;  // Borrowed face from fFaceRec.
    u32 face2_pp;
    FT_Size   fFTSize;  // The size to apply to the fFace.
    u32 fsize2_pp;
    FT_Int    fStrikeIndex; // The bitmap strike for the fFace (or -1 if none).

    /** The rest of the matrix after FreeType handles the size.
     *  With outline font rasterization this is handled by FreeType with FT_Set_Transform.
     *  With bitmap only fonts this matrix must be applied to scale the bitmap.
     */
    SkMatrix  fMatrix22Scalar;
    /** Same as fMatrix22Scalar, but in FreeType units and space. */
    FT_Matrix fMatrix22;
    /** The actual size requested. */
    SkVector  fScale;

    uint32_t  fLoadGlyphFlags;
    bool      fDoLinearMetrics;
    bool      fLCDIsVert;

    FT_Error setupSize();
    static bool getBoundsOfCurrentOutlineGlyph(FT_GlyphSlot glyph, SkRect* bounds);
    static void setGlyphBounds(SkGlyph* glyph, SkRect* bounds, bool subpixel);
    bool getCBoxForLetter(char letter, FT_BBox* bbox);
    // Caller must lock f_t_mutex() before calling this function.
    void updateGlyphBoundsIfLCD(SkGlyph* glyph);
    // Caller must lock f_t_mutex() before calling this function.
    // update FreeType2 glyph slot with glyph emboldened
    void emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph, SkGlyphID gid, u32 face2_p, u32 glyph2_p, SkGlyphID gid2);
    bool shouldSubpixelBitmap(const SkGlyph&, const SkMatrix&);
};

///////////////////////////////////////////////////////////////////////////

static bool canEmbed(FT_Face face) {
    MLOG("IN canEmbed");
    // Not used in demo
    FT_UShort fsType = FT_Get_FSType_Flags(face);  // does not change state of face
    return (fsType & (FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING |
                      FT_FSTYPE_BITMAP_EMBEDDING_ONLY)) == 0;
}

static bool canSubset(FT_Face face) {
    MLOG("In canSubset");
    // Not used in demo
    FT_UShort fsType = FT_Get_FSType_Flags(face); // does not change state of face
    return (fsType & FT_FSTYPE_NO_SUBSETTING) == 0;
}

static SkAdvancedTypefaceMetrics::FontType get_font_type(FT_Face face) {
    const char* fontType = FT_Get_X11_Font_Format(face);
    static struct { const char* s; SkAdvancedTypefaceMetrics::FontType t; } values[] = {
        { "Type 1",     SkAdvancedTypefaceMetrics::kType1_Font    },
        { "CID Type 1", SkAdvancedTypefaceMetrics::kType1CID_Font },
        { "CFF",        SkAdvancedTypefaceMetrics::kCFF_Font      },
        { "TrueType",   SkAdvancedTypefaceMetrics::kTrueType_Font },
    };
    for(const auto& v : values) { if (strcmp(fontType, v.s) == 0) { return v.t; } }
    return SkAdvancedTypefaceMetrics::kOther_Font;
}

static bool is_opentype_font_data_standard_format(const SkTypeface& typeface) {
    // FreeType reports TrueType for any data that can be decoded to TrueType or OpenType.
    // However, there are alternate data formats for OpenType, like wOFF and wOF2.
    std::unique_ptr<SkStreamAsset> stream = typeface.openStream(nullptr);
    if (!stream) {
        return false;
    }
    char buffer[4];
    if (stream->read(buffer, 4) < 4) {
        return false;
    }

    SkFourByteTag tag = SkSetFourByteTag(buffer[0], buffer[1], buffer[2], buffer[3]);
    SK_OT_ULONG otTag = SkEndian_SwapBE32(tag);
    return otTag == SkSFNTHeader::fontType_WindowsTrueType::TAG ||
           otTag == SkSFNTHeader::fontType_MacTrueType::TAG ||
           otTag == SkSFNTHeader::fontType_PostScript::TAG ||
           otTag == SkSFNTHeader::fontType_OpenTypeCFF::TAG ||
           otTag == SkTTCFHeader::TAG;
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_FreeType::onGetAdvancedMetrics() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
        MLOG("In SkTypeface_FreeType::onGetAdvancedMetrics");

    if (!face) {
        return nullptr;
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);
    info->fPostScriptName.set(FT_Get_Postscript_Name(face));
    info->fFontName = info->fPostScriptName;

    if (FT_HAS_MULTIPLE_MASTERS(face)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kVariable_FontFlag;
    }
    if (!canEmbed(face)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag;
    }
    if (!canSubset(face)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag;
    }

    info->fType = get_font_type(face);
    if (info->fType == SkAdvancedTypefaceMetrics::kTrueType_Font &&
        !is_opentype_font_data_standard_format(*this))
    {
        info->fFlags |= SkAdvancedTypefaceMetrics::kAltDataFormat_FontFlag;
    }

    info->fStyle = (SkAdvancedTypefaceMetrics::StyleFlags)0;
    if (FT_IS_FIXED_WIDTH(face)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    if (face->style_flags & FT_STYLE_FLAG_ITALIC) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }

    PS_FontInfoRec psFontInfo;
    TT_Postscript* postTable;
    if (FT_Get_PS_Font_Info(face, &psFontInfo) == 0) {
        info->fItalicAngle = psFontInfo.italic_angle;
    } else if ((postTable = (TT_Postscript*)FT_Get_Sfnt_Table(face, ft_sfnt_post)) != nullptr) {
        info->fItalicAngle = SkFixedFloorToInt(postTable->italicAngle);
    } else {
        info->fItalicAngle = 0;
    }

    info->fAscent = face->ascender;
    info->fDescent = face->descender;

    TT_PCLT* pcltTable;
    TT_OS2* os2Table;
    if ((pcltTable = (TT_PCLT*)FT_Get_Sfnt_Table(face, ft_sfnt_pclt)) != nullptr) {
        info->fCapHeight = pcltTable->CapHeight;
        uint8_t serif_style = pcltTable->SerifStyle & 0x3F;
        if (2 <= serif_style && serif_style <= 6) {
            info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
        } else if (9 <= serif_style && serif_style <= 12) {
            info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
        }
    } else if (((os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2)) != nullptr) &&
               // sCapHeight is available only when version 2 or later.
               os2Table->version != 0xFFFF &&
               os2Table->version >= 2)
    {
        info->fCapHeight = os2Table->sCapHeight;
    }
    info->fBBox = SkIRect::MakeLTRB(face->bbox.xMin, face->bbox.yMax,
                                    face->bbox.xMax, face->bbox.yMin);
    return info;
}

void SkTypeface_FreeType::getGlyphToUnicodeMap(SkUnichar* dstArray) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    MLOG("In SkTypeface_FreeType::getGlyphToUnicodeMap");

    if (!face) {
        return;
    }

    FT_Long numGlyphs = face->num_glyphs;
    if (!dstArray) { SkASSERT(numGlyphs == 0); }
    sk_bzero(dstArray, sizeof(SkUnichar) * numGlyphs);

    FT_UInt glyphIndex;
        MLOG(" - Flag: FT_Get_First_Char");

    SkUnichar charCode = FT_Get_First_Char(face, &glyphIndex);
    while (glyphIndex) {
        SkASSERT(glyphIndex < SkToUInt(numGlyphs));
        // Use the first character that maps to this glyphID. https://crbug.com/359065
        if (0 == dstArray[glyphIndex]) {
            dstArray[glyphIndex] = charCode;
        }
        charCode = FT_Get_Next_Char(face, charCode, &glyphIndex);
    }
}

void SkTypeface_FreeType::getPostScriptGlyphNames(SkString* dstArray) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    MLOG("In getPostScriptGlyphNames");

    if (!face) {
        return;
    }

    FT_Long numGlyphs = face->num_glyphs;
    if (!dstArray) { SkASSERT(numGlyphs == 0); }

// Does not modify face
    if (FT_HAS_GLYPH_NAMES(face)) {
        for (int gID = 0; gID < numGlyphs; ++gID) {
            char glyphName[128];  // PS limit for names is 127 bytes.
            FT_Get_Glyph_Name(face, gID, glyphName, 128);
            dstArray[gID] = glyphName;
        }
    }
}

bool SkTypeface_FreeType::onGetPostScriptName(SkString* skPostScriptName) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    MLOG("In onGetPostScriptName");

    if (!face) {
        return false;
    }

    const char* ftPostScriptName = FT_Get_Postscript_Name(face);
    if (!ftPostScriptName) {
        return false;
    }
    if (skPostScriptName) {
        *skPostScriptName = ftPostScriptName;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////

static bool bothZero(SkScalar a, SkScalar b) {
    return 0 == a && 0 == b;
}

// returns false if there is any non-90-rotation or skew
static bool isAxisAligned(const SkScalerContextRec& rec) {
    return 0 == rec.fPreSkewX &&
           (bothZero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
            bothZero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

std::unique_ptr<SkScalerContext> SkTypeface_FreeType::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    auto c = std::make_unique<SkScalerContext_FreeType>(
            sk_ref_sp(const_cast<SkTypeface_FreeType*>(this)), effects, desc);
    if (c->success()) {
        return std::move(c);
    }
    return SkScalerContext::MakeEmpty(
            sk_ref_sp(const_cast<SkTypeface_FreeType*>(this)), effects, desc);
}

/** Copy the design variation coordinates into 'coordinates'.
 *
 *  @param coordinates the buffer into which to write the design variation coordinates.
 *  @param coordinateCount the number of entries available through 'coordinates'.
 *
 *  @return The number of axes, or -1 if there is an error.
 *  If 'coordinates != nullptr' and 'coordinateCount >= numAxes' then 'coordinates' will be
 *  filled with the variation coordinates describing the position of this typeface in design
 *  variation space. It is possible the number of axes can be retrieved but actual position
 *  cannot.
 */
static int GetVariationDesignPosition(AutoFTAccess& fta,
    SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount)
{
    FT_Face face = fta.face();
    
    MLOG("In GetVariationDesignPosition");

    if (!face) {
        return -1;
    }

    if (!(face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return 0;
    }

    FT_MM_Var* variations = nullptr;
    if (FT_Get_MM_Var(face, &variations)) {
        return -1;
    }

ONOFF(
    u32 face2_p = fta.face2_p();
    u32 variation2_pp = Z_mallocZ_ii(4);
    if (Z_FT_Get_MM_VarZ_iii(face2_p, variation2_pp)) {
        MLOG("!!! Raw FT_Get_MM_Var succeeded but the sandboxed version failed!!!");
        return -1;
    }
)
    UniqueVoidPtr autoFreeVariations(variations);

    if (!coordinates || coordinateCount < SkToInt(variations->num_axis)) {
        return variations->num_axis;
    }

    AutoSTMalloc<4, FT_Fixed> coords(variations->num_axis);
    if (FT_Get_Var_Design_Coordinates(face, variations->num_axis, coords.get())) {
        return -1;
    }

    // TODO
    for (FT_UInt i = 0; i < variations->num_axis; ++i) {
        coordinates[i].axis = variations->axis[i].tag;
        coordinates[i].value = SkFixedToScalar(coords[i]);
    }

    return variations->num_axis;
}

std::unique_ptr<SkFontData> SkTypeface_FreeType::cloneFontData(const SkFontArguments& args) const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return nullptr;
    }

    Scanner::AxisDefinitions axisDefinitions;
    if (!Scanner::GetAxes(face, &axisDefinitions)) {
        return nullptr;
    }
    int axisCount = axisDefinitions.size();

    AutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> currentPosition(axisCount);
    int currentAxisCount = GetVariationDesignPosition(fta, currentPosition, axisCount);

    SkString name;
    AutoSTMalloc<4, SkFixed> axisValues(axisCount);
    Scanner::computeAxisValues(axisDefinitions, args.getVariationDesignPosition(), axisValues, name,
                               currentAxisCount == axisCount ? currentPosition.get() : nullptr);

    int ttcIndex;
    std::unique_ptr<SkStreamAsset> stream = this->openStream(&ttcIndex);

    return std::make_unique<SkFontData>(std::move(stream),
                                        ttcIndex,
                                        args.getPalette().index,
                                        axisValues.get(),
                                        axisCount,
                                        args.getPalette().overrides,
                                        args.getPalette().overrideCount);
}

void SkTypeface_FreeType::onFilterRec(SkScalerContextRec* rec) const {
    //BOGUS: http://code.google.com/p/chromium/issues/detail?id=121119
    //Cap the requested size as larger sizes give bogus values.
    //Remove when http://code.google.com/p/skia/issues/detail?id=554 is fixed.
    //Note that this also currently only protects against large text size requests,
    //the total matrix is not taken into account here.
    if (rec->fTextSize > SkIntToScalar(1 << 14)) {
        rec->fTextSize = SkIntToScalar(1 << 14);
    }

    SkFontHinting h = rec->getHinting();
    if (SkFontHinting::kFull == h && !isLCD(*rec)) {
        // collapse full->normal hinting if we're not doing LCD
        h = SkFontHinting::kNormal;
    }

    // rotated text looks bad with hinting, so we disable it as needed
    if (!isAxisAligned(*rec)) {
        h = SkFontHinting::kNone;
    }
    rec->setHinting(h);

#ifndef SK_GAMMA_APPLY_TO_A8
    if (!isLCD(*rec)) {
        // SRGBTODO: Is this correct? Do we want contrast boost?
        rec->ignorePreBlend();
    }
#endif
}

int SkTypeface_FreeType::GetUnitsPerEm(FT_Face face) {
    SkASSERT(face);
    MLOG("In SkTypeface_FreeType::GetUnitsPerEm");

    SkScalar upem = SkIntToScalar(face->units_per_EM);
    // At least some versions of FreeType set face->units_per_EM to 0 for bitmap only fonts.
    if (upem == 0) {
        TT_Header* ttHeader = (TT_Header*)FT_Get_Sfnt_Table(face, ft_sfnt_head);
        if (ttHeader) {
            upem = SkIntToScalar(ttHeader->Units_Per_EM);
        }
    }
    return upem;
}

int SkTypeface_FreeType::onGetUPEM() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return 0;
    }
    return GetUnitsPerEm(face);
}

bool SkTypeface_FreeType::onGetKerningPairAdjustments(const uint16_t glyphs[],
                                      int count, int32_t adjustments[]) const {
    AutoFTAccess fta(this);
    MLOG("In SkTypeface_FreeType::onGetKerningPairAdjustments");

    FT_Face face = fta.face();
    if (!face || !FT_HAS_KERNING(face)) {
        return false;
    }

    for (int i = 0; i < count - 1; ++i) {
        FT_Vector delta;
        FT_Error err = FT_Get_Kerning(face, glyphs[i], glyphs[i+1],
                                      FT_KERNING_UNSCALED, &delta);
        if (err) {
            return false;
        }
        adjustments[i] = delta.x;
    }
    return true;
}

/** Returns the bitmap strike equal to or just larger than the requested size. */
static FT_Int chooseBitmapStrike(FT_Face face, FT_F26Dot6 scaleY) {
    if (face == nullptr) {
        LOG_INFO("chooseBitmapStrike aborted due to nullptr face.\n");
        return -1;
    }

    FT_Pos requestedPPEM = scaleY;  // FT_Bitmap_Size::y_ppem is in 26.6 format.
    FT_Int chosenStrikeIndex = -1;
    FT_Pos chosenPPEM = 0;
    for (FT_Int strikeIndex = 0; strikeIndex < face->num_fixed_sizes; ++strikeIndex) {
        FT_Pos strikePPEM = face->available_sizes[strikeIndex].y_ppem;
        if (strikePPEM == requestedPPEM) {
            // exact match - our search stops here
            return strikeIndex;
        } else if (chosenPPEM < requestedPPEM) {
            // attempt to increase chosenPPEM
            if (chosenPPEM < strikePPEM) {
                chosenPPEM = strikePPEM;
                chosenStrikeIndex = strikeIndex;
            }
        } else {
            // attempt to decrease chosenPPEM, but not below requestedPPEM
            if (requestedPPEM < strikePPEM && strikePPEM < chosenPPEM) {
                chosenPPEM = strikePPEM;
                chosenStrikeIndex = strikeIndex;
            }
        }
    }
    return chosenStrikeIndex;
}

SkScalerContext_FreeType::SkScalerContext_FreeType(sk_sp<SkTypeface_FreeType> typeface,
                                                   const SkScalerContextEffects& effects,
                                                   const SkDescriptor* desc)
    : SkScalerContext_FreeType_Base(std::move(typeface), effects, desc)
    , fFace(nullptr)
    , fFTSize(nullptr)
    , fsize2_pp(0)
    , fStrikeIndex(-1)
{
    MLOG("In SkScalerContext_FreeType::ctor");

    SkAutoMutexExclusive  ac(f_t_mutex());
    fFaceRec = static_cast<SkTypeface_FreeType*>(this->getTypeface())->getFaceRec();

    // load the font file
    if (nullptr == fFaceRec) {
        LOG_INFO("Could not create FT_Face.\n");
        return;
    }

    fLCDIsVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);

    // compute the flags we send to Load_Glyph
    bool linearMetrics = this->isLinearMetrics();
    {
        FT_Int32 loadFlags = FT_LOAD_DEFAULT;

        if (SkMask::kBW_Format == fRec.fMaskFormat) {
            // See http://code.google.com/p/chromium/issues/detail?id=43252#c24
            loadFlags = FT_LOAD_TARGET_MONO;
            if (fRec.getHinting() == SkFontHinting::kNone) {
                loadFlags |= FT_LOAD_NO_HINTING;
                linearMetrics = true;
            }
        } else {
            switch (fRec.getHinting()) {
            case SkFontHinting::kNone:
                loadFlags = FT_LOAD_NO_HINTING;
                linearMetrics = true;
                break;
            case SkFontHinting::kSlight:
                loadFlags = FT_LOAD_TARGET_LIGHT;  // This implies FORCE_AUTOHINT
                linearMetrics = true;
                break;
            case SkFontHinting::kNormal:
                loadFlags = FT_LOAD_TARGET_NORMAL;
                break;
            case SkFontHinting::kFull:
                loadFlags = FT_LOAD_TARGET_NORMAL;
                if (isLCD(fRec)) {
                    if (fLCDIsVert) {
                        loadFlags = FT_LOAD_TARGET_LCD_V;
                    } else {
                        loadFlags = FT_LOAD_TARGET_LCD;
                    }
                }
                break;
            default:
                LOG_INFO("---------- UNKNOWN hinting %d\n", fRec.getHinting());
                break;
            }
        }

        if (fRec.fFlags & SkScalerContext::kForceAutohinting_Flag) {
            loadFlags |= FT_LOAD_FORCE_AUTOHINT;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        } else {
            loadFlags |= FT_LOAD_NO_AUTOHINT;
#endif
        }

        if ((fRec.fFlags & SkScalerContext::kEmbeddedBitmapText_Flag) == 0) {
            loadFlags |= FT_LOAD_NO_BITMAP;
        }

        // Always using FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH to get correct
        // advances, as fontconfig and cairo do.
        // See http://code.google.com/p/skia/issues/detail?id=222.
        loadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

        // Use vertical layout if requested.
        if (this->isVertical()) {
            loadFlags |= FT_LOAD_VERTICAL_LAYOUT;
        }

        fLoadGlyphFlags = loadFlags;
    }

    SkUniqueFTSize ftSize([this]() -> FT_Size {
        MLOG(" -- Calling FT_New_Size");

        FT_Size size;
        FT_Error err = FT_New_Size(fFaceRec->fFace.get(), &size);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_New_Size(%s) failed.", fFaceRec->fFace->family_name);
            return nullptr;
        }
        return size;
    }());
    if (nullptr == ftSize) {
        LOG_INFO("Could not create FT_Size.\n");
        return;
    }

    FT_Error err = FT_Activate_Size(ftSize.get());
    if (err != 0) {
        SK_TRACEFTR(err, "FT_Activate_Size(%s) failed.", fFaceRec->fFace->family_name);
        return;
    }

    fsize2_pp = 0;
ONOFF(

    u32 ft2_size_pp = Z_mallocZ_ii(4);
    if (Z_FT_New_SizeZ_iii(read_u32(fFaceRec->face2_pp), ft2_size_pp)) {
        printf ("FT_New_Size failed in sandbox\n");
        return;
    }
    if (Z_FT_Activate_SizeZ_ii(read_u32(ft2_size_pp))) {
        printf("FT_Activate_Size failed in sandbox\n");
        return;
    }
    fsize2_pp = ft2_size_pp;
)
    fRec.computeMatrices(SkScalerContextRec::PreMatrixScale::kFull, &fScale, &fMatrix22Scalar);
    FT_F26Dot6 scaleX = SkScalarToFDot6(fScale.fX);
    FT_F26Dot6 scaleY = SkScalarToFDot6(fScale.fY);

    if (FT_IS_SCALABLE(fFaceRec->fFace)) {
        err = FT_Set_Char_Size(fFaceRec->fFace.get(), scaleX, scaleY, 72, 72);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_Set_CharSize(%s, %f, %f) failed.",
                        fFaceRec->fFace->family_name, fScale.fX, fScale.fY);
            return;
        }

ONOFF(

        // This makes the glyphs of uneven sizes?
        if (Z_FT_Set_Char_SizeZ_iiiiii(read_u32(fFaceRec->face2_pp), scaleX, scaleY, 72, 72)) {
            printf("FT_Set_Char_Size failed in sandbox\n");
            return;
        }
)
        // Adjust the matrix to reflect the actually chosen scale.
        // FreeType currently does not allow requesting sizes less than 1, this allow for scaling.
        // Don't do this at all sizes as that will interfere with hinting.
        if (fScale.fX < 1 || fScale.fY < 1) {
            SkScalar upem = fFaceRec->fFace->units_per_EM;
            FT_Size_Metrics& ftmetrics = fFaceRec->fFace->size->metrics;
            SkScalar x_ppem = upem * SkFT_FixedToScalar(ftmetrics.x_scale) / 64.0f;
            SkScalar y_ppem = upem * SkFT_FixedToScalar(ftmetrics.y_scale) / 64.0f;
            fMatrix22Scalar.preScale(fScale.x() / x_ppem, fScale.y() / y_ppem);
        }

        // FT_LOAD_COLOR with scalable fonts means allow SVG.
        // It also implies attempt to render COLR if available, but this is not used.
#if defined(FT_CONFIG_OPTION_SVG)
        if (SkGraphics::GetOpenTypeSVGDecoderFactory()) {
            fLoadGlyphFlags |= FT_LOAD_COLOR;
        }
#endif
    } else if (FT_HAS_FIXED_SIZES(fFaceRec->fFace)) {
        fStrikeIndex = chooseBitmapStrike(fFaceRec->fFace.get(), scaleY);
        if (fStrikeIndex == -1) {
            LOG_INFO("No glyphs for font \"%s\" size %f.\n",
                     fFaceRec->fFace->family_name, fScale.fY);
            return;
        }

        err = FT_Select_Size(fFaceRec->fFace.get(), fStrikeIndex);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_Select_Size(%s, %d) failed.",
                        fFaceRec->fFace->family_name, fStrikeIndex);
            fStrikeIndex = -1;
            return;
        }

ONOFF(
        if (Z_FT_Select_SizeZ_iii(read_u32(fFaceRec->face2_pp), fStrikeIndex)) {
            printf("FT_Select_Size failed in sandbox\n");
            return;
        }
)
        // Adjust the matrix to reflect the actually chosen scale.
        // It is likely that the ppem chosen was not the one requested, this allows for scaling.
        fMatrix22Scalar.preScale(fScale.x() / fFaceRec->fFace->size->metrics.x_ppem,
                                 fScale.y() / fFaceRec->fFace->size->metrics.y_ppem);

        // FreeType does not provide linear metrics for bitmap fonts.
        linearMetrics = false;

        // FreeType documentation says:
        // FT_LOAD_NO_BITMAP -- Ignore bitmap strikes when loading.
        // Bitmap-only fonts ignore this flag.
        //
        // However, in FreeType 2.5.1 color bitmap only fonts do not ignore this flag.
        // Force this flag off for bitmap only fonts.
        fLoadGlyphFlags &= ~FT_LOAD_NO_BITMAP;

        // Color bitmaps are supported.
        fLoadGlyphFlags |= FT_LOAD_COLOR;
    } else {
        LOG_INFO("Unknown kind of font \"%s\" size %f.\n", fFaceRec->fFace->family_name, fScale.fY);
        return;
    }

    fMatrix22.xx = SkScalarToFixed(fMatrix22Scalar.getScaleX());
    fMatrix22.xy = SkScalarToFixed(-fMatrix22Scalar.getSkewX());
    fMatrix22.yx = SkScalarToFixed(-fMatrix22Scalar.getSkewY());
    fMatrix22.yy = SkScalarToFixed(fMatrix22Scalar.getScaleY());

    fFTSize = ftSize.release();
    fFace = fFaceRec->fFace.get();
    face2_pp = fFaceRec->face2_pp;
    fDoLinearMetrics = linearMetrics;
}

SkScalerContext_FreeType::~SkScalerContext_FreeType() {
    SkAutoMutexExclusive  ac(f_t_mutex());
    MLOG("In SkScalerContext_FreeType::dtor");

    if (fFTSize != nullptr) {
        FT_Done_Size(fFTSize);
    }

    fFaceRec = nullptr;
}

/*  We call this before each use of the fFace, since we may be sharing
    this face with other context (at different sizes).
*/
FT_Error SkScalerContext_FreeType::setupSize() {
    f_t_mutex().assertHeld();
    MLOG("In SkScalerContext_FreeType::setupSize");

    FT_Error err = FT_Activate_Size(fFTSize);
    if (err != 0) {
        return err;
    }
ONOFF(

    err = Z_FT_Activate_SizeZ_ii(read_u32(fsize2_pp));
    if (err != 0) {
        printf("Z_FT_Activate_SizeZ_ii Failed!\n");
        return err;
    }
    int sz_mat = sizeof(fMatrix22);
    u32 mat = Z_mallocZ_ii(sz_mat);
    // Direct copy doesn't work!
    // memcpy(wasm_addr_to_absolute(mat), &fMatrix22, sz_mat);

    // Separate assignment works!
    write_u32(mat, fMatrix22.xx);
    write_u32(mat+4, fMatrix22.xy);
    write_u32(mat+8, fMatrix22.yx);
    write_u32(mat+12, fMatrix22.yy);
)
    FT_Set_Transform(fFace, &fMatrix22, nullptr);
    // This makes the fonts disappear?
    // After mat is written to by separate members, the font reappears and size seems now correct
    // And after setting the transforms, the coordniates loaded are completely correct!
    printf("fMatrix22 (%lu %lu) = %ld %ld;%ld %ld\n", sizeof(fMatrix22), sizeof(FT_Matrix), fMatrix22.xx, fMatrix22.xy,
            fMatrix22.yx, fMatrix22.yy);
ONOFF(
    outbuf(mat, 16);
)
    //Z_FT_Set_TransformZ_viii(read_u32(face2_pp), mat, 0);
    return 0;
}

bool SkScalerContext_FreeType::generateAdvance(SkGlyph* glyph) {
   /* unhinted and light hinted text have linearly scaled advances
    * which are very cheap to compute with some font formats...
    */
    MLOG("In SkScalerContext_FreeType::generateAdvance");
    // Unused.

    if (!fDoLinearMetrics) {
        return false;
    }

    SkAutoMutexExclusive  ac(f_t_mutex());

    if (this->setupSize()) {
        glyph->zeroMetrics();
        return true;
    }

    FT_Error    error;
    FT_Fixed    advance;

    error = FT_Get_Advance( fFace, glyph->getGlyphID(),
                            fLoadGlyphFlags | FT_ADVANCE_FLAG_FAST_ONLY,
                            &advance );

    if (error != 0) {
        return false;
    }

    const SkScalar advanceScalar = SkFT_FixedToScalar(advance);
    glyph->fAdvanceX = SkScalarToFloat(fMatrix22Scalar.getScaleX() * advanceScalar);
    glyph->fAdvanceY = SkScalarToFloat(fMatrix22Scalar.getSkewY() * advanceScalar);
    return true;
}

bool SkScalerContext_FreeType::getBoundsOfCurrentOutlineGlyph(FT_GlyphSlot glyph, SkRect* bounds) {
    MLOG("In SkScalerContext_FreeType::getBoundsOfCurrentOutlineGlyph");

    if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        SkASSERT(false);
        return false;
    }
    if (0 == glyph->outline.n_contours) {
        return false;
    }

    FT_BBox bbox;
    FT_Outline_Get_CBox(&glyph->outline, &bbox);
    *bounds = SkRect::MakeLTRB(SkFDot6ToScalar(bbox.xMin), -SkFDot6ToScalar(bbox.yMax),
                               SkFDot6ToScalar(bbox.xMax), -SkFDot6ToScalar(bbox.yMin));
    return true;
}

bool SkScalerContext_FreeType::getCBoxForLetter(char letter, FT_BBox* bbox) {
    MLOG("In SkScalerContext_FreeType::getCBoxForLetter");
    
    const FT_UInt glyph_id = FT_Get_Char_Index(fFace, letter);
    if (!glyph_id) {
        return false;
    }

ONOFF(
    const FT_UInt glyph_id2 = Z_FT_Get_Char_IndexZ_iii(read_u32(face2_pp), letter);
    if (!glyph_id) {
        return false;
    }


    printf("GID1 = %d, GID2 = %d\n", glyph_id, glyph_id2);
)
       MLOG2("Load Glyph", letter);

    if (FT_Load_Glyph(fFace, glyph_id, fLoadGlyphFlags)) {
        return false;
    }
    if (fFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        return false;
    }

ONOFF(
    if (Z_FT_Load_GlyphZ_iiii(read_u32(face2_pp), glyph_id2, fLoadGlyphFlags)) {
        printf(" ERROR: loading glyph outside sandbox is okay but in the sandbox is not.");
        return false;
    }
)

    // TODO add face2_pp->glyph->format
ONOFF(
    u32 face2_p = read_u32(face2_pp);
    u32 glyph2_p = read_u32(face2_p + 21 * 4); // offset of glyph
)

#if DEMO_ON == 1
emboldenIfNeeded(fFace, fFace->glyph, SkTo<SkGlyphID>(glyph_id), face2_p, glyph2_p, SkTo<SkGlyphID>(glyph_id2));
#else
emboldenIfNeeded(fFace, fFace->glyph, SkTo<SkGlyphID>(glyph_id), 0, 0, 0);
#endif

    FT_Outline_Get_CBox(&fFace->glyph->outline, bbox);
    
    return true;
}

void SkScalerContext_FreeType::setGlyphBounds(SkGlyph* glyph, SkRect* bounds, bool subpixel) {
    SkIRect irect;
    if (bounds->isEmpty()) {
        irect = SkIRect::MakeEmpty();
    } else {
        if (subpixel) {
            bounds->offset(SkFixedToScalar(glyph->getSubXFixed()),
                           SkFixedToScalar(glyph->getSubYFixed()));
        }

        irect = bounds->roundOut();
        if (!SkTFitsIn<decltype(glyph->fWidth )>(irect.width ()) ||
            !SkTFitsIn<decltype(glyph->fHeight)>(irect.height()) ||
            !SkTFitsIn<decltype(glyph->fTop   )>(irect.top   ()) ||
            !SkTFitsIn<decltype(glyph->fLeft  )>(irect.left  ())  )
        {
            irect = SkIRect::MakeEmpty();
        }
    }
    glyph->fWidth  = SkToU16(irect.width ());
    glyph->fHeight = SkToU16(irect.height());
    glyph->fTop    = SkToS16(irect.top   ());
    glyph->fLeft   = SkToS16(irect.left  ());
}

void SkScalerContext_FreeType::updateGlyphBoundsIfLCD(SkGlyph* glyph) {
    if (glyph->fMaskFormat == SkMask::kLCD16_Format &&
        glyph->fWidth > 0 && glyph->fHeight > 0)
    {
        if (fLCDIsVert) {
            glyph->fHeight += 2;
            glyph->fTop -= 1;
        } else {
            glyph->fWidth += 2;
            glyph->fLeft -= 1;
        }
    }
}

bool SkScalerContext_FreeType::shouldSubpixelBitmap(const SkGlyph& glyph, const SkMatrix& matrix) {
    MLOG("In SkScalerContext_FreeType::shouldSubpixelBitmap");

    // If subpixel rendering of a bitmap *can* be done.
    bool mechanism = fFace->glyph->format == FT_GLYPH_FORMAT_BITMAP &&
                     this->isSubpixel() &&
                     (glyph.getSubXFixed() || glyph.getSubYFixed());

    // If subpixel rendering of a bitmap *should* be done.
    // 1. If the face is not scalable then always allow subpixel rendering.
    //    Otherwise, if the font has an 8ppem strike 7 will subpixel render but 8 won't.
    // 2. If the matrix is already not identity the bitmap will already be resampled,
    //    so resampling slightly differently shouldn't make much difference.
    bool policy = !FT_IS_SCALABLE(fFace) || !matrix.isIdentity();

    return mechanism && policy;
}

void SkScalerContext_FreeType::generateMetrics(SkGlyph* glyph, SkArenaAlloc* alloc) {
    SkAutoMutexExclusive  ac(f_t_mutex());
    MLOG("In SkScalerContext_FreeType::generateMetrics");

    if (this->setupSize()) {
        glyph->zeroMetrics();
        return;
    }

    FT_Bool haveLayers = false;
#ifdef FT_COLOR_H
    // See https://skbug.com/12945, if the face isn't marked scalable then paths cannot be loaded.
    if (FT_IS_SCALABLE(fFace)) {
        SkRect bounds = SkRect::MakeEmpty();
#ifdef TT_SUPPORT_COLRV1
        FT_OpaquePaint opaqueLayerPaint{nullptr, 1};
        MLOG(" In FT_SCALABLE");
        if (FT_Get_Color_Glyph_Paint(fFace, glyph->getGlyphID(),
                                     FT_COLOR_INCLUDE_ROOT_TRANSFORM, &opaqueLayerPaint)) {
            haveLayers = true;
            glyph->fScalerContextBits = ScalerContextBits::COLRv1;

            // COLRv1 optionally provides a ClipBox.
            FT_ClipBox clipBox;
            if (FT_Get_Color_Glyph_ClipBox(fFace, glyph->getGlyphID(), &clipBox)) {
                // Find bounding box of clip box corner points, needed when clipbox is transformed.
                FT_BBox bbox;
                bbox.xMin = clipBox.bottom_left.x;
                bbox.xMax = clipBox.bottom_left.x;
                bbox.yMin = clipBox.bottom_left.y;
                bbox.yMax = clipBox.bottom_left.y;
                for (auto& corner : {clipBox.top_left, clipBox.top_right, clipBox.bottom_right}) {
                    bbox.xMin = std::min(bbox.xMin, corner.x);
                    bbox.yMin = std::min(bbox.yMin, corner.y);
                    bbox.xMax = std::max(bbox.xMax, corner.x);
                    bbox.yMax = std::max(bbox.yMax, corner.y);
                }
                bounds = SkRect::MakeLTRB(SkFDot6ToScalar(bbox.xMin), -SkFDot6ToScalar(bbox.yMax),
                                          SkFDot6ToScalar(bbox.xMax), -SkFDot6ToScalar(bbox.yMin));
            } else {
                // Traverse the glyph graph with a focus on measuring the required bounding box.
                // The call to computeColrV1GlyphBoundingBox may modify the face.
                // Reset the face to load the base glyph for metrics.
                if (!computeColrV1GlyphBoundingBox(fFace, glyph->getGlyphID(), &bounds) ||
                    this->setupSize())
                {
                    glyph->zeroMetrics();
                    return;
                }
            }
        }
#endif // #TT_SUPPORT_COLRV1

        if (!haveLayers) {
            FT_LayerIterator layerIterator = { 0, 0, nullptr };
            FT_UInt layerGlyphIndex;
            FT_UInt layerColorIndex;
            FT_Int32 flags = fLoadGlyphFlags;
            flags |= FT_LOAD_BITMAP_METRICS_ONLY;  // Don't decode any bitmaps.
            flags |= FT_LOAD_NO_BITMAP; // Ignore embedded bitmaps.
            flags &= ~FT_LOAD_RENDER;  // Don't scan convert.
            flags &= ~FT_LOAD_COLOR;  // Ignore SVG.
            // For COLRv0 compute the glyph bounding box from the union of layer bounding boxes.
            while (FT_Get_Color_Glyph_Layer(fFace, glyph->getGlyphID(), &layerGlyphIndex,
                                            &layerColorIndex, &layerIterator)) {
                haveLayers = true;
                       MLOG3ID(" -- Load Glyph", glyph->getGlyphID(), layerGlyphIndex);

                if (FT_Load_Glyph(fFace, layerGlyphIndex, flags)) {
                    glyph->zeroMetrics();
                    return;
                }

                       MLOG3ID(" -- Load Gly2", glyph->getGlyphID(), layerGlyphIndex);

                SkRect currentBounds;
                if (getBoundsOfCurrentOutlineGlyph(fFace->glyph, &currentBounds)) {
                    bounds.join(currentBounds);
                }
            }
            if (haveLayers) {
                glyph->fScalerContextBits = ScalerContextBits::COLRv0;
            }
        }

        if (haveLayers) {
            glyph->fMaskFormat = SkMask::kARGB32_Format;
            glyph->setPath(alloc, nullptr, false);
            setGlyphBounds(glyph, &bounds, this->isSubpixel());
        }
    }
#endif  //FT_COLOR_H
       MLOG2ID(" - Load Glyph", glyph->getGlyphID());

    // Even if haveLayers, the base glyph must be loaded to get the metrics.
    if (FT_Load_Glyph(fFace, glyph->getGlyphID(), fLoadGlyphFlags | FT_LOAD_BITMAP_METRICS_ONLY)) {
        glyph->zeroMetrics();
        return;
    }


ONOFF(

    u32 face2_p = read_u32(face2_pp);
    u32 glyph2_p = read_u32(face2_p + 21 * 4);

    if (Z_FT_Load_GlyphZ_iiii(face2_p, glyph->getGlyphID(), fLoadGlyphFlags | FT_LOAD_BITMAP_METRICS_ONLY )) {
        glyph->zeroMetrics();
        printf("!!Failed Load Glyph!!");
        fflush(stdout);
        return;
    }
)
    if (!haveLayers) {
        MLOG("  No LAYERS ");
#if DEMO_ON == 1
        emboldenIfNeeded(fFace, fFace->glyph, glyph->getGlyphID(), face2_p, glyph2_p, glyph->getGlyphID());
#else
        emboldenIfNeeded(fFace, fFace->glyph, glyph->getGlyphID(), 0, 0, 0);
#endif

        if (fFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
            MLOG("   --- OUTLINE");
            SkRect bounds;
            if (!getBoundsOfCurrentOutlineGlyph(fFace->glyph, &bounds)) {
                bounds = SkRect::MakeEmpty();
            }
            setGlyphBounds(glyph, &bounds, this->isSubpixel());
            updateGlyphBoundsIfLCD(glyph);

        } else if (fFace->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
                    MLOG("   --- BITMAP");

            glyph->setPath(alloc, nullptr, false);

            if (this->isVertical()) {
                FT_Vector vector;
                vector.x =  fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
                vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
                FT_Vector_Transform(&vector, &fMatrix22);
                fFace->glyph->bitmap_left += SkFDot6Floor(vector.x);
                fFace->glyph->bitmap_top  += SkFDot6Floor(vector.y);
            }

            if (fFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
                glyph->fMaskFormat = SkMask::kARGB32_Format;
            }

            SkRect bounds = SkRect::MakeXYWH(SkIntToScalar(fFace->glyph->bitmap_left ),
                                            -SkIntToScalar(fFace->glyph->bitmap_top  ),
                                             SkIntToScalar(fFace->glyph->bitmap.width),
                                             SkIntToScalar(fFace->glyph->bitmap.rows ));
            fMatrix22Scalar.mapRect(&bounds);
            setGlyphBounds(glyph, &bounds, this->shouldSubpixelBitmap(*glyph, fMatrix22Scalar));

#if defined(FT_CONFIG_OPTION_SVG)
        } else if (fFace->glyph->format == FT_GLYPH_FORMAT_SVG) {
                            MLOG("   --- SVG");

            glyph->fScalerContextBits = ScalerContextBits::SVG;
            glyph->fMaskFormat = SkMask::kARGB32_Format;
            glyph->setPath(alloc, nullptr, false);

            SkPictureRecorder recorder;
            SkRect infiniteRect = SkRect::MakeLTRB(-SK_ScalarInfinity, -SK_ScalarInfinity,
                                                    SK_ScalarInfinity,  SK_ScalarInfinity);
            sk_sp<SkBBoxHierarchy> bboxh = SkRTreeFactory()();
            SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
            SkCanvas* recordingCanvas = recorder.beginRecording(infiniteRect, bboxh);
            if (!this->drawSVGGlyph(fFace, *glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                glyph->zeroMetrics();
                return;
            }
            sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
            SkRect bounds = pic->cullRect();
            SkASSERT(bounds.isFinite());

            // drawSVGGlyph already applied the subpixel positioning.
            setGlyphBounds(glyph, &bounds, false);
#endif  // FT_CONFIG_OPTION_SVG

        } else {
            SkDEBUGFAIL("unknown glyph format");
            glyph->zeroMetrics();
            return;
        }
    }

    if (this->isVertical()) {
        if (fDoLinearMetrics) {
            const SkScalar advanceScalar = SkFT_FixedToScalar(fFace->glyph->linearVertAdvance);
            glyph->fAdvanceX = SkScalarToFloat(fMatrix22Scalar.getSkewX() * advanceScalar);
            glyph->fAdvanceY = SkScalarToFloat(fMatrix22Scalar.getScaleY() * advanceScalar);
        } else {
            glyph->fAdvanceX = -SkFDot6ToFloat(fFace->glyph->advance.x);
            glyph->fAdvanceY =  SkFDot6ToFloat(fFace->glyph->advance.y);
        }
    } else {
        if (fDoLinearMetrics) {
            const SkScalar advanceScalar = SkFT_FixedToScalar(fFace->glyph->linearHoriAdvance);
            glyph->fAdvanceX = SkScalarToFloat(fMatrix22Scalar.getScaleX() * advanceScalar);
            glyph->fAdvanceY = SkScalarToFloat(fMatrix22Scalar.getSkewY() * advanceScalar);
        } else {
            glyph->fAdvanceX =  SkFDot6ToFloat(fFace->glyph->advance.x);
            glyph->fAdvanceY = -SkFDot6ToFloat(fFace->glyph->advance.y);
        }
    }

#ifdef ENABLE_GLYPH_SPEW
    LOG_INFO("Metrics(glyph:%d flags:0x%x) w:%d\n", glyph->getGlyphID(), fLoadGlyphFlags, glyph->fWidth);
#endif
}

void SkScalerContext_FreeType::generateImage(const SkGlyph& glyph) {
    SkAutoMutexExclusive  ac(f_t_mutex());
    MLOG("In SkScalerContext_FreeType::generateImage");

    if (this->setupSize()) {
        sk_bzero(glyph.fImage, glyph.imageSize());
        return;
    }

    if (glyph.fScalerContextBits == ScalerContextBits::COLRv0 ||
        glyph.fScalerContextBits == ScalerContextBits::COLRv1 ||
        glyph.fScalerContextBits == ScalerContextBits::SVG     )
    {
        SkASSERT(glyph.maskFormat() == SkMask::kARGB32_Format);
        SkBitmap dstBitmap;
        // TODO: mark this as sRGB when the blits will be sRGB.
        dstBitmap.setInfo(SkImageInfo::Make(glyph.fWidth, glyph.fHeight,
                                            kN32_SkColorType,
                                            kPremul_SkAlphaType),
                                            glyph.rowBytes());
        dstBitmap.setPixels(glyph.fImage);

        SkCanvas canvas(dstBitmap);
        if constexpr (kSkShowTextBlitCoverage) {
            canvas.clear(0x33FF0000);
        } else {
            canvas.clear(SK_ColorTRANSPARENT);
        }
        canvas.translate(-glyph.fLeft, -glyph.fTop);

        SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
        if (glyph.fScalerContextBits == ScalerContextBits::COLRv0) {
#ifdef FT_COLOR_H
            this->drawCOLRv0Glyph(fFace, glyph, fLoadGlyphFlags, palette, &canvas);
#endif
        } else if (glyph.fScalerContextBits == ScalerContextBits::COLRv1) {
#ifdef TT_SUPPORT_COLRV1
            this->drawCOLRv1Glyph(fFace, glyph, fLoadGlyphFlags, palette, &canvas);
#endif
        } else if (glyph.fScalerContextBits == ScalerContextBits::SVG) {
#if defined(FT_CONFIG_OPTION_SVG)
       MLOG2ID("Load Glyph", glyph.getGlyphID());
            if (glyph.getGlyphID() < 128) return; 
            if (FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags)) {
                return;
            }
ONOFF(
            if (Z_FT_Load_GlyphZ_iiii(read_u32(face2_pp, glyph.getGlyphID(), fLoadGlyphFlags))) {
                return;
            }
)
            this->drawSVGGlyph(fFace, glyph, fLoadGlyphFlags, palette, &canvas);
#endif
        }
        return;
    }
       MLOG2ID("Load Glyph", glyph.getGlyphID());
            // if (glyph.getGlyphID() < 128) return; 

        printf("LoadFLAGS : %d\n", fLoadGlyphFlags);
    if (FT_Load_Glyph(fFace, 
        //(glyph.getGlyphID() > 32)?glyph.getGlyphID()+1:glyph.getGlyphID(),
        glyph.getGlyphID(),
        fLoadGlyphFlags)) {
        sk_bzero(glyph.fImage, glyph.imageSize());
        return;
    }
ONOFF(
)
ONOFF(
    if (Z_FT_Load_GlyphZ_iiii(read_u32(face2_pp), glyph.getGlyphID(), fLoadGlyphFlags)) {
        printf("SAND LOAD failed face2_pp = %d!\n", face2_pp);
        sk_bzero(glyph.fImage, glyph.imageSize());
        return;
    }
    //FT_Outline_Embolden(&(fFace->glyph)->outline, 64);

    u32 face2_p = read_u32(face2_pp);
    u32 glyph_p = read_u32(face2_p + 21 * 4); // offset of glyph

    printf("+++face2_pp = %d, face2_p = %d\n", face2_pp, face2_p);
    printf("+++glyph_p = %d\n", glyph_p);
    fflush(stdout);


#if DEMO_ON==1
    emboldenIfNeeded(fFace, fFace->glyph, glyph.getGlyphID(), face2_p, glyph_p, glyph.getGlyphID());
#else
    emboldenIfNeeded(fFace, fFace->glyph, glyph.getGlyphID(), 0, 0, 0);
#endif
    // void* new_buf = malloc()


  u32 bitmap = glyph_p + 19 * 4;  // offset of glyph->bitmap ?? why 19? // looks like that object members are "flattened" instead of being a pointer
    u32 outline_p = glyph_p + 27 * 4;
    u32 outline_n_contours_p = outline_p + 0;
    u32 outline_n_points_p = outline_p + 2;
    u32 outline_points_pp = outline_p + 4;
    u32 outline_tags_pp = outline_p + 8;
    u32 outline_contours_pp = outline_p + 12;
    u32 outline_flags_p = outline_p + 16;
  int nr_2 = read_u32(bitmap);
  int nc_2 = read_u32(bitmap+4);
  int pitch_2 = read_u32(bitmap+8);
  u32 buf2_p = read_u32(bitmap+12);

    printf("-+-bitmap=%d\n", bitmap); fflush(stdout);
    printf("-++nr_2 = %d, nc_2=%d, pitch_2=%d, buf2_p=%d\n", nr_2, nc_2, pitch_2, buf2_p);

      NOTIMPLEMENTED();

    printf("SAND LOAD: face2_p: %u, glyph_p: %u, bitmap: %u, nr %d, nc %d, pitch %d, buf_p %d\n", face2_p, glyph_p, bitmap, nr_2, nc_2,pitch_2,buf2_p);
    printf("");
    printf("");
    printf("SAND LOAD: Outline Before: %d, %d, %p, %p, %d\n", 
        fFace->glyph->outline.n_contours,
        fFace->glyph->outline.n_points,
        fFace->glyph->outline.points, 
        fFace->glyph->outline.tags, 
        fFace->glyph->outline.flags);
    printf("Points before\n");
    for (int i = 0; i < fFace->glyph->outline.n_points; ++i) {
        printf("(%ld, %ld)\n",(fFace->glyph->outline.points)[i].x, (fFace->glyph->outline.points)[i].y);
    }
    for (int i = 0; i < fFace->glyph->outline.n_points;++i) {
        printf("%d\n", fFace->glyph->outline.tags[i]);
    }

    // printf("SAND LOAD: Outline Before: %ld, %ld, %ld, %ld, %ld, %d %d %d %d %d %d, %d %d %d %d\n", 
    //     fFace->num_faces, fFace->face_index, fFace->face_flags, fFace->style_flags, fFace->num_glyphs,
    //     fFace->num_fixed_sizes, fFace->num_charmaps, fFace->units_per_EM, fFace->ascender, fFace->descender, fFace->height,
    //     fFace->max_advance_width, fFace->max_advance_height, fFace->underline_position, fFace->underline_thickness);
    fFace->glyph->bitmap.buffer = (unsigned char*)wasm_addr_to_absolute(buf2_p);
    //fFace->glyph->outline.n_contours = read_u16(outline_n_contours_p);
    //fFace->glyph->outline.n_points = read_u16(outline_n_points_p);
    FT_Vector* old_points = fFace->glyph->outline.points;
    fFace->glyph->outline.points = (FT_Vector*)malloc(sizeof(FT_Vector) * fFace->glyph->outline.n_points);

    u32 outline_points_p = read_u32(outline_points_pp);

    // Cannot read as u32 and then convert. Need to read i32.
    for (int i = 0; i <fFace->glyph->outline.n_points; ++i) {
        fFace->glyph->outline.points[i].x = read_i32(outline_points_p+i*2*4); // old_points[i].x; // 
        fFace->glyph->outline.points[i].y = read_i32(outline_points_p+i*2*4+4); //old_points[i].y; // 
        //if (fFace->glyph->outline.points[i].x > 10000 ) fFace->glyph->outline.points[i].x = 0;
        //if (fFace->glyph->outline.points[i].y > 10000 ) fFace->glyph->outline.points[i].y = 0;
    }
//    (FT_Vector *)wasm_addr_to_absolute(read_u32(outline_points_pp));
    //fFace->glyph->outline.tags = (char*) malloc()
    
   // wasm_addr_to_absolute(read_u32(outline_tags_pp));
    //fFace->glyph->outline.contours = (short *)wasm_addr_to_absolute(read_u32(outline_contours_pp));
    printf("Points after\n");

    for (int i = 0; i < fFace->glyph->outline.n_points; ++i) {
        printf("(%ld, %ld)\n",(fFace->glyph->outline.points)[i].x, (fFace->glyph->outline.points)[i].y);
    }
    // for (int i = 0; i <fFace->glyph->outline.n_points; ++i) {
    //     fFace->glyph->outline.points[i].x =read_u32(outline_points_p+i*2*4);
    //     fFace->glyph->outline.points[i].y =read_u32(outline_points_p+i*2*4+4);
    // }
    outbuf8(read_u32(outline_tags_pp), fFace->glyph->outline.n_points*4);

    outbuf(outline_points_p, 20*4);
    //fFace->glyph->outline.flags = read_u32(outline_flags_p);
    // printf("SAND LOAD: Outline: %u, %u, %u, %u, %u\n", 
    //     read_u32(outline_n_contours_p),
    //     read_u32(outline_n_points_p),
    //     read_u32(outline_points_pp), 
    //     read_u32(outline_tags_pp), 
    //     read_u32(outline_tags_pp));
    printf("SAND LOAD: Outline After: %d, %d, %p, %p, %d\n", 
        fFace->glyph->outline.n_contours,
        fFace->glyph->outline.n_points,
        fFace->glyph->outline.points, 
        fFace->glyph->outline.tags, 
        fFace->glyph->outline.flags);
    // for (int i = 0; i < n_points; ++i) {
    //     printf("(%d, %d)\n",(fFace->glyph->outline.points)[i].x, (fFace->glyph->outline.points)[i].y);
    // }
    //outbuf(outline_p, 35*4);
    // for (int i = face2_p + 17*4; i < face2_p+17*4+8*2; i += 2) {
    //     printf("%d ", read_u16(i));
    // }

    for (int i = outline_p; i < outline_p+8*2; i += 2) {
        printf("%d ", read_u16(i));
    }
    printf("\n");
    fflush(stdout);
)
    SkMatrix* bitmapMatrix = &fMatrix22Scalar;
    SkMatrix subpixelBitmapMatrix;
    /* Intentionally commented out to see if shouldSubpixelBitmap still appears in fontfrog
    if (this->shouldSubpixelBitmap(glyph, *bitmapMatrix)) {
        subpixelBitmapMatrix = fMatrix22Scalar;
        subpixelBitmapMatrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                                           SkFixedToScalar(glyph.getSubYFixed()));
        bitmapMatrix = &subpixelBitmapMatrix;
    }
    */

    //fFace->glyph->outline.n_contours = glyph_p
    generateGlyphImage(fFace, glyph, *bitmapMatrix);
    printf("generateGlyph Image done!\n");
    fflush(stdout);
}

sk_sp<SkDrawable> SkScalerContext_FreeType::generateDrawable(const SkGlyph& glyph) {
    // Because FreeType's FT_Face is stateful (not thread safe) and the current design of this
    // SkTypeface and SkScalerContext does not work around this, it is necessary lock at least the
    // FT_Face when using it (this implementation currently locks the whole FT_Library).
    // It should be possible to draw the drawable straight out of the FT_Face. However, this would
    // mean locking each time any such drawable is drawn. To avoid locking, this implementation
    // creates drawables backed as pictures so that they can be played back later without locking.
    SkAutoMutexExclusive  ac(f_t_mutex());

    if (this->setupSize()) {
        return nullptr;
    }

#if defined(FT_COLOR_H) || defined(TT_SUPPORT_COLRV1) || defined(FT_CONFIG_OPTION_SVG)
    if (glyph.fScalerContextBits == ScalerContextBits::COLRv0 ||
        glyph.fScalerContextBits == ScalerContextBits::COLRv1 ||
        glyph.fScalerContextBits == ScalerContextBits::SVG     )
    {
        SkSpan<SkColor> palette(fFaceRec->fSkPalette.get(), fFaceRec->fFTPaletteEntryCount);
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkRect::Make(glyph.mask().fBounds));
        if (glyph.fScalerContextBits == ScalerContextBits::COLRv0) {
#ifdef FT_COLOR_H
            if (!this->drawCOLRv0Glyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return nullptr;
            }
#else
            return nullptr;
#endif
        } else if (glyph.fScalerContextBits == ScalerContextBits::COLRv1) {
#ifdef TT_SUPPORT_COLRV1
            if (!this->drawCOLRv1Glyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return nullptr;
            }
#else
            return nullptr;
#endif
        } else if (glyph.fScalerContextBits == ScalerContextBits::SVG) {
#if defined(FT_CONFIG_OPTION_SVG)
       MLOG2ID("Load Glyph", glyph->getGlyphID());
                if (glyph.getGlyphID() < 128) return; 

            if (FT_Load_Glyph(fFace, glyph.getGlyphID(), fLoadGlyphFlags)) {
                return nullptr;
            }
            if (!this->drawSVGGlyph(fFace, glyph, fLoadGlyphFlags, palette, recordingCanvas)) {
                return nullptr;
            }
#else
            return nullptr;
#endif
        }
        return recorder.finishRecordingAsDrawable();
    }
#endif
    return nullptr;
}

bool SkScalerContext_FreeType::generatePath(const SkGlyph& glyph, SkPath* path) {
    SkASSERT(path);

    SkAutoMutexExclusive  ac(f_t_mutex());
    MLOG("In SkScalerContext_FreeType::generatePath");

    SkGlyphID glyphID = glyph.getGlyphID();
    // FT_IS_SCALABLE is documented to mean the face contains outline glyphs.
    if (!FT_IS_SCALABLE(fFace) || this->setupSize()) {
        path->reset();
        return false;
    }

    uint32_t flags = fLoadGlyphFlags;
    flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
    flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)
       MLOG2ID("Load Glyph", glyphID);

    FT_Error err = FT_Load_Glyph(fFace, glyphID, flags);
    if (err != 0 || fFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        path->reset();
        return false;
    }

ONOFF(

    u32 face2_p = read_u32(face2_pp);
    u32 glyph2_p = read_u32(face2_p + 21 * 4);
    err = Z_FT_Load_GlyphZ_iiii(face2_p, glyphID, flags);
    if (err != 0) {
        printf ("  ERROR: load failure!\n");
        return false;
    }
)

#if DEMO_ON==1
    emboldenIfNeeded(fFace, fFace->glyph, glyphID, face2_p, glyph2_p, glyphID);
#else
    emboldenIfNeeded(fFace, fFace->glyph, glyphID, 0, 0, 0);
#endif

    if (!generateGlyphPath(fFace, path)) {
        path->reset();
        return false;
    }

    // The path's origin from FreeType is always the horizontal layout origin.
    // Offset the path so that it is relative to the vertical origin if needed.
    if (this->isVertical()) {
        FT_Vector vector;
        vector.x = fFace->glyph->metrics.vertBearingX - fFace->glyph->metrics.horiBearingX;
        vector.y = -fFace->glyph->metrics.vertBearingY - fFace->glyph->metrics.horiBearingY;
        FT_Vector_Transform(&vector, &fMatrix22);
        path->offset(SkFDot6ToScalar(vector.x), -SkFDot6ToScalar(vector.y));
    }
    return true;
}

void SkScalerContext_FreeType::generateFontMetrics(SkFontMetrics* metrics) {
    MLOG("In SkScalerContext_FreeType::generateFontMetrics");

    if (nullptr == metrics) {
        return;
    }

    SkAutoMutexExclusive ac(f_t_mutex());

    if (this->setupSize()) {
        sk_bzero(metrics, sizeof(*metrics));
        return;
    }

    FT_Face face = fFace;
    metrics->fFlags = 0;

    SkScalar upem = SkIntToScalar(SkTypeface_FreeType::GetUnitsPerEm(face));

    // use the os/2 table as a source of reasonable defaults.
    SkScalar x_height = 0.0f;
    SkScalar avgCharWidth = 0.0f;
    SkScalar cap_height = 0.0f;
    SkScalar strikeoutThickness = 0.0f, strikeoutPosition = 0.0f;
    TT_OS2* os2 = (TT_OS2*) FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2) {
        x_height = SkIntToScalar(os2->sxHeight) / upem * fScale.y();
        avgCharWidth = SkIntToScalar(os2->xAvgCharWidth) / upem;
        strikeoutThickness = SkIntToScalar(os2->yStrikeoutSize) / upem;
        strikeoutPosition = -SkIntToScalar(os2->yStrikeoutPosition) / upem;
        metrics->fFlags |= SkFontMetrics::kStrikeoutThicknessIsValid_Flag;
        metrics->fFlags |= SkFontMetrics::kStrikeoutPositionIsValid_Flag;
        if (os2->version != 0xFFFF && os2->version >= 2) {
            cap_height = SkIntToScalar(os2->sCapHeight) / upem * fScale.y();
        }
    }

    // pull from format-specific metrics as needed
    SkScalar ascent, descent, leading, xmin, xmax, ymin, ymax;
    SkScalar underlineThickness, underlinePosition;
    if (face->face_flags & FT_FACE_FLAG_SCALABLE) { // scalable outline font
        // FreeType will always use HHEA metrics if they're not zero.
        // It completely ignores the OS/2 fsSelection::UseTypoMetrics bit.
        // It also ignores the VDMX tables, which are also of interest here
        // (and override everything else when they apply).
        static const int kUseTypoMetricsMask = (1 << 7);
        if (os2 && os2->version != 0xFFFF && (os2->fsSelection & kUseTypoMetricsMask)) {
            ascent = -SkIntToScalar(os2->sTypoAscender) / upem;
            descent = -SkIntToScalar(os2->sTypoDescender) / upem;
            leading = SkIntToScalar(os2->sTypoLineGap) / upem;
        } else {
            ascent = -SkIntToScalar(face->ascender) / upem;
            descent = -SkIntToScalar(face->descender) / upem;
            leading = SkIntToScalar(face->height + (face->descender - face->ascender)) / upem;
        }
        xmin = SkIntToScalar(face->bbox.xMin) / upem;
        xmax = SkIntToScalar(face->bbox.xMax) / upem;
        ymin = -SkIntToScalar(face->bbox.yMin) / upem;
        ymax = -SkIntToScalar(face->bbox.yMax) / upem;
        underlineThickness = SkIntToScalar(face->underline_thickness) / upem;
        underlinePosition = -SkIntToScalar(face->underline_position +
                                           face->underline_thickness / 2) / upem;

        metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
        metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;

        // we may be able to synthesize x_height and cap_height from outline
        if (!x_height) {
            FT_BBox bbox;
            if (getCBoxForLetter('x', &bbox)) {
                x_height = SkIntToScalar(bbox.yMax) / 64.0f;
            }
        }
        if (!cap_height) {
            FT_BBox bbox;
            if (getCBoxForLetter('H', &bbox)) {
                cap_height = SkIntToScalar(bbox.yMax) / 64.0f;
            }
        }
    } else if (fStrikeIndex != -1) { // bitmap strike metrics
        SkScalar xppem = SkIntToScalar(face->size->metrics.x_ppem);
        SkScalar yppem = SkIntToScalar(face->size->metrics.y_ppem);
        ascent = -SkIntToScalar(face->size->metrics.ascender) / (yppem * 64.0f);
        descent = -SkIntToScalar(face->size->metrics.descender) / (yppem * 64.0f);
        leading = (SkIntToScalar(face->size->metrics.height) / (yppem * 64.0f)) + ascent - descent;

        xmin = 0.0f;
        xmax = SkIntToScalar(face->available_sizes[fStrikeIndex].width) / xppem;
        ymin = descent;
        ymax = ascent;
        // The actual bitmaps may be any size and placed at any offset.
        metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;

        underlineThickness = 0;
        underlinePosition = 0;
        metrics->fFlags &= ~SkFontMetrics::kUnderlineThicknessIsValid_Flag;
        metrics->fFlags &= ~SkFontMetrics::kUnderlinePositionIsValid_Flag;

        TT_Postscript* post = (TT_Postscript*) FT_Get_Sfnt_Table(face, ft_sfnt_post);
        if (post) {
            underlineThickness = SkIntToScalar(post->underlineThickness) / upem;
            underlinePosition = -SkIntToScalar(post->underlinePosition) / upem;
            metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
            metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;
        }
    } else {
        sk_bzero(metrics, sizeof(*metrics));
        return;
    }

    // synthesize elements that were not provided by the os/2 table or format-specific metrics
    if (!x_height) {
        x_height = -ascent * fScale.y();
    }
    if (!avgCharWidth) {
        avgCharWidth = xmax - xmin;
    }
    if (!cap_height) {
      cap_height = -ascent * fScale.y();
    }

    // disallow negative linespacing
    if (leading < 0.0f) {
        leading = 0.0f;
    }

    metrics->fTop = ymax * fScale.y();
    metrics->fAscent = ascent * fScale.y();
    metrics->fDescent = descent * fScale.y();
    metrics->fBottom = ymin * fScale.y();
    metrics->fLeading = leading * fScale.y();
    metrics->fAvgCharWidth = avgCharWidth * fScale.y();
    metrics->fXMin = xmin * fScale.y();
    metrics->fXMax = xmax * fScale.y();
    metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
    metrics->fXHeight = x_height;
    metrics->fCapHeight = cap_height;
    metrics->fUnderlineThickness = underlineThickness * fScale.y();
    metrics->fUnderlinePosition = underlinePosition * fScale.y();
    metrics->fStrikeoutThickness = strikeoutThickness * fScale.y();
    metrics->fStrikeoutPosition = strikeoutPosition * fScale.y();

    if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS
#if defined(FT_CONFIG_OPTION_SVG)
        || face->face_flags & FT_FACE_FLAG_SVG
#endif  // FT_CONFIG_OPTION_SVG
    ) {
        // The bounds are only valid for the default variation of variable glyphs.
        // https://docs.microsoft.com/en-us/typography/opentype/spec/head
        // For SVG glyphs this number is often incorrect for its non-`glyf` points.
        // https://github.com/fonttools/fonttools/issues/2566
        metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;
    }
}

///////////////////////////////////////////////////////////////////////////////

// hand-tuned value to reduce outline embolden strength
#ifndef SK_OUTLINE_EMBOLDEN_DIVISOR
    #ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        #define SK_OUTLINE_EMBOLDEN_DIVISOR   34
    #else
        #define SK_OUTLINE_EMBOLDEN_DIVISOR   24
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////

void SkScalerContext_FreeType::emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph, SkGlyphID gid, u32 f2_p, u32 glyph2_p, SkGlyphID gid2) {
    // check to see if the embolden bit is set
    MLOG("In SkScalerContext_FreeType::emboldenIfNeeded");

    if (0 == (fRec.fFlags & SkScalerContext::kEmbolden_Flag)) {
        return;
    }

    switch (glyph->format) {
        case FT_GLYPH_FORMAT_OUTLINE:
                   MLOG2ID("  OUTLINE: Load Glyph", gid);

            FT_Pos strength;
            strength = FT_MulFix(face->units_per_EM, face->size->metrics.y_scale)
                       / SK_OUTLINE_EMBOLDEN_DIVISOR;
            FT_Outline_Embolden(&glyph->outline, strength);
            break;
        case FT_GLYPH_FORMAT_BITMAP:
            if (!fFace->glyph->bitmap.buffer) {
       MLOG2ID("  BITMAP: Load Glyph", gid);

                FT_Load_Glyph(fFace, gid, fLoadGlyphFlags);
ONOFF(
                Z_FT_Load_GlyphZ_iiii(read_u32(face2_pp), gid2, fLoadGlyphFlags);
)
            }
            FT_GlyphSlot_Own_Bitmap(glyph);
            FT_Bitmap_Embolden(glyph->library, &glyph->bitmap, kBitmapEmboldenStrength, 0);
            break;
        default:
            SkDEBUGFAIL("unknown glyph format");
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "src/base/SkUtils.h"

SkTypeface_FreeType::SkTypeface_FreeType(const SkFontStyle& style, bool isFixedPitch)
    : INHERITED(style, isFixedPitch)
{}

SkTypeface_FreeType::~SkTypeface_FreeType() {
    if (fFaceRec) {
        SkAutoMutexExclusive ac(f_t_mutex());
        fFaceRec.reset();
    }
}

// Just made up, so we don't end up storing 1000s of entries
constexpr int kMaxC2GCacheCount = 512;

void SkTypeface_FreeType::onCharsToGlyphs(const SkUnichar uni[], int count,
                                          SkGlyphID glyphs[]) const {
    // Try the cache first, *before* accessing freetype lib/face, as that
    // can be very slow. If we do need to compute a new glyphID, then
    // access those freetype objects and continue the loop.
    MLOG("SkTypeface_FreeType::onCharsToGlyphs");

    int i;
    {
        // Optimistically use a shared lock.
        SkAutoSharedMutexShared ama(fC2GCacheMutex);
        for (i = 0; i < count; ++i) {
            int index = fC2GCache.findGlyphIndex(uni[i]);
            if (index < 0) {
                break;
            }
            glyphs[i] = SkToU16(index);
        }
        if (i == count) {
            // we're done, no need to access the freetype objects
            return;
        }
    }

    // Need to add more so grab an exclusive lock.
    SkAutoSharedMutexExclusive ama(fC2GCacheMutex);
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        sk_bzero(glyphs, count * sizeof(glyphs[0]));
        return;
    }

    for (; i < count; ++i) {
        SkUnichar c = uni[i];
        int index = fC2GCache.findGlyphIndex(c);
        if (index >= 0) {
            glyphs[i] = SkToU16(index);
        } else {
            glyphs[i] = SkToU16(FT_Get_Char_Index(face, c));
            fC2GCache.insertCharAndGlyph(~index, c, glyphs[i]);
        }
    }

    if (fC2GCache.count() > kMaxC2GCacheCount) {
        fC2GCache.reset();
    }
}

int SkTypeface_FreeType::onCountGlyphs() const {
    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    return face ? face->num_glyphs : 0;
}

SkTypeface::LocalizedStrings* SkTypeface_FreeType::onCreateFamilyNameIterator() const {
    sk_sp<SkTypeface::LocalizedStrings> nameIter =
        SkOTUtils::LocalizedStrings_NameTable::MakeForFamilyNames(*this);
    if (!nameIter) {
        SkString familyName;
        this->getFamilyName(&familyName);
        SkString language("und"); //undetermined
        nameIter = sk_make_sp<SkOTUtils::LocalizedStrings_SingleName>(familyName, language);
    }
    return nameIter.release();
}

bool SkTypeface_FreeType::onGlyphMaskNeedsCurrentColor() const {
    fGlyphMasksMayNeedCurrentColorOnce([this]{
        static constexpr SkFourByteTag COLRTag = SkSetFourByteTag('C', 'O', 'L', 'R');
        fGlyphMasksMayNeedCurrentColor = this->getTableSize(COLRTag) > 0;
#if defined(FT_CONFIG_OPTION_SVG)
        static constexpr SkFourByteTag SVGTag = SkSetFourByteTag('S', 'V', 'G', ' ');
        fGlyphMasksMayNeedCurrentColor |= this->getTableSize(SVGTag) > 0 ;
#endif  // FT_CONFIG_OPTION_SVG
    });
    return fGlyphMasksMayNeedCurrentColor;
}

int SkTypeface_FreeType::onGetVariationDesignPosition(
    SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{
    AutoFTAccess fta(this);
    return GetVariationDesignPosition(fta, coordinates, coordinateCount);
}

int SkTypeface_FreeType::onGetVariationDesignParameters(
    SkFontParameters::Variation::Axis parameters[], int parameterCount) const
{
        MLOG("SkTypeface_FreeType::onGetVariationDesignParameters");

    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return -1;
    }

    if (!(face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
        return 0;
    }

    FT_MM_Var* variations = nullptr;
    if (FT_Get_MM_Var(face, &variations)) {
        return -1;
    }
    UniqueVoidPtr autoFreeVariations(variations);

    if (!parameters || parameterCount < SkToInt(variations->num_axis)) {
        return variations->num_axis;
    }

    for (FT_UInt i = 0; i < variations->num_axis; ++i) {
        parameters[i].tag = variations->axis[i].tag;
        parameters[i].min = SkFixedToScalar(variations->axis[i].minimum);
        parameters[i].def = SkFixedToScalar(variations->axis[i].def);
        parameters[i].max = SkFixedToScalar(variations->axis[i].maximum);
        FT_UInt flags = 0;
        bool hidden = !FT_Get_Var_Axis_Flags(variations, i, &flags) &&
                      (flags & FT_VAR_AXIS_FLAG_HIDDEN);
        parameters[i].setHidden(hidden);
    }

    return variations->num_axis;
}

int SkTypeface_FreeType::onGetTableTags(SkFontTableTag tags[]) const {
    MLOG("In SkTypeface_FreeType::onGetTableTags");

    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return 0;
    }

    FT_ULong tableCount = 0;
    FT_Error error;

    // When 'tag' is nullptr, returns number of tables in 'length'.
    error = FT_Sfnt_Table_Info(face, 0, nullptr, &tableCount);
    if (error) {
        return 0;
    }

    if (tags) {
        for (FT_ULong tableIndex = 0; tableIndex < tableCount; ++tableIndex) {
            FT_ULong tableTag;
            FT_ULong tablelength;
            error = FT_Sfnt_Table_Info(face, tableIndex, &tableTag, &tablelength);
            if (error) {
                return 0;
            }
            tags[tableIndex] = static_cast<SkFontTableTag>(tableTag);
        }
    }
    return tableCount;
}

size_t SkTypeface_FreeType::onGetTableData(SkFontTableTag tag, size_t offset,
                                           size_t length, void* data) const
{
        MLOG("In SkTypeface_FreeType::onGetTableData");

    AutoFTAccess fta(this);
    FT_Face face = fta.face();
    if (!face) {
        return 0;
    }

    FT_ULong tableLength = 0;
    FT_Error error;

    // When 'length' is 0 it is overwritten with the full table length; 'offset' is ignored.
    error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &tableLength);
    if (error) {
        return 0;
    }

    if (offset > tableLength) {
        return 0;
    }
    FT_ULong size = std::min((FT_ULong)length, tableLength - (FT_ULong)offset);
    if (data) {
        error = FT_Load_Sfnt_Table(face, tag, offset, reinterpret_cast<FT_Byte*>(data), &size);
        if (error) {
            return 0;
        }
    }

    return size;
}

sk_sp<SkData> SkTypeface_FreeType::onCopyTableData(SkFontTableTag tag) const {
    AutoFTAccess fta(this);
    MLOG("In SkTypeface_FreeType::onCopyTableData");

    FT_Face face = fta.face();
    if (!face) {
        return nullptr;
    }

    FT_ULong tableLength = 0;
    FT_Error error;

    // When 'length' is 0 it is overwritten with the full table length; 'offset' is ignored.
    error = FT_Load_Sfnt_Table(face, tag, 0, nullptr, &tableLength);
    if (error) {
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(tableLength);
    if (data) {
        error = FT_Load_Sfnt_Table(face, tag, 0,
                                   reinterpret_cast<FT_Byte*>(data->writable_data()), &tableLength);
        if (error) {
            data.reset();
        }
    }
    return data;
}

SkTypeface_FreeType::FaceRec* SkTypeface_FreeType::getFaceRec() const {
    f_t_mutex().assertHeld();
    fFTFaceOnce([this]{ fFaceRec = SkTypeface_FreeType::FaceRec::Make(this); });
    return fFaceRec.get();
}

std::unique_ptr<SkFontData> SkTypeface_FreeType::makeFontData() const {
    return this->onMakeFontData();
}

void SkTypeface_FreeType::FontDataPaletteToDescriptorPalette(const SkFontData& fontData,
                                                             SkFontDescriptor* desc) {
    desc->setPaletteIndex(fontData.getPaletteIndex());
    int paletteOverrideCount = fontData.getPaletteOverrideCount();
    auto overrides = desc->setPaletteEntryOverrides(paletteOverrideCount);
    for (int i = 0; i < paletteOverrideCount; ++i) {
        overrides[i] = fontData.getPaletteOverrides()[i];
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int SkTypeface_FreeType::Scanner::ninst = 10;

 SkMutex globalMutex;

base::Lock& GetWasmboxLock() {
    static base::Lock g_lock;
    return g_lock;
}

SkTypeface_FreeType::Scanner::Scanner() : fLibrary(nullptr) {
    base::AutoLock lock(GetWasmboxLock());
        MLOG("In SkTypeface_FreeType::Scanner::Scanner");

    if (FT_New_Library(&gFTMemory, &fLibrary)) {
        return;
    }
    FT_Add_Default_Modules(fLibrary);
    FT_Set_Default_Properties(fLibrary);

    printf("Scanner ninst = %d\n", ninst++);
    fflush(stdout);
    ft2_pp = 0;
// SkAutoMutexExclusive libraryLock(globalMutex);
ONOFF(
    if (wasmbox_inited == 3) {
                    pid_t pid = getpid();
            //pid_t tid = gettid();
                    std::stringstream ss; ss << std::this_thread::get_id();

            std::string tid = ss.str();
        printf("WASMBOX inited() in Scanner::ctor %d %s %p %d, %p\n", pid, tid.c_str(), this, wasmbox_inited, &wasmbox_inited);
        fflush(stdout);
            // if (pid != 1) {
            wasm_rt_init();
            // WASM_RT_ADD_PREFIX(init)();
                // printf(" pid not 1: wasmbox_inited\n"); fflush(stdout);
            // }  else {
            //     printf(" pid is 1: wasmbox_inited failed\n"); fflush(stdout);
            // }
        wasmbox_inited = 1;
        printf("wasmbox_inited = %d %p\n", wasmbox_inited, &wasmbox_inited);
        fflush(stdout);
    } else {
    pid_t pid = getpid();
            //pid_t tid = gettid();
                    std::stringstream ss; ss << std::this_thread::get_id();
            std::string tid = ss.str();
        printf("WASMBOX already inited() in Scanner::ctor %d %s\n", pid, tid.c_str());
        fflush(stdout);
    }
    ft2_pp = Z_mallocZ_ii(4);
        // u32 mem_p = Z_FT_New_MemoryZ_iv(); // pointer to a memory address
        // printf("ft2_box = %u, Mem P = %udd\n", ft2_pp, mem_p);
        // printf("ft2_p = %u\n", read_u32(ft2_pp));
        // if (Z_FT_New_LibraryZ_iii(mem_p, ft2_pp)) {
        //     printf("ERROR: Failed to init FreeType (Sandboxed)\n");
        //     return;
        // }
        // printf("WASMBOX: NewLib is created!\n");
        // u32 ft2_p = read_u32(ft2_pp);
        // printf("ft2_p = %u\n", read_u32(ft2_pp));

        // Z_FT_Add_Default_ModulesZ_vi(ft2_p);
    u32 ft2_p = read_u32(ft2_pp);
    if (Z_FT_Init_FreeTypeZ_ii(ft2_pp)) {
        printf("ERROR: Failed to init FreeType (Sandboxed)\n");
        return;
    }
)
    MLOG(" END of Scanner::Scanner()");
    LOG(ERROR) << base::debug::StackTrace();
    printf(" END2 of Scanner::Scanner() %p\n", this);
    fflush(stdout);
}
SkTypeface_FreeType::Scanner::~Scanner() {
        printf("In SkTypeface_FreeType::Scanner::~Scanner %p\n", this);
        fflush(stdout);
    if (fLibrary) {
        MLOG(" dtor fLibrary");
        FT_Done_Library(fLibrary);
    }
ONOFF(

    if (read_u32(ft2_pp)) {
        Z_FT_Done_LibraryZ_ii(read_u32(ft2_pp));
    }
)
}

FT_Face SkTypeface_FreeType::Scanner::openFace(SkStreamAsset* stream, int ttcIndex,
                                               FT_Stream ftStream) const
{
    MLOG("In Scanner::openFace()!!");

    if (fLibrary == nullptr || stream == nullptr) {
        return nullptr;
    }

    FT_Open_Args args;
    memset(&args, 0, sizeof(args));

    const void* memoryBase = stream->getMemoryBase();

ONOFF(
    u32 mem2_p = Z_mallocZ_ii(stream->getLength());
    printf("malloc %zu at %d\n", stream->getLength(), mem2_p); fflush(stdout);
    u32 args2_p = Z_mallocZ_ii(64); // args actually only needs 32 bytes. Just to be safe.
    printf("malloc args2_p %d at %d\n", 64, args2_p); fflush(stdout);
)

ONOFF(
    //ONOFF(
        memcpy(wasm_addr_to_absolute(mem2_p), memoryBase, stream->getLength());
    //)
) 

    if (memoryBase) {
        printf("IS MemoryBase!\n");
        args.flags = FT_OPEN_MEMORY;
        args.memory_base = (const FT_Byte*)memoryBase;
        args.memory_size = stream->getLength();
ONOFF(
        write_u32(args2_p, FT_OPEN_MEMORY);
        write_u32(args2_p+4, mem2_p);
        write_u32(args2_p+8, stream->getLength());
)
    } else {
        printf("IS MemoryStream!\n");
        memset(ftStream, 0, sizeof(*ftStream));
        ftStream->size = stream->getLength();
        ftStream->descriptor.pointer = stream;
        ftStream->read  = sk_ft_stream_io;
        ftStream->close = sk_ft_stream_close;

        args.flags = FT_OPEN_STREAM;
        args.stream = ftStream;
    }

    FT_Face face;
    printf("In %s\n", __PRETTY_FUNCTION__);

    

    if (FT_Open_Face(fLibrary, &args, ttcIndex, &face)) {
        return nullptr;
    }
ONOFF(
    u32 face2_pp = Z_mallocZ_ii(4);
    printf("Face fam:%s, sty:%s\n", face->family_name, face->style_name);
    printf("About to OPENFACE!\n");
    printf("ft2_pp: %u, ft2_p:%u \n", ft2_pp, read_u32(ft2_pp));
    if (Z_FT_Open_FaceZ_iiiii(read_u32(ft2_pp), args2_p, ttcIndex, face2_pp)) { // this ft2_pp is a member
        printf("FACE: OpenFace in sandbox failed\n");
        return nullptr;
    }
    printf("Face2 fam:%s, sty:%s\n", 
        (char*)wasm_addr_to_absolute(read_u32(read_u32(face2_pp)+5*4)),
        (char*)wasm_addr_to_absolute(read_u32(read_u32(face2_pp)+6*4)));
)
    return face;
}

bool SkTypeface_FreeType::Scanner::recognizedFont(SkStreamAsset* stream, int* numFaces) const {
    SkAutoMutexExclusive libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    MLOG("RECOGNIZEDFONT!");
    SkUniqueFTFace face(this->openFace(stream, -1, &streamRec));
    if (!face) {
        return false;
    }

    *numFaces = face->num_faces;
    return true;
}

bool SkTypeface_FreeType::Scanner::scanFont(
    SkStreamAsset* stream, int ttcIndex,
    SkString* name, SkFontStyle* style, bool* isFixedPitch, AxisDefinitions* axes) const
{
    MLOG("SCANFONT1");
    SkAutoMutexExclusive libraryLock(fLibraryMutex);

    FT_StreamRec streamRec;
    SkUniqueFTFace face(this->openFace(stream, ttcIndex, &streamRec));
    if (!face) {
        return false;
    }

    int weight = SkFontStyle::kNormal_Weight;
    int width = SkFontStyle::kNormal_Width;
    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    if (face->style_flags & FT_STYLE_FLAG_BOLD) {
        weight = SkFontStyle::kBold_Weight;
    }
    if (face->style_flags & FT_STYLE_FLAG_ITALIC) {
        slant = SkFontStyle::kItalic_Slant;
    }

    bool hasAxes = face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS;
    TT_OS2* os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face.get(), ft_sfnt_os2));
    bool hasOs2 = os2 && os2->version != 0xffff;

    PS_FontInfoRec psFontInfo;

    if (hasOs2) {
        weight = os2->usWeightClass;
        width = os2->usWidthClass;

        // OS/2::fsSelection bit 9 indicates oblique.
        if (SkToBool(os2->fsSelection & (1u << 9))) {
            slant = SkFontStyle::kOblique_Slant;
        }
    }

    // Let variable axes override properties from the OS/2 table.
    if (hasAxes) {
      AxisDefinitions axisDefinitions;
      if (GetAxes(face.get(), &axisDefinitions)) {
        size_t numAxes = axisDefinitions.size();
        static constexpr SkFourByteTag wghtTag = SkSetFourByteTag('w', 'g', 'h', 't');
        static constexpr SkFourByteTag wdthTag = SkSetFourByteTag('w', 'd', 't', 'h');
        static constexpr SkFourByteTag slntTag = SkSetFourByteTag('s', 'l', 'n', 't');
        std::optional<size_t> wghtIndex;
        std::optional<size_t> wdthIndex;
        std::optional<size_t> slntIndex;
        for(size_t i = 0; i < numAxes; ++i) {
          if (axisDefinitions[i].fTag == wghtTag) {
              // Rough validity check, is there sufficient spread and are ranges
              // within 0-1000.
              int wghtRange = SkFixedToScalar(axisDefinitions[i].fMaximum) -
                              SkFixedToScalar(axisDefinitions[i].fMinimum);
              if (wghtRange > 5 && wghtRange <= 1000 &&
                  SkFixedToScalar(axisDefinitions[i].fMaximum) <= 1000) {
                  wghtIndex = i;
              }
          }
          if (axisDefinitions[i].fTag == wdthTag) {
              // Rough validity check, is there a spread and are ranges within
              // 0-500.
              int widthRange = SkFixedToScalar(axisDefinitions[i].fMaximum) -
                               SkFixedToScalar(axisDefinitions[i].fMinimum);
              if (widthRange > 0 && widthRange <= 500 &&
                  SkFixedToScalar(axisDefinitions[i].fMaximum) <= 500)
                  wdthIndex = i;
          }
          if (axisDefinitions[i].fTag == slntTag)
            slntIndex = i;
        }
        AutoSTMalloc<4, FT_Fixed> coords(numAxes);
        if ((wghtIndex || wdthIndex || slntIndex) &&
            !FT_Get_Var_Design_Coordinates(face.get(), numAxes, coords.get())) {
            if (wghtIndex) {
                SkASSERT(*wghtIndex < numAxes);
                weight = SkScalarRoundToInt(SkFixedToScalar(coords[*wghtIndex]));
            }
            if (wdthIndex) {
                SkASSERT(*wdthIndex < numAxes);
                SkScalar wdthValue = SkFixedToScalar(coords[*wdthIndex]);
                width = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(wdthValue);
            }
            if (slntIndex) {
                SkASSERT(*slntIndex < numAxes);
                // https://docs.microsoft.com/en-us/typography/opentype/spec/dvaraxistag_slnt
                // "Scale interpretation: Values can be interpreted as the angle,
                // in counter-clockwise degrees, of oblique slant from whatever
                // the designer considers to be upright for that font design."
                if (SkFixedToScalar(coords[*slntIndex]) < 0) {
                    slant = SkFontStyle::kOblique_Slant;
                }
            }
        }
      }
    }

    if (!hasOs2 && !hasAxes && 0 == FT_Get_PS_Font_Info(face.get(), &psFontInfo) && psFontInfo.weight) {
        static const struct {
            char const * const name;
            int const weight;
        } commonWeights [] = {
            // There are probably more common names, but these are known to exist.
            { "all", SkFontStyle::kNormal_Weight }, // Multiple Masters usually default to normal.
            { "black", SkFontStyle::kBlack_Weight },
            { "bold", SkFontStyle::kBold_Weight },
            { "book", (SkFontStyle::kNormal_Weight + SkFontStyle::kLight_Weight)/2 },
            { "demi", SkFontStyle::kSemiBold_Weight },
            { "demibold", SkFontStyle::kSemiBold_Weight },
            { "extra", SkFontStyle::kExtraBold_Weight },
            { "extrabold", SkFontStyle::kExtraBold_Weight },
            { "extralight", SkFontStyle::kExtraLight_Weight },
            { "hairline", SkFontStyle::kThin_Weight },
            { "heavy", SkFontStyle::kBlack_Weight },
            { "light", SkFontStyle::kLight_Weight },
            { "medium", SkFontStyle::kMedium_Weight },
            { "normal", SkFontStyle::kNormal_Weight },
            { "plain", SkFontStyle::kNormal_Weight },
            { "regular", SkFontStyle::kNormal_Weight },
            { "roman", SkFontStyle::kNormal_Weight },
            { "semibold", SkFontStyle::kSemiBold_Weight },
            { "standard", SkFontStyle::kNormal_Weight },
            { "thin", SkFontStyle::kThin_Weight },
            { "ultra", SkFontStyle::kExtraBold_Weight },
            { "ultrablack", SkFontStyle::kExtraBlack_Weight },
            { "ultrabold", SkFontStyle::kExtraBold_Weight },
            { "ultraheavy", SkFontStyle::kExtraBlack_Weight },
            { "ultralight", SkFontStyle::kExtraLight_Weight },
        };
        int const index = SkStrLCSearch(&commonWeights[0].name, std::size(commonWeights),
                                        psFontInfo.weight, sizeof(commonWeights[0]));
        if (index >= 0) {
            weight = commonWeights[index].weight;
        } else {
            LOG_INFO("Do not know weight for: %s (%s) \n", face->family_name, psFontInfo.weight);
        }
    }

    if (name != nullptr) {
        name->set(face->family_name);
    }
    if (style != nullptr) {
        *style = SkFontStyle(weight, width, slant);
    }
    if (isFixedPitch != nullptr) {
        *isFixedPitch = FT_IS_FIXED_WIDTH(face);
    }

    if (axes != nullptr && !GetAxes(face.get(), axes)) {
        return false;
    }
    return true;
}

bool SkTypeface_FreeType::Scanner::GetAxes(FT_Face face, AxisDefinitions* axes) {
    SkASSERT(face && axes);
    MLOG("In Scanner::GetAxes");

    if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
        FT_MM_Var* variations = nullptr;
        FT_Error err = FT_Get_MM_Var(face, &variations);
        if (err) {
            LOG_INFO("INFO: font %s claims to have variations, but none found.\n",
                     face->family_name);
            return false;
        }
        UniqueVoidPtr autoFreeVariations(variations);

        axes->reset(variations->num_axis);
        for (FT_UInt i = 0; i < variations->num_axis; ++i) {
            const FT_Var_Axis& ftAxis = variations->axis[i];
            (*axes)[i].fTag = ftAxis.tag;
            (*axes)[i].fMinimum = ftAxis.minimum;
            (*axes)[i].fDefault = ftAxis.def;
            (*axes)[i].fMaximum = ftAxis.maximum;
        }
    }
    return true;
}

/*static*/ void SkTypeface_FreeType::Scanner::computeAxisValues(
    AxisDefinitions axisDefinitions,
    const SkFontArguments::VariationPosition position,
    SkFixed* axisValues,
    const SkString& name,
    const SkFontArguments::VariationPosition::Coordinate* current)
{
    for (int i = 0; i < axisDefinitions.size(); ++i) {
        const Scanner::AxisDefinition& axisDefinition = axisDefinitions[i];
        const SkScalar axisMin = SkFixedToScalar(axisDefinition.fMinimum);
        const SkScalar axisMax = SkFixedToScalar(axisDefinition.fMaximum);

        // Start with the default value.
        axisValues[i] = axisDefinition.fDefault;

        // Then the current value.
        if (current) {
            for (int j = 0; j < axisDefinitions.size(); ++j) {
                const auto& coordinate = current[j];
                if (axisDefinition.fTag == coordinate.axis) {
                    const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                    axisValues[i] = SkScalarToFixed(axisValue);
                    break;
                }
            }
        }

        // Then the requested value.
        // The position may be over specified. If there are multiple values for a given axis,
        // use the last one since that's what css-fonts-4 requires.
        for (int j = position.coordinateCount; j --> 0;) {
            const auto& coordinate = position.coordinates[j];
            if (axisDefinition.fTag == coordinate.axis) {
                const SkScalar axisValue = SkTPin(coordinate.value, axisMin, axisMax);
                if (coordinate.value != axisValue) {
                    LOG_INFO("Requested font axis value out of range: "
                             "%s '%c%c%c%c' %f; pinned to %f.\n",
                             name.c_str(),
                             (axisDefinition.fTag >> 24) & 0xFF,
                             (axisDefinition.fTag >> 16) & 0xFF,
                             (axisDefinition.fTag >>  8) & 0xFF,
                             (axisDefinition.fTag      ) & 0xFF,
                             SkScalarToDouble(coordinate.value),
                             SkScalarToDouble(axisValue));
                }
                axisValues[i] = SkScalarToFixed(axisValue);
                break;
            }
        }
        // TODO: warn on defaulted axis?
    }

    SkDEBUGCODE(
        // Check for axis specified, but not matched in font.
        for (int i = 0; i < position.coordinateCount; ++i) {
            SkFourByteTag skTag = position.coordinates[i].axis;
            bool found = false;
            for (int j = 0; j < axisDefinitions.size(); ++j) {
                if (skTag == axisDefinitions[j].fTag) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                LOG_INFO("Requested font axis not found: %s '%c%c%c%c'\n",
                         name.c_str(),
                         (skTag >> 24) & 0xFF,
                         (skTag >> 16) & 0xFF,
                         (skTag >>  8) & 0xFF,
                         (skTag)       & 0xFF);
            }
        }
    )
}


SkTypeface_FreeTypeStream::SkTypeface_FreeTypeStream(std::unique_ptr<SkFontData> fontData,
                                                     const SkString familyName,
                                                     const SkFontStyle& style, bool isFixedPitch)
    : SkTypeface_FreeType(style, isFixedPitch)
    , fFamilyName(std::move(familyName))
    , fData(std::move(fontData))
{ }

SkTypeface_FreeTypeStream::~SkTypeface_FreeTypeStream() {}

void SkTypeface_FreeTypeStream::onGetFamilyName(SkString* familyName) const {
    *familyName = fFamilyName;
}

std::unique_ptr<SkStreamAsset> SkTypeface_FreeTypeStream::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fData->getIndex();
    return fData->getStream()->duplicate();
}

std::unique_ptr<SkFontData> SkTypeface_FreeTypeStream::onMakeFontData() const {
    return std::make_unique<SkFontData>(*fData);
}

sk_sp<SkTypeface> SkTypeface_FreeTypeStream::onMakeClone(const SkFontArguments& args) const {
    std::unique_ptr<SkFontData> data = this->cloneFontData(args);
    if (!data) {
        return nullptr;
    }

    SkString familyName;
    this->getFamilyName(&familyName);

    return sk_make_sp<SkTypeface_FreeTypeStream>(
        std::move(data), familyName, this->fontStyle(), this->isFixedPitch());
}

void SkTypeface_FreeTypeStream::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    desc->setFamilyName(fFamilyName.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(SkTypeface_FreeType::FactoryId);
    SkTypeface_FreeType::FontDataPaletteToDescriptorPalette(*fData, desc);
    *serialize = true;
}

sk_sp<SkTypeface> SkTypeface_FreeType::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                      const SkFontArguments& args) {
    using Scanner = SkTypeface_FreeType::Scanner;
    static Scanner scanner;
    bool isFixedPitch;
    SkFontStyle style;
    SkString name;
    Scanner::AxisDefinitions axisDefinitions;
    if (!scanner.scanFont(stream.get(), args.getCollectionIndex(),
                          &name, &style, &isFixedPitch, &axisDefinitions)) {
        return nullptr;
    }

    const SkFontArguments::VariationPosition position = args.getVariationDesignPosition();
    AutoSTMalloc<4, SkFixed> axisValues(axisDefinitions.size());
    Scanner::computeAxisValues(axisDefinitions, position, axisValues, name);

    auto data = std::make_unique<SkFontData>(
        std::move(stream), args.getCollectionIndex(), args.getPalette().index,
        axisValues.get(), axisDefinitions.size(),
        args.getPalette().overrides, args.getPalette().overrideCount);
    return sk_make_sp<SkTypeface_FreeTypeStream>(std::move(data), name, style, isFixedPitch);
}
