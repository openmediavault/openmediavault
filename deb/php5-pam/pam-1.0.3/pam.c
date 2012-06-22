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

/* $Id: pam.c 291416 2009-11-29 10:47:35Z mikl $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pam.h"
#include <security/pam_appl.h>

ZEND_DECLARE_MODULE_GLOBALS(pam)

/* {{{ pam_functions[]
 */
zend_function_entry pam_functions[] = {
	PHP_FE(pam_auth,	NULL)
	PHP_FE(pam_chpass,	NULL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ pam_module_entry
 */
zend_module_entry pam_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"pam",
	pam_functions,
	PHP_MINIT(pam),
	PHP_MSHUTDOWN(pam),
	NULL,					/* Replace with NULL if there's nothing to do at request start */
	NULL,					/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pam),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_PAM_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PAM
ZEND_GET_MODULE(pam)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("pam.servicename", "php", PHP_INI_ALL, OnUpdateString, servicename, zend_pam_globals, pam_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_pam_init_globals
 */
static void php_pam_init_globals(zend_pam_globals *pam_globals)
{
	pam_globals->servicename = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pam)
{
	ZEND_INIT_MODULE_GLOBALS(pam, php_pam_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pam)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pam)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "PAM support", "enabled");
	php_info_print_table_row(2, "Extension version", PHP_PAM_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
 * auth_pam_talker: supply authentication information to PAM when asked
 *
 * Assumptions:
 *   A password is asked for by requesting input without echoing
 *   A username is asked for by requesting input _with_ echoing
 *
 */
static
int auth_pam_talker(int num_msg,
				const struct pam_message ** msg,
				struct pam_response ** resp,
				void *appdata_ptr)
{
	unsigned short i = 0;
	pam_auth_t *userinfo = (pam_auth_t *) appdata_ptr;
	struct pam_response *response = 0;

	/* parameter sanity checking */
	if (!resp || !msg || !userinfo)
		return PAM_CONV_ERR;

	/* allocate memory to store response */
	response = malloc(num_msg * sizeof(struct pam_response));
	if (!response)
		return PAM_CONV_ERR;

	/* copy values */
	for (i = 0; i < num_msg; i++) {
		/* initialize to safe values */
		response[i].resp_retcode = 0;
		response[i].resp = 0;

		/* select response based on requested output style */
		switch (msg[i]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
				/* on memory allocation failure, auth fails */
				response[i].resp = strdup(userinfo->name);
				break;
			case PAM_PROMPT_ECHO_OFF:
				response[i].resp = strdup(userinfo->pw);
				break;
			default:
				if (response)
				free(response);
				return PAM_CONV_ERR;
		}
	}
	/* everything okay, set PAM response values */
	*resp = response;
	return PAM_SUCCESS;
}

/*
 * chpass_pam_talker: supply authentication information to PAM when asked
 *
 * Assumptions:
 *   A password is asked for by requesting input without echoing
 *   A username is asked for by requesting input _with_ echoing
 *
 */
static
int chpass_pam_talker(int num_msg,
				const struct pam_message ** msg,
				struct pam_response ** resp,
				void *appdata_ptr)
{
	unsigned short i = 0;
	pam_chpass_t *userinfo = (pam_chpass_t *) appdata_ptr;
	struct pam_response *response = 0;

	/* parameter sanity checking */
	if (!resp || !msg || !userinfo)
		return PAM_CONV_ERR;

	/* allocate memory to store response */
	response = malloc(num_msg * sizeof(struct pam_response));
	if (!response)
		return PAM_CONV_ERR;

	/* copy values */
	for (i = 0; i < num_msg; i++) {
		/* initialize to safe values */
		response[i].resp_retcode = 0;
		response[i].resp = 0;

		/* select response based on requested output style */
		switch (msg[i]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
				/* on memory allocation failure, auth fails */
				response[i].resp = strdup(userinfo->name);
				break;
			case PAM_PROMPT_ECHO_OFF:
				if (userinfo->count++) {
					response[i].resp = strdup(userinfo->newpw);
				}
				else {
					response[i].resp = strdup(userinfo->oldpw);
				}
				break;
			default:
				if (response)
				free(response);
				return PAM_CONV_ERR;
		}
	}
	/* everything okay, set PAM response values */
	*resp = response;
	return PAM_SUCCESS;
}

/* {{{ proto bool pam_auth( string host, string password [, string &status [ bool checkacctmgmt = true ] ])
   Authenticates a user and returns TRUE on success, FALSE on failure */
PHP_FUNCTION(pam_auth)
{
	char *username, *password;
	int username_len, password_len;
	zval *status = NULL, **server, **remote_addr;
	zend_bool checkacctmgmt = 1;

	pam_auth_t userinfo = {NULL, NULL};
	struct pam_conv conv_info = {&auth_pam_talker, (void *) &userinfo};
	pam_handle_t *pamh = NULL;
	int result;
	char *error_msg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|zb", &username, &username_len, &password, &password_len, &status, &checkacctmgmt) == FAILURE) {
		return;
	}

	userinfo.name = username;
	userinfo.pw = password;

	if ((result = pam_start(PAM_G(servicename), userinfo.name, &conv_info, &pamh)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_start");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg, 0);
		}
		RETURN_FALSE;
	}

	if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **)&server) == SUCCESS && Z_TYPE_PP(server) == IS_ARRAY) {
		if (zend_hash_find(Z_ARRVAL_PP(server), "REMOTE_ADDR", sizeof("REMOTE_ADDR"), (void **)&remote_addr) == SUCCESS && Z_TYPE_PP(remote_addr) == IS_STRING) {
			pam_set_item(pamh, PAM_RHOST, Z_STRVAL_PP(remote_addr));
		}
	}

	if ((result = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_authenticate");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg, 0);
		}
		pam_end(pamh, PAM_SUCCESS);
		RETURN_FALSE;
	}

	if (checkacctmgmt) {
		if ((result = pam_acct_mgmt(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
			if (status) {
				spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_acct_mgmt");
				zval_dtor(status);
				ZVAL_STRING(status, error_msg, 0);
			}
			pam_end(pamh, PAM_SUCCESS);
			RETURN_FALSE;
		}
	}

	pam_end(pamh, PAM_SUCCESS);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool pam_auth( string host, string password [, string &status ])
   Changes a users password and returns TRUE on success, FALSE on failure */
PHP_FUNCTION(pam_chpass)
{
	char *username, *oldpass, *newpass;
	int username_len, oldpass_len, newpass_len;
	zval *status = NULL;

	pam_chpass_t userinfo = {NULL, NULL, NULL, 0};
	struct pam_conv conv_info = {&chpass_pam_talker, (void *) &userinfo};
	pam_handle_t *pamh = NULL;
	int result;
	char *error_msg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|z", &username, &username_len, &oldpass, &oldpass_len, &newpass, &newpass_len, &status) == FAILURE) {
		return;
	}

	userinfo.name = username;
	userinfo.oldpw = oldpass;
	userinfo.newpw = newpass;

	if ((result = pam_start(PAM_G(servicename), userinfo.name, &conv_info, &pamh)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_start");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg, 0);
		}
		RETURN_FALSE;
	}

	if ((result = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_authenticate");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg, 0);
		}
		pam_end(pamh, PAM_SUCCESS);
		RETURN_FALSE;
	}

	if ((result = pam_chauthtok(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_chauthtok");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg, 0);
		}
		pam_end(pamh, PAM_SUCCESS);
		RETURN_FALSE;
	}

	pam_end(pamh, PAM_SUCCESS);
	RETURN_TRUE;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
