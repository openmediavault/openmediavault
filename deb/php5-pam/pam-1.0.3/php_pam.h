/*
  +----------------------------------------------------------------------+
  | PHP version 4.0                                                      |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Mikael Johansson <mikael AT synd DOT info>                  |
  |          Chad Cunningham                                             |
  +----------------------------------------------------------------------+
*/

/* $Id: php_pam.h 291417 2009-11-29 10:49:27Z mikl $ */

#ifndef PHP_PAM_H
#define PHP_PAM_H

extern zend_module_entry pam_module_entry;
#define phpext_pam_ptr &pam_module_entry

#define PHP_PAM_VERSION "1.0.3"

#ifdef PHP_WIN32
#define PHP_PAM_API __declspec(dllexport)
#else
#define PHP_PAM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(pam);
PHP_MSHUTDOWN_FUNCTION(pam);
PHP_MINFO_FUNCTION(pam);

PHP_FUNCTION(pam_auth);
PHP_FUNCTION(pam_chpass);

ZEND_BEGIN_MODULE_GLOBALS(pam)
	char *servicename;
ZEND_END_MODULE_GLOBALS(pam)

typedef struct {
	char *name, *pw;
} pam_auth_t;

typedef struct {
	char *name, *oldpw, *newpw;
	int count;
} pam_chpass_t;

#ifdef ZTS
#define PAM_G(v) TSRMG(pam_globals_id, zend_pam_globals *, v)
#else
#define PAM_G(v) (pam_globals.v)
#endif

#endif	/* PHP_PAM_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
