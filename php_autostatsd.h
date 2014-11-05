#ifndef PHP_AUTOSTATSD_H
#define PHP_AUTOSTATSD_H

#define PHP_AUTOSTATSD_VERSION "1.0"

#include "php.h"

extern zend_module_entry autostatsd_module_entry;
#define phpext_autostatsd_ptr &autostatsd_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(autostatsd);
PHP_MSHUTDOWN_FUNCTION(autostatsd);
PHP_RINIT_FUNCTION(autostatsd);
PHP_RSHUTDOWN_FUNCTION(autostatsd);
PHP_FUNCTION(autostatsd_template_get_attributes);

#endif