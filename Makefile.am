ACLOCAL_AMFLAGS=-I m4
include $(top_srcdir)/am_doxygen.am

noinst_LTLIBRARIES = libmodcorba.la
noinst_DATA = mod_corba.la
dist_pkgdata_DATA = $(top_builddir)/conf/01-fred-mod-corba-apache.conf
CLEANFILES = $(top_builddir)/mod_corba.la $(top_builddir)/conf/01-fred-mod-corba-apache.conf
EXTRA_DIST = unconfig.h fred-mod-corba.spec doxy.conf

AM_CPPFLAGS = ${APACHE_CFLAGS} ${APACHE_CPPFLAGS} ${ORBIT_CFLAGS} ${CPPFLAGS} 
AM_LDFLAGS = ${ORBIT_LIBS} ${LIBS} ${LDFLAGS}

libmodcorba_la_SOURCES = mod_corba.c

if DX_COND_html
doc_DATA = @DX_DOCDIR@/html/*
$(doc_DATA): doxygen-doc
MOSTLYCLEANFILES = ${DX_CLEANFILES}
endif

install-exec-local:
	mkdir -p ${DESTDIR}${APACHE_LIBEXECDIR}; \
	${APXS} -S LIBEXECDIR=${DESTDIR}${APACHE_LIBEXECDIR} -i -n 'mod_corba' mod_corba.la

uninstall-local:
	rm -rf ${DESTDIR}${APACHE_LIBEXECDIR}/mod_corba.so

mod_corba.la: libmodcorba.la
	${APXS} -c -o $@ $< ${APACHE_CFLAGS} ${ORBIT_CFLAGS} ${AM_LDFLAGS}
