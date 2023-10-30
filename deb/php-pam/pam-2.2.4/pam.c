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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pam.h"
#include <security/pam_appl.h>
#include <security/_pam_macros.h>

#if PHP_VERSION_ID < 80000
#include "pam_legacy_arginfo.h"
#else
#include "pam_arginfo.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(pam)

/* True global resources - no need for thread safety here */
static int le_pam;

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("pam.servicename", "php", PHP_INI_ALL, OnUpdateString, servicename, zend_pam_globals, pam_globals)
	STD_PHP_INI_BOOLEAN("pam.force_servicename", "0", PHP_INI_ALL, OnUpdateBool, force_servicename, zend_pam_globals, pam_globals)
PHP_INI_END()
/* }}} */

/* {{{ auth_pam_talker: supply authentication information to PAM when asked
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
	struct pam_response *response = NULL;

	/* parameter sanity checking */
	if (!resp || !msg || !userinfo)
		return PAM_CONV_ERR;

	/* allocate memory to store response */
	response = malloc(num_msg * sizeof(struct pam_response));
	if (!response)
		return PAM_CONV_ERR;

	/* copy values */
	for (i = 0; i < num_msg; ++i) {
		/* initialize to safe values */
		response[i].resp_retcode = 0;
		response[i].resp = NULL;

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
				_pam_drop_reply(response, i);
				return PAM_CONV_ERR;
		}
	}
	/* everything okay, set PAM response values */
	*resp = response;
	return PAM_SUCCESS;
}
/* }}} */

/* {{{ chpass_pam_talker: supply authentication information to PAM when asked
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
	struct pam_response *response = NULL;

	/* parameter sanity checking */
	if (!resp || !msg || !userinfo)
		return PAM_CONV_ERR;

	/* allocate memory to store response */
	response = malloc(num_msg * sizeof(struct pam_response));
	if (!response)
		return PAM_CONV_ERR;

	/* copy values */
	for (i = 0; i < num_msg; ++i) {
		/* initialize to safe values */
		response[i].resp_retcode = 0;
		response[i].resp = NULL;

		/* select response based on requested output style */
		switch (msg[i]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
				/* on memory allocation failure, auth fails */
				response[i].resp = strdup(userinfo->name);
				break;
			case PAM_PROMPT_ECHO_OFF:
				response[i].resp = strdup((userinfo->count++) ? userinfo->newpw : userinfo->oldpw);
				break;
			default:
				_pam_drop_reply(response, i);
				return PAM_CONV_ERR;
		}
	}
	/* everything okay, set PAM response values */
	*resp = response;
	return PAM_SUCCESS;
}
/* }}} */

/* {{{ proto bool pam_auth( string username, string password [, string &status [, bool checkacctmgmt = true [, string servicename ] ] ])
   Authenticates a user and returns TRUE on success, FALSE on failure */
PHP_FUNCTION(pam_auth)
{
	char *username, *password, *srvname = NULL;
	size_t username_len, password_len, srvname_len = 0;
	zval *status = NULL, *server, *remote_addr;
	zend_bool checkacctmgmt = 1;

	pam_auth_t userinfo = {NULL, NULL};
	struct pam_conv conv_info = {&auth_pam_talker, (void *) &userinfo};
	pam_handle_t *pamh = NULL;
	int result;
	char *error_msg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|z/bs", &username, &username_len, &password, &password_len, &status, &checkacctmgmt, &srvname, &srvname_len) == FAILURE) {
		RETURN_FALSE;
	}

	userinfo.name = username;
	userinfo.pw = password;

	if ((result = pam_start((PAM_G(force_servicename) || !srvname || srvname_len < 1 || !srvname[0]) ? PAM_G(servicename) : srvname, userinfo.name, &conv_info, &pamh)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_start");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg);
			efree(error_msg);
		}
		RETURN_FALSE;
	}

	if ((server = zend_hash_str_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER")-1)) != NULL && Z_TYPE_P(server) == IS_ARRAY) {
		if ((remote_addr = zend_hash_str_find(Z_ARRVAL_P(server), "REMOTE_ADDR", sizeof("REMOTE_ADDR")-1)) != NULL && Z_TYPE_P(remote_addr) == IS_STRING) {
			pam_set_item(pamh, PAM_RHOST, Z_STRVAL_P(remote_addr));
		}
	}

	if ((result = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_authenticate");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg);
			efree(error_msg);
		}
		pam_end(pamh, PAM_SUCCESS);
		RETURN_FALSE;
	}

	if (checkacctmgmt) {
		if ((result = pam_acct_mgmt(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
			if (status) {
				spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_acct_mgmt");
				zval_dtor(status);
				ZVAL_STRING(status, error_msg);
				efree(error_msg);
			}
			pam_end(pamh, PAM_SUCCESS);
			RETURN_FALSE;
		}
	}

	pam_end(pamh, PAM_SUCCESS);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool pam_chpass( string username, string oldpassword, string newpassword [, string &status [, string servicename ] ])
   Changes a users password and returns TRUE on success, FALSE on failure */
PHP_FUNCTION(pam_chpass)
{
	char *username, *oldpass, *newpass, *srvname = NULL;
	size_t username_len, oldpass_len, newpass_len, srvname_len = 0;
	zval *status = NULL;

	pam_chpass_t userinfo = {NULL, NULL, NULL, 0};
	struct pam_conv conv_info = {&chpass_pam_talker, (void *) &userinfo};
	pam_handle_t *pamh = NULL;
	int result;
	char *error_msg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss|z/s", &username, &username_len, &oldpass, &oldpass_len, &newpass, &newpass_len, &status, &srvname, &srvname_len) == FAILURE) {
		RETURN_FALSE;
	}

	userinfo.name = username;
	userinfo.oldpw = oldpass;
	userinfo.newpw = newpass;

	if ((result = pam_start((PAM_G(force_servicename) || !srvname || srvname_len < 1 || !srvname[0]) ? PAM_G(servicename) : srvname, userinfo.name, &conv_info, &pamh)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_start");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg);
			efree(error_msg);
		}
		RETURN_FALSE;
	}

	if ((result = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_authenticate");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg);
			efree(error_msg);
		}
		pam_end(pamh, PAM_SUCCESS);
		RETURN_FALSE;
	}

	if ((result = pam_chauthtok(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
		if (status) {
			spprintf(&error_msg, 0, "%s (in %s)", (char *) pam_strerror(pamh, result), "pam_chauthtok");
			zval_dtor(status);
			ZVAL_STRING(status, error_msg);
			efree(error_msg);
		}
		pam_end(pamh, PAM_SUCCESS);
		RETURN_FALSE;
	}

	pam_end(pamh, PAM_SUCCESS);
	RETURN_TRUE;
}
/* }}} */

/* {{{ php_pam_init_globals
 */
static void php_pam_init_globals(zend_pam_globals *pam_globals)
{
	pam_globals->servicename = NULL;
	pam_globals->force_servicename = 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pam)
{
#if defined(ZTS) && defined(COMPILE_DL_PAM)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
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

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pam)
{
#if defined(ZTS) && defined(COMPILE_DL_PAM)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
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

/* {{{ pam_module_entry
 */
zend_module_entry pam_module_entry = {
	STANDARD_MODULE_HEADER,
	"pam",
	ext_functions,
	PHP_MINIT(pam),
	PHP_MSHUTDOWN(pam),
	PHP_RINIT(pam),
	NULL,	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pam),
	PHP_PAM_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PAM
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(pam)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
