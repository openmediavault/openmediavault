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

#ifndef PHP_PROCTITLE_H
#define PHP_PROCTITLE_H

#define PHP_PROCTITLE_VERSION "0.1.2"

extern zend_module_entry proctitle_module_entry;
#define phpext_proctitle_ptr &proctitle_module_entry

PHP_MINIT_FUNCTION(proctitle);
PHP_MINFO_FUNCTION(proctitle);

PHP_FUNCTION(setproctitle);

#endif	/* PHP_PROCTITLE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
