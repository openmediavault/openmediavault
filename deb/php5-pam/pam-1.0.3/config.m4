dnl $Id: config.m4 223567 2006-11-19 10:22:36Z mikl $
dnl config.m4 for extension pam
dnl vim:se ts=2 sw=2 et:

PHP_ARG_WITH(pam, for PAM support,
[  --with-pam[=DIR]       Include PAM support. DIR is the libpam install prefix])

if test -z "$PHP_DEBUG"; then
  AC_ARG_ENABLE(debug,
  [  --enable-debug          compile with debugging symbols],[
    PHP_DEBUG=$enableval
  ],[
    PHP_DEBUG=no
  ]) 
fi

if test "$PHP_PAM" != "no"; then
  AC_DEFINE(HAVE_PAM, 1, [ ])

  for i in $PHP_PAM /usr/local /usr /; do
    if test -r $i/include/security/pam_appl.h; then
      PAM_DIR=$i
    fi
  done

  if test -z "$PAM_DIR"; then
    AC_MSG_ERROR([install the the PAM library or use --with-pam=DIR to specify the location.])
  fi

  PHP_CHECK_LIBRARY(pam, pam_start,,[
    AC_MSG_ERROR([failed to find and link against libpam])
  ],-L$PAM_DIR/$PHP_LIBDIR)
  
  PHP_ADD_LIBRARY_WITH_PATH(pam, $PAM_DIR/$PHP_LIBDIR, PAM_SHARED_LIBADD)
  PHP_ADD_INCLUDE($PAM_DIR/include)
  PHP_SUBST(PAM_SHARED_LIBADD)
  
  if test "$PHP_DEBUG" = "yes"; then
    CFLAGS="$CFLAGS -Wall"
  fi

  PHP_NEW_EXTENSION(pam, pam.c, $ext_shared)
fi
