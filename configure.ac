#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# vim: set ts=4 sw=4:

AC_PREREQ(2.59)
AC_INIT([fred-mod-corba], 1.4.1, [], [fred-mod-corba])
AC_CONFIG_SRCDIR([mod_corba.c])

AH_TOP([#include "unconfig.h" /* why this? See comment in unconfig.h */])

### function to check if APACHE_LIBEXECDIR path contain one or more 
### 'lib' part
### this is bug(feature) on some distributions (e.g. arch) where
### directory for apache modules is /usr/lib/apache. On distros like
### debian, fedora this directory is /usr/lib/apache/1.3 (debian)
### and /usr/lib/httpd/modules. So on debian and fedora code to
### create APACHE_LIBEXECDIR works fine, but on that 'weird' distros
### it produce result like /usr/lib/lib/apache, and that is wrong
AC_DEFUN([AC_CHECK_LIBEXECDIR],
[
 #count number of lib parts in string
 count=`echo $APACHE_LIBEXECDIR | sed 's/\//\n/g' | grep -c 'lib'`
 if test "$count" -gt "1"; then
     #lets trim only last one part (e.g. on arch remain only 'apache')
     APACHE_LIBEXECDIR=${LIBRARYDIR}${APACHE_LEDIR##${APACHE_LEDIR%/*}};
 fi
])

### place where mod_corba.so is installed
AC_DEFUN([AC_CREATE_MODULE_INSTALLDIR],
[
 module_installdir=$APACHE_LIBEXECDIR
 AC_SUBST([MODULE_INSTALLDIR], [$module_installdir])
])


# doxygen
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(fred-mod-corba, doxy.conf, [doc])

# init automake
AM_INIT_AUTOMAKE([-Wall -Werror])

# fix automake 1.12 moan
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_EGREP
AC_PROG_AWK
AC_PROG_INSTALL
AM_PROG_LIBTOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Check for orbit2
AM_PATH_ORBIT2([2.0.0],[],[AC_MSG_ERROR([missing orbit2 header files])])
AX_PATH_ORBIT2_COSNAME()

# check apxs tool
AX_WITH_APXS([no],[${PATH}:/usr/local/bin:/usr/local/sbin:/usr/sbin:/sbin])
if test "$APXS" = "no"; then
    AC_MSG_ERROR([apxs executable does not exist (use --with-apxs)])
fi
# check for ap_release.h
# if present we have a apache 2.0.x and greater (in lower it's not included)
# syntax of \#define AP_SERVER_MAJORVERSION differ in minor versions
APXS_INCDIR="`${APXS} -q INCLUDEDIR`"
AC_CHECK_FILE([$APXS_INCDIR/ap_release.h],
	      [AC_MSG_NOTICE([apache version >=2.0.x found])],
	      [AC_MSG_ERROR([apache version >=2.0.x not found.])])

# check apr-config
AC_ARG_WITH([apr-config], [AC_HELP_STRING([[--with-apr-config]],
                          [location of apr-config executable])],
                          [apr_config="$with_val"],
                          [AC_PATH_PROGS([apr_config], 
                          [apr-config apr-0-config apr-1-config], [no],
                          [$PATH:/usr/sbin/:/usr/local/apache2/bin])])
if test "$APR_CONFIG" = "no"; then
    AC_MSG_ERROR([apr-config executable "$APR_CONFIG" does not exist (use --with-apr-config)])
fi
$apr_config --cppflags &> /dev/null
if test "$?" != "0"; then
    AC_MSG_ERROR([$apr_config is not valid apr-config executable])
fi

# set apache flags
APACHE_CFLAGS="-I`${APXS} -q INCLUDEDIR` -I`${apr_config} --includedir`"
APACHE_CPPFLAGS="`${apr_config} --cppflags`"
AC_SUBST(APACHE_CFLAGS)
AC_SUBST(APACHE_CPPFLAGS)

#set apache module path
#hack: replacing lib-part of dir with ${libdir} to support prefix install
APACHE_LEDIR=`${APXS} -q LIBEXECDIR`
AS_AC_EXPAND([LIBRARYDIR], [$libdir])
APACHE_LIBEXECDIR=${LIBRARYDIR}${APACHE_LEDIR##${APACHE_LEDIR%/*/*}};

AC_CHECK_LIBEXECDIR()

AC_SUBST(APACHE_LIBEXECDIR)

AC_CREATE_MODULE_INSTALLDIR()

#option for CORBA context name
AC_ARG_WITH([contextname], [AC_HELP_STRING([--with-contextname],
            [Name of CORBA context [fred]])],
            [AC_SUBST([contextname], [$withval])],
            [AC_SUBST([contextname], [fred])])

#put context name into config.h file
AC_DEFINE_UNQUOTED([CONTEXT_NAME], [["]$contextname["]], [name of CORBA context])

# check svn revision (if available)
AC_SUBST([SVNREV], [unknown])
AC_MSG_NOTICE([trying to get svn revision])
AC_PATH_PROG([SVN], [svn], [false])
if test "$SVN" != "false"; then
	AC_CHECK_FILE([.svn], [AC_SUBST([SVNREV],
				  [`$SVN info | $EGREP Revision |  $AWK '{ print $2;};'`])])
	AC_MSG_CHECKING([SVN revision])
	AC_MSG_RESULT([$SVNREV])
else
	AC_MSG_CHECKING([SVN revision])
	AC_MSG_RESULT([$SVNREV])
fi
AC_DEFINE_UNQUOTED([SVN_REV], [["]$SVNREV["]], [SVN revision])

# finalization
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		 fred-mod-corba.spec conf/01-fred-mod-corba-apache.conf])
AC_OUTPUT
