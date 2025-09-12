dnl $Id$
dnl config.m4 for extension pam

dnl Check PHP version:
AC_MSG_CHECKING(PHP version)
if test ! -z "$phpincludedir"; then
    PHP_VERSION=`grep 'PHP_VERSION ' $phpincludedir/main/php_version.h | sed -e 's/.*"\([[0-9\.]]*\)".*/\1/g' 2>/dev/null`
elif test ! -z "$PHP_CONFIG"; then
    PHP_VERSION=`$PHP_CONFIG --version 2>/dev/null`
fi

if test x"$PHP_VERSION" = "x"; then
    AC_MSG_WARN([none])
else
    PHP_MAJOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/g' 2>/dev/null`
    PHP_MINOR_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/g' 2>/dev/null`
    PHP_RELEASE_VERSION=`echo $PHP_VERSION | sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/g' 2>/dev/null`
    AC_MSG_RESULT([$PHP_VERSION])
fi

if test $PHP_MAJOR_VERSION -lt 7; then
    AC_MSG_ERROR([need at least PHP 7 or newer])
fi

PHP_ARG_WITH(pam, for PAM support,
[  --with-pam[=DIR]       Include PAM support. DIR is the libpam install prefix])

if test "$PHP_PAM" != "no"; then
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/include/security/pam_appl.h"  # you most likely want to change this
  if test -r $PHP_PAM/$SEARCH_FOR; then # path given as parameter
    PAM_DIR=$PHP_PAM
  else # search default path list
    AC_MSG_CHECKING([for PAM files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        PAM_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$PAM_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([install the PAM library or use --with-pam=DIR to specify the location])
  fi

  PHP_ADD_INCLUDE($PAM_DIR/include)

  LIBNAME=pam # you may want to change this
  LIBSYMBOL=pam_start # you most likely want to change this

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PAM_DIR/$PHP_LIBDIR, PAM_SHARED_LIBADD)
    AC_DEFINE(HAVE_PAM,1,[ ])
  ],[
    AC_MSG_ERROR([wrong PAM lib version or lib not found])
  ],[
    -L$PAM_DIR/$PHP_LIBDIR -lm
  ])

  PHP_SUBST(PAM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pam, pam.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
