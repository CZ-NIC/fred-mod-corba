ACLOCAL_AMFLAGS=-I ${top_srcdir}/m4/

noinst_LTLIBRARIES = libmodcorba.la
noinst_DATA = mod_corba.la

INCLUDES = ${APACHE_CFLAGS} ${APACHE_CPPFLAGS} ${ORBIT_CFLAGS} ${CPPFLAGS} 
AM_LDFLAGS = ${ORBIT_LIBS} ${LIBS} ${LDFLAGS}

libmodcorba_la_SOURCES = mod_corba.c

install-exec-local:
	${APXS} -i -n 'mod_corba' mod_corba.la

mod_corba.la: libmodcorba.la
	${APXS} -c -o $@ $< ${APACHE_CFLAGS} ${ORBIT_CFLAGS} ${AM_LDFLAGS}
