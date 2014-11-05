#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_autostatsd.h"
#include "php_network.h"

zend_function_entry autostatsd_functions[] = {
	{NULL, NULL, NULL}
};

zend_module_entry autostatsd_module_entry = {
	STANDARD_MODULE_HEADER,
	"autostatsd",
	autostatsd_functions,
	PHP_MINIT(autostatsd),
	PHP_MSHUTDOWN(autostatsd),
	PHP_RINIT(autostatsd),
	PHP_RSHUTDOWN(autostatsd),
	NULL,
	PHP_AUTOSTATSD_VERSION,
	STANDARD_MODULE_PROPERTIES
};

PHP_INI_BEGIN()
PHP_INI_ENTRY("autostatsd.host", "127.0.0.1", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("autostatsd.port", 8125, PHP_INI_ALL, NULL)
PHP_INI_END()

#ifdef COMPILE_DL_AUTOSTATSD
ZEND_GET_MODULE(autostatsd)
#endif


PHP_MINIT_FUNCTION(autostatsd)
{
    REGISTER_INI_ENTRIES();

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(autostatsd)
{
    UNREGISTER_INI_ENTRIES();

    return SUCCESS;
}

PHP_RINIT_FUNCTION(autostatsd)
{
	return SUCCESS;
}

static php_stream *stream = NULL;

void open_statsd_stream()
{
	int options = ENFORCE_SAFE_MODE;
	int flags = STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT;

	char *host = INI_STR("autostatsd.host");
	int port = INI_INT("autostatsd.port");

	char *url = NULL;
	int url_len = spprintf(&url, 0, "%s%s:%d", "udp://", host, port);

	char *errstr;
	int errcode;

	stream = php_stream_xport_create(url, url_len, options, flags, NULL, NULL, NULL, &errstr, &errcode);
}

void close_statsd_stream()
{
	php_stream_close(stream);
}

void push_stat(char *metric, double val, char *type)
{
	char *data = NULL;
	int data_len = spprintf(&data, 0, "%s:%g|%s", metric, val, type);
	php_stream_write(stream, data, data_len);
}

double get_current_time()
{
	double current_time = 0;

	zval *function_name;
	MAKE_STD_ZVAL(function_name);
	ZVAL_STRING(function_name, "microtime", 1);

	zval *bool_value;
	MAKE_STD_ZVAL(bool_value);
	ZVAL_BOOL(bool_value, 1);

	zval *params = {bool_value};

	zval *microtime;
	MAKE_STD_ZVAL(microtime);

	if (call_user_function(CG(function_table), NULL, function_name, microtime, 1, &params TSRMLS_CC) == SUCCESS) {
		current_time = Z_DVAL_P(microtime);
    }

	zval_dtor(function_name);
	zval_dtor(bool_value);
	zval_dtor(params);
	zval_dtor(microtime);
	FREE_ZVAL(function_name);
	FREE_ZVAL(bool_value);
	FREE_ZVAL(params);
	FREE_ZVAL(microtime);

	return current_time;
}

double get_request_start_time()
{
	double start_time = 0;

	zval **server_vars, **request_time;
    if (zend_hash_find(&EG(symbol_table), "_SERVER", 8, &server_vars) != FAILURE) {
        HashTable *ht = Z_ARRVAL_PP(server_vars);
        if (zend_hash_find(ht, "REQUEST_TIME_FLOAT", 19, &request_time) != FAILURE) {
            start_time = Z_DVAL_PP(request_time);
        }
    }

    return start_time;
}

double get_elapsed_time()
{
	double current_time = get_current_time();
	double start_time = get_request_start_time();
	double elapsed_time_ms = 0;

	if (start_time != 0 && current_time != 0) {
		elapsed_time_ms = (current_time-start_time) * 1000;
	}

	return elapsed_time_ms;
}

PHP_RSHUTDOWN_FUNCTION(autostatsd)
{
	open_statsd_stream();

	push_stat("php.request.count", 1, "c");
	push_stat("php.request.memory.peak", zend_memory_peak_usage(0), "h");
	push_stat("php.request.memory.peak.real", zend_memory_peak_usage(1), "h");
	push_stat("php.request.memory.current", zend_memory_usage(0), "h");
	push_stat("php.request.memory.current.real", zend_memory_usage(1), "h");
	push_stat("php.request.time", get_elapsed_time(), "ms");

	close_statsd_stream();

	return SUCCESS;
}
