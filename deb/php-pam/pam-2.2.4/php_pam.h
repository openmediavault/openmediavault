/*
  +----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Amish                                                       |
  | PHP 4.0: Mikael Johansson <mikael AT synd DOT info>                  |
  |          Chad Cunningham                                             |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_PAM_H
#define PHP_PAM_H

extern zend_module_entry pam_module_entry;
#define phpext_pam_ptr &pam_module_entry

#define PHP_PAM_VERSION "2.2.4"

#ifdef PHP_WIN32
#	define PHP_PAM_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PAM_API __attribute__ ((visibility("default")))
#else
#	define PHP_PAM_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(pam)
	char *servicename;
	zend_bool force_servicename;
ZEND_END_MODULE_GLOBALS(pam)

typedef struct {
	char *name, *pw;
} pam_auth_t;

typedef struct {
	char *name, *oldpw, *newpw;
	int count;
} pam_chpass_t;

#define PAM_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(pam, v)

#if defined(ZTS) && defined(COMPILE_DL_PAM)
ZEND_TSRMLS_CACHE_EXTERN()
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
