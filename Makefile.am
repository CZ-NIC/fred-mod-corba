ACLOCAL_AMFLAGS=-I m4

noinst_LTLIBRARIES = libmodcorba.la
noinst_DATA = mod_corba.la
dist_pkgdata_DATA = ${srcdir}/conf/01-fred-mod-corba-apache.conf
CLEANFILES = mod_corba.la
EXTRA_DIST = unconfig.h fred-mod-corba.spec

AM_CPPFLAGS = ${APACHE_CFLAGS} ${APACHE_CPPFLAGS} ${ORBIT_CFLAGS} ${CPPFLAGS} 
AM_LDFLAGS = ${ORBIT_LIBS} ${LIBS} ${LDFLAGS}

libmodcorba_la_SOURCES = mod_corba.c

install-exec-local:
	mkdir -p ${DESTDIR}${APACHE_LIBEXECDIR}; \
	${APXS} -S LIBEXECDIR=${DESTDIR}${APACHE_LIBEXECDIR} -i -n 'mod_corba' mod_corba.la

uninstall-local:
	rm -rf ${DESTDIR}${APACHE_LIBEXECDIR}/mod_corba.so

mod_corba.la: libmodcorba.la
	${APXS} -c -o $@ $< ${APACHE_CFLAGS} ${ORBIT_CFLAGS} ${AM_LDFLAGS}
