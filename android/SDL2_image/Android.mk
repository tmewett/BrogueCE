SDL_IMAGE_LOCAL_PATH := $(call my-dir)


# Enable this if you want PNG and JPG support with minimal dependencies
USE_STBIMAGE ?= true

# The additional formats below require downloading third party dependencies,
# using the external/download.sh script.

# Enable this if you want to support loading AVIF images
# The library path should be a relative path to this directory.
SUPPORT_AVIF ?= false
AVIF_LIBRARY_PATH := external/libavif
DAV1D_LIBRARY_PATH := external/dav1d

# Enable this if you want to support loading JPEG images using libjpeg
# The library path should be a relative path to this directory.
SUPPORT_JPG ?= false
SUPPORT_SAVE_JPG ?= true
JPG_LIBRARY_PATH := external/jpeg

# Enable this if you want to support loading JPEG-XL images
# The library path should be a relative path to this directory.
SUPPORT_JXL ?= false
JXL_LIBRARY_PATH := external/libjxl

# Enable this if you want to support loading PNG images using libpng
# The library path should be a relative path to this directory.
SUPPORT_PNG ?= false
SUPPORT_SAVE_PNG ?= true
PNG_LIBRARY_PATH := external/libpng

# Enable this if you want to support loading WebP images
# The library path should be a relative path to this directory.
SUPPORT_WEBP ?= false
WEBP_LIBRARY_PATH := external/libwebp


# Build the library
ifeq ($(SUPPORT_AVIF),true)
    include $(SDL_IMAGE_LOCAL_PATH)/$(AVIF_LIBRARY_PATH)/Android.mk
    include $(SDL_IMAGE_LOCAL_PATH)/$(DAV1D_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_JPG),true)
    include $(SDL_IMAGE_LOCAL_PATH)/$(JPG_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_JXL),true)
    include $(SDL_IMAGE_LOCAL_PATH)/$(JXL_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_PNG),true)
    include $(SDL_IMAGE_LOCAL_PATH)/$(PNG_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_WEBP),true)
    include $(SDL_IMAGE_LOCAL_PATH)/$(WEBP_LIBRARY_PATH)/Android.mk
endif


# Restore local path
LOCAL_PATH := $(SDL_IMAGE_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_image

LOCAL_SRC_FILES :=  \
    src/IMG.c           \
    src/IMG_avif.c      \
    src/IMG_bmp.c       \
    src/IMG_gif.c       \
    src/IMG_jpg.c       \
    src/IMG_jxl.c       \
    src/IMG_lbm.c       \
    src/IMG_pcx.c       \
    src/IMG_png.c       \
    src/IMG_pnm.c       \
    src/IMG_qoi.c       \
    src/IMG_stb.c       \
    src/IMG_svg.c       \
    src/IMG_tga.c       \
    src/IMG_tif.c       \
    src/IMG_webp.c      \
    src/IMG_WIC.c       \
    src/IMG_xcf.c       \
    src/IMG_xpm.c.arm   \
    src/IMG_xv.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_CFLAGS := -DLOAD_BMP -DLOAD_GIF -DLOAD_LBM -DLOAD_PCX -DLOAD_PNM \
                -DLOAD_SVG -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM -DLOAD_XV  \
                -DLOAD_QOI
LOCAL_LDLIBS :=
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := SDL2

ifeq ($(USE_STBIMAGE),true)
    LOCAL_CFLAGS += -DLOAD_JPG -DLOAD_PNG -DUSE_STBIMAGE
endif

ifeq ($(SUPPORT_AVIF),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(AVIF_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DLOAD_AVIF
    LOCAL_STATIC_LIBRARIES += avif
    LOCAL_WHOLE_STATIC_LIBRARIES += dav1d dav1d-8bit dav1d-16bit
endif

ifeq ($(SUPPORT_JPG),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(JPG_LIBRARY_PATH)              \
                        $(LOCAL_PATH)/$(JPG_LIBRARY_PATH)/android
    LOCAL_CFLAGS += -DLOAD_JPG
    LOCAL_STATIC_LIBRARIES += jpeg
ifeq ($(SUPPORT_SAVE_JPG),true)
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_JPG=1
else
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_JPG=0
endif
endif

ifeq ($(SUPPORT_JXL),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(JXL_LIBRARY_PATH)/lib/include \
                        $(LOCAL_PATH)/$(JXL_LIBRARY_PATH)/android
    LOCAL_CFLAGS += -DLOAD_JXL
    LOCAL_STATIC_LIBRARIES += jxl
endif

ifeq ($(SUPPORT_PNG),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(PNG_LIBRARY_PATH)             \
                        $(LOCAL_PATH)/$(PNG_LIBRARY_PATH)/android
    LOCAL_CFLAGS += -DLOAD_PNG
    LOCAL_STATIC_LIBRARIES += png
    LOCAL_LDLIBS += -lz
ifeq ($(SUPPORT_SAVE_PNG),true)
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_PNG=1
else
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_PNG=0
endif
endif

ifeq ($(SUPPORT_WEBP),true)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(WEBP_LIBRARY_PATH)/src
    LOCAL_CFLAGS += -DLOAD_WEBP
    LOCAL_STATIC_LIBRARIES += webpdemux
    LOCAL_STATIC_LIBRARIES += webp
endif

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include

include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL2_image static library
#
###########################

LOCAL_MODULE := SDL2_image_static

LOCAL_MODULE_FILENAME := libSDL2_image

LOCAL_LDLIBS :=
LOCAL_EXPORT_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)
