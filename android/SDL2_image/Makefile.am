# Makefile.am for the SDL sample image loading library and viewer
ACLOCAL_AMFLAGS = -I acinclude

lib_LTLIBRARIES = libSDL2_image.la

libSDL2_imageincludedir = $(includedir)/SDL2
libSDL2_imageinclude_HEADERS =	\
	include/SDL_image.h

if USE_IMAGEIO
IMAGEIO_SOURCE = src/IMG_ImageIO.m
endif

libSDL2_image_la_SOURCES =	\
	src/IMG.c			\
	src/IMG_avif.c		\
	src/IMG_bmp.c		\
	src/IMG_gif.c		\
	src/IMG_jpg.c		\
	src/IMG_jxl.c		\
	src/IMG_lbm.c		\
	src/IMG_pcx.c		\
	src/IMG_png.c		\
	src/IMG_pnm.c		\
	src/IMG_qoi.c		\
	src/IMG_svg.c		\
	src/IMG_stb.c		\
	src/IMG_tga.c		\
	src/IMG_tif.c		\
	src/IMG_xcf.c		\
	src/IMG_xpm.c		\
	src/IMG_xv.c		\
	src/IMG_webp.c		\
	src/IMG_WIC.c		\
	$(IMAGEIO_SOURCE)	\
	src/miniz.h			\
	src/nanosvg.h		\
	src/nanosvgrast.h		\
	src/qoi.h			\
	src/stb_image.h		\
	src/tiny_jpeg.h

EXTRA_DIST = \
	.gitmodules		\
	Android.mk		\
	CHANGES.txt 		\
	CMakeLists.txt		\
	src/IMG_ImageIO.m		\
	src/IMG_xxx.c		\
	LICENSE.txt 		\
	Makefile.os2		\
	docs/README-emscripten.txt 	\
	README.txt 		\
	SDL2_image.spec		\
	SDL2_imageConfig.cmake.in \
	VisualC			\
	VisualC-WinRT		\
	Xcode			\
	autogen.sh		\
	cmake			\
	external		\
	mingw			\
	src/version.rc


libSDL2_image_la_CPPFLAGS = -I$(srcdir)/include
libSDL2_image_la_LDFLAGS = 	\
	-no-undefined		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) $(LT_EXTRA)
libSDL2_image_la_LIBADD = $(IMG_LIBS)
if USE_VERSION_RC
libSDL2_image_la_DEPENDENCIES = src/version.o
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = SDL2_image.pc

.rc.o:
	$(RC) $< $@

noinst_PROGRAMS = showimage showanim

showimage_SOURCES = examples/showimage.c
showimage_CPPFLAGS = -I$(srcdir)/include
showimage_LDADD = libSDL2_image.la

showanim_SOURCES = examples/showanim.c
showanim_CPPFLAGS = -I$(srcdir)/include
showanim_LDADD = libSDL2_image.la

SUBDIRS = .

if BUILD_TESTS
SUBDIRS += test
endif

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: distcheck

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

dist-hook:
	ls $(distdir)/external | $(FGREP) -v download.sh | $(FGREP) -v Get-GitModules.ps1 | while read dir; do rm -rf "$(distdir)/external/$$dir"; done

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(libdir)/cmake/SDL2_image
	$(INSTALL) -m 644 sdl2_image-config.cmake $(DESTDIR)$(libdir)/cmake/SDL2_image
	$(INSTALL) -m 644 sdl2_image-config-version.cmake $(DESTDIR)$(libdir)/cmake/SDL2_image
	$(INSTALL) -m 644 $(srcdir)/cmake/Findlibjxl.cmake $(DESTDIR)$(libdir)/cmake/SDL2_image
	$(INSTALL) -m 644 $(srcdir)/cmake/Findwebp.cmake $(DESTDIR)$(libdir)/cmake/SDL2_image

uninstall-hook:
	rm $(DESTDIR)$(libdir)/cmake/SDL2_image/sdl2_image-config.cmake
	rm $(DESTDIR)$(libdir)/cmake/SDL2_image/sdl2_image-config-version.cmake
	rm $(DESTDIR)$(libdir)/cmake/SDL2_image/Findlibjxl.cmake
	rm $(DESTDIR)$(libdir)/cmake/SDL2_image/Findwebp.cmake
	rm -r $(DESTDIR)$(libdir)/cmake/SDL2_image
