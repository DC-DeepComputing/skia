package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "skcms",
    srcs = [
        "skcms/skcms.cc",
        "skcms/skcms_internal.h",
        "skcms/src/Transform_inl.h",
    ],
    hdrs = [
        "//include:third-party-skcms-hdrs",
    ],
    # needed because skcms.cc just does include skcms.h
    strip_include_prefix = "//include/third_party/skcms/",
)

cc_library(
    name = "libjpeg-turbo",
    srcs = [
        "externals/libjpeg-turbo/jaricom.c",
        "externals/libjpeg-turbo/jcapimin.c",
        "externals/libjpeg-turbo/jcapistd.c",
        "externals/libjpeg-turbo/jcarith.c",
        "externals/libjpeg-turbo/jccoefct.c",
        "externals/libjpeg-turbo/jccolor.c",
        "externals/libjpeg-turbo/jcdctmgr.c",
        "externals/libjpeg-turbo/jchuff.c",
        "externals/libjpeg-turbo/jcinit.c",
        "externals/libjpeg-turbo/jcmainct.c",
        "externals/libjpeg-turbo/jcmarker.c",
        "externals/libjpeg-turbo/jcmaster.c",
        "externals/libjpeg-turbo/jcomapi.c",
        "externals/libjpeg-turbo/jcparam.c",
        "externals/libjpeg-turbo/jcphuff.c",
        "externals/libjpeg-turbo/jcprepct.c",
        "externals/libjpeg-turbo/jcsample.c",
        "externals/libjpeg-turbo/jdapimin.c",
        "externals/libjpeg-turbo/jdapistd.c",
        "externals/libjpeg-turbo/jdarith.c",
        "externals/libjpeg-turbo/jdcoefct.c",
        "externals/libjpeg-turbo/jdcolor.c",
        "externals/libjpeg-turbo/jddctmgr.c",
        "externals/libjpeg-turbo/jdhuff.c",
        "externals/libjpeg-turbo/jdinput.c",
        "externals/libjpeg-turbo/jdmainct.c",
        "externals/libjpeg-turbo/jdmarker.c",
        "externals/libjpeg-turbo/jdmaster.c",
        "externals/libjpeg-turbo/jdmerge.c",
        "externals/libjpeg-turbo/jdphuff.c",
        "externals/libjpeg-turbo/jdpostct.c",
        "externals/libjpeg-turbo/jdsample.c",
        "externals/libjpeg-turbo/jerror.c",
        "externals/libjpeg-turbo/jfdctflt.c",
        "externals/libjpeg-turbo/jfdctfst.c",
        "externals/libjpeg-turbo/jfdctint.c",
        "externals/libjpeg-turbo/jidctflt.c",
        "externals/libjpeg-turbo/jidctfst.c",
        "externals/libjpeg-turbo/jidctint.c",
        "externals/libjpeg-turbo/jidctred.c",
        "externals/libjpeg-turbo/jmemmgr.c",
        "externals/libjpeg-turbo/jmemnobs.c",
        "externals/libjpeg-turbo/jpeg_nbits_table.c",
        "externals/libjpeg-turbo/jquant1.c",
        "externals/libjpeg-turbo/jquant2.c",
        "externals/libjpeg-turbo/jutils.c",
    ],
    # TODO(kjlubick) neon support
    hdrs = glob([
        "externals/libjpeg-turbo/*.h",
    ]),
    local_defines = [
        "TURBO_FOR_WINDOWS",
        "USE_CLZ_INTRINSIC",
        "C_ARITH_CODING_SUPPORTED=1",
        "D_ARITH_CODING_SUPPORTED=1",
    ],
    strip_include_prefix = "externals/libjpeg-turbo/",
    textual_hdrs = [
        "externals/libjpeg-turbo/jccolext.c",
        "externals/libjpeg-turbo/jdmrgext.c",
        "externals/libjpeg-turbo/jdcolext.c",
        "externals/libjpeg-turbo/jdcol565.c",
        "externals/libjpeg-turbo/jdmrg565.c",
        "externals/libjpeg-turbo/jstdhuff.c",
    ],
)

filegroup(
    name = "libjpeg-turbo-hdrs",
    srcs = glob([
        "externals/libjpeg-turbo/*.h",
    ]),
)
