EXTRA_DIST = Doxyfile Dumas-1.set example.cc

lib_LTLIBRARIES=libdjinni-2.4.la
libdjinni_2_4_la_SOURCES=twister.cc routes.cc
libdjinni_2_4_la_CPPFLAGS=$(BOOST_CPPFLAGS)
libdjinni_2_4_la_LDFLAGS=$(BOOST_LDFLAGS)
pkginclude_HEADERS=annealers.h penalties.h routes.h utils.h djinni.h


dist-hook:
	for file in $(BUILT_SOURCES) ; do \
		rm -f $(distdir)/$$file ; \
	done

docs: Doxyfile
	doxygen

