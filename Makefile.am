ACLOCAL_AMFLAGS=-I ${top_srcdir}/m4/

noinst_LTLIBRARIES = libmodcorba.la
noinst_DATA = mod_corba.la
CLEANFILES = mod_corba.la
EXTRA_DIST = unconfig.h


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
