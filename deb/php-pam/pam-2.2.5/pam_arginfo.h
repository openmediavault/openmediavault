/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5d7edd77ec802eac610edf6086a09ea41aa67afb */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pam_auth, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, username, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(1, status, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, checkacctmgmt, _IS_BOOL, 0, "true")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, servicename, IS_STRING, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_pam_chpass, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, username, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, oldpassword, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, newpassword, IS_STRING, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(1, status, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, servicename, IS_STRING, 0, "null")
ZEND_END_ARG_INFO()


ZEND_FUNCTION(pam_auth);
ZEND_FUNCTION(pam_chpass);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(pam_auth, arginfo_pam_auth)
	ZEND_FE(pam_chpass, arginfo_pam_chpass)
	ZEND_FE_END
};
