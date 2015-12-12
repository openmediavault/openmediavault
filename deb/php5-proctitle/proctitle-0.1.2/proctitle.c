/*
  +----------------------------------------------------------------------+
  | PHP Version 5 / proctitle                                            |
  +----------------------------------------------------------------------+
  | Copyright (c) 2005-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+

  $Id$ 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <SAPI.h>
#include <dlfcn.h>
#include <string.h>

#ifdef HAVE_PRCTL
#include <sys/prctl.h>
#endif

#include "php_proctitle.h"

static char *argv0 = NULL;
#define MAX_TITLE_LENGTH        128

#ifndef PHP_SYSTEM_PROVIDES_SETPROCTITLE
static void setproctitle(char *title, int tlen)
{
	char    buffer[MAX_TITLE_LENGTH];

	if(!argv0) {
		return; /* no point running all this if we got no argv0 */
	}

	/* space padding */
	memset(buffer, 0x20, MAX_TITLE_LENGTH);
	buffer[MAX_TITLE_LENGTH-1] = '\0';

	/* title too long => truncate */
	if (tlen >= (MAX_TITLE_LENGTH-1)) tlen = (MAX_TITLE_LENGTH-1);

	memcpy(buffer, title, tlen);

	snprintf(argv0, MAX_TITLE_LENGTH, "%s", buffer);
}

PHP_MINIT_FUNCTION(proctitle)
{
	sapi_module_struct *symbol = NULL;

	symbol = &sapi_module;

	if( symbol )
		argv0 = symbol->executable_location;
}
#endif

/* {{{ proto void setproctitle(string title)
 * Changes the current process' title in system's list of processes
 */
PHP_FUNCTION(setproctitle)
{
	char    *title;
	long    tlen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",&title,
			&tlen) == FAILURE) {
		RETURN_NULL();
	}

#ifndef PHP_SYSTEM_PROVIDES_SETPROCTITLE
	/* local (incompatible) setproctitle */
	setproctitle(title, tlen);
#else
	/* let's use system setproctitle() (BSD or compatible) */
	setproctitle("%s", title);
#endif
}
/* }}} */

#if HAVE_PRCTL
/* {{{ bool mixed setthreadtitle(string title)
   Sets the thread name */
PHP_FUNCTION(setthreadtitle)
{
	char *title;
	int title_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &title, &title_len) == FAILURE) {
		return;
	}
	if (0 == prctl(PR_SET_NAME, title, 0, 0, 0)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}

}
/* }}} */
#endif

/* {{{ proctitle_functions[]
 *
 * Every user visible function must have an entry in proctitle_functions[].
 */
static zend_function_entry proctitle_functions[] = {
	PHP_FE(setproctitle,	NULL)
#if HAVE_PRCTL
	PHP_FE(setthreadtitle,	NULL)
#endif
	{NULL, NULL, NULL}	/* Must be the last line in proctitle_functions[] */
};
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(proctitle)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "proctitle support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ proctitle_module_entry
 */
zend_module_entry proctitle_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"proctitle",
	proctitle_functions,
#ifndef PHP_SYSTEM_PROVIDES_SETPROCTITLE
	PHP_MINIT(proctitle),
#else
	NULL,
#endif
	NULL,
	NULL,
	NULL,
	PHP_MINFO(proctitle),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_PROCTITLE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PROCTITLE
ZEND_GET_MODULE(proctitle)
#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
