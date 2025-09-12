/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5d7edd77ec802eac610edf6086a09ea41aa67afb */

ZEND_BEGIN_ARG_INFO_EX(arginfo_pam_auth, 0, 0, 2)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(1, status)
	ZEND_ARG_INFO(0, checkacctmgmt)
	ZEND_ARG_INFO(0, servicename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_pam_chpass, 0, 0, 3)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, oldpassword)
	ZEND_ARG_INFO(0, newpassword)
	ZEND_ARG_INFO(1, status)
	ZEND_ARG_INFO(0, servicename)
ZEND_END_ARG_INFO()


ZEND_FUNCTION(pam_auth);
ZEND_FUNCTION(pam_chpass);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(pam_auth, arginfo_pam_auth)
	ZEND_FE(pam_chpass, arginfo_pam_chpass)
	ZEND_FE_END
};
