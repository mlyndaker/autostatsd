#ifndef PHP_AUTOSTATSD_H
#define PHP_AUTOSTATSD_H

#define PHP_AUTOSTATSD_VERSION "1.0"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_autostatsd.h"

#include "time.h"
#include "SAPI.h"
#include "statsd_stream.h"

extern zend_module_entry autostatsd_module_entry;
#define phpext_autostatsd_ptr &autostatsd_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

#define AUTOSTATSD_DEFAULT_HOST "127.0.0.1"
#define AUTOSTATSD_DEFAULT_PORT "8125"
#define AUTOSTATSD_DEFAULT_BUFFER_SIZE "512"

PHP_MINIT_FUNCTION(autostatsd);
PHP_MSHUTDOWN_FUNCTION(autostatsd);
PHP_RINIT_FUNCTION(autostatsd);
PHP_RSHUTDOWN_FUNCTION(autostatsd);
PHP_FUNCTION(autostatsd_template_get_attributes);

#endif
