/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#ifdef __cplusplus
}
#endif // __cplusplus
#include "php_pinyin.h"
#include "dictionary.h"

ZEND_DECLARE_MODULE_GLOBALS(pinyin)

/* True global resources - no need for thread safety here */
static int le_pinyin;
static CDictionary g_dic;
static bool g_dic_load = false;
/* {{{ pinyin_functions[]
 *
 * Every user visible function must have an entry in pinyin_functions[].
 */
const zend_function_entry pinyin_functions[] = {
	PHP_FE(confirm_pinyin_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(pinyin, NULL)
	PHP_FE_END	/* Must be the last line in pinyin_functions[] */
};
/* }}} */

/* {{{ pinyin_module_entry
 */
zend_module_entry pinyin_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"pinyin",
	pinyin_functions,
	PHP_MINIT(pinyin),
	PHP_MSHUTDOWN(pinyin),
	PHP_RINIT(pinyin),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pinyin),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pinyin),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_PINYIN_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PINYIN
ZEND_GET_MODULE(pinyin)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY((char*)"pinyin.dict_file", (char*)"", PHP_INI_ALL, OnUpdateString, dict_file, zend_pinyin_globals, pinyin_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_pinyin_init_globals
 */
static void php_pinyin_init_globals(zend_pinyin_globals *pinyin_globals)
{
	pinyin_globals->dict_file = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pinyin)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pinyin)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pinyin)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pinyin)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pinyin)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pinyin support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_pinyin_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_pinyin_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "pinyin", arg);
	RETURN_STRINGL(strg, len, 0);
}

PHP_FUNCTION(pinyin)
{
	char* chinese = NULL;
	int chinese_len;
	char* splitword = NULL;
	char *strg;
	int splitword_len;
	UINT load_count = 0;
    const char *dict_file = INI_STR((char*)"pinyin.dict_file");
    
    if (!strlen(dict_file))
    {
        zend_throw_exception(NULL, "Failed to load Chinese dictionary. Please use 'pinyin.dict_file' configure it in php.ini or ini_set('pinyin.dict_file', 'realpath/to/the/dict').", 0 TSRMLS_CC);
        RETURN_NULL();
    }

	if (!g_dic_load)
	{
		if (!g_dic.load_directionary(dict_file, load_count))
		{
			RETURN_NULL();
		}
		g_dic_load = true;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", 
			&chinese, &chinese_len, &splitword, &splitword_len) == FAILURE)
	{
		RETURN_NULL();
	}

	string str_chinese(chinese, chinese_len);
	string str_split(splitword, splitword_len);
	string str_pinyin;

	g_dic.chinese2pinyin(str_chinese, str_pinyin, str_split);
	
    int len = spprintf(&strg, 0, "%s", str_pinyin.c_str());
	
    RETURN_STRINGL(strg, len, 1);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
