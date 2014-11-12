#include "php_autostatsd.h"

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
PHP_INI_ENTRY("autostatsd.host", AUTOSTATSD_DEFAULT_HOST, PHP_INI_ALL, NULL)
PHP_INI_ENTRY("autostatsd.port", AUTOSTATSD_DEFAULT_PORT, PHP_INI_ALL, NULL)
PHP_INI_ENTRY("autostatsd.buffer_size", AUTOSTATSD_DEFAULT_BUFFER_SIZE, PHP_INI_ALL, NULL)
PHP_INI_END()

#ifdef COMPILE_DL_AUTOSTATSD
ZEND_GET_MODULE(autostatsd)
#endif

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
    if (zend_hash_find(&EG(symbol_table), "_SERVER", 8, (void **)&server_vars) != FAILURE) {
        HashTable *ht = Z_ARRVAL_PP(server_vars);
        if (zend_hash_find(ht, "REQUEST_TIME_FLOAT", 19, (void **)&request_time) != FAILURE) {
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

PHP_RSHUTDOWN_FUNCTION(autostatsd)
{
    statsd_stream *ss = statsd_stream_create(
        INI_STR("autostatsd.host"),
        INI_INT("autostatsd.port"),
        INI_INT("autostatsd.buffer_size")
    );

    statsd_stream_buffer_metric(ss, "php.request.count", 1, "c");
    statsd_stream_buffer_metric(ss, "php.request.memory.peak", zend_memory_peak_usage(0), "h");
    statsd_stream_buffer_metric(ss, "php.request.memory.peak.real", zend_memory_peak_usage(1), "h");
    statsd_stream_buffer_metric(ss, "php.request.memory.current", zend_memory_usage(0), "h");
    statsd_stream_buffer_metric(ss, "php.request.memory.current.real", zend_memory_usage(1), "h");
    statsd_stream_buffer_metric(ss, "php.request.time", get_elapsed_time(), "ms");

    statsd_stream_close(ss);
    statsd_stream_free(ss);

    return SUCCESS;
}
