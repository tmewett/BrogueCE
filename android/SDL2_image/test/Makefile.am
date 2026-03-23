testmetadir = $(datadir)/installed-tests/$(PACKAGE_TARNAME)
testexecdir = $(libexecdir)/installed-tests/$(PACKAGE_TARNAME)

test_programs = \
	testimage \
	$(NULL)

testimage_CPPFLAGS = -I$(top_srcdir)/include
testimage_SOURCES = main.c
testimage_LDADD = \
	../libSDL2_image.la \
	$(SDL_LIBS) \
	-lSDL2_test \
	$(NULL)

AM_TESTS_ENVIRONMENT = \
	SDL_TEST_SRCDIR=$(abs_srcdir) \
	SDL_TEST_BUILDDIR=$(abs_builddir) \
	SDL_VIDEODRIVER=dummy \
	$(NULL)

EXTRA_DIST = \
	CMakeLists.txt Makefile.os2 template.test.in

if INSTALL_TESTS
testexec_PROGRAMS = $(test_programs)
else
noinst_PROGRAMS = $(test_programs)
endif

TESTS = $(test_programs)

if INSTALL_TESTS
dist_testexec_DATA = \
	palette.bmp \
	palette.gif \
	sample.avif \
	sample.bmp \
	sample.cur \
	sample.ico \
	sample.jpg \
	sample.jxl \
	sample.pcx \
	sample.png \
	sample.pnm \
	sample.qoi \
	sample.tga \
	sample.tif \
	sample.webp \
	sample.xcf \
	sample.xpm \
	svg-class.bmp \
	svg-class.svg \
	svg.bmp \
	svg.svg \
	svg64.bmp \
	$(NULL)

all-local: generatetestmeta
generatetestmeta:
	rm -f *.test
	set -e; for exe in $(test_programs); do \
		sed \
			-e 's#@installedtestsdir@#$(testexecdir)#g' \
			-e "s#@exe@#$$exe#g" \
			< $(srcdir)/template.test.in > $$exe.test; \
	done

install-data-hook: installtestmeta
installtestmeta: generatetestmeta
	install -d $(DESTDIR)$(testmetadir)
	install -m644 *.test $(DESTDIR)$(testmetadir)

clean-local:
	rm -f *.test
	rm -f save.jpg save.bmp CompareSurfaces*.bmp
endif
