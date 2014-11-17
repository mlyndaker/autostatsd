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
PHP_INI_ENTRY("autostatsd.metric_prefix", AUTOSTATSD_DEFAULT_METRIC_PREFIX, PHP_INI_ALL, NULL)
PHP_INI_END()

#ifdef COMPILE_DL_AUTOSTATSD
ZEND_GET_MODULE(autostatsd)
#endif

/**
 * Calculate the real world running time of the request.
 *
 * @return double Elapsed time in microseconds or 0.00 on error
 */
double request_elapsed_time()
{
    struct timeval tp = {0};

    if(gettimeofday(&tp, NULL)) {
        return 0.00;
    }

    double current_time = (double)(tp.tv_sec + tp.tv_usec / 1000000.00);
    double request_time = sapi_get_request_time();

    // check for errors
    if (current_time <= 0.00 || request_time <= 0.00) {
        return 0.00;
    }

    return (current_time - request_time) * 1000;
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

    char *pre = INI_STR("autostatsd.metric_prefix");

    statsd_stream_buffer_metric(ss, pre, "request.count", 1, "c");
    statsd_stream_buffer_metric(ss, pre, "request.time", request_elapsed_time(), "ms");
    statsd_stream_buffer_metric(ss, pre, "request.memory.peak", zend_memory_peak_usage(0), "g");
    statsd_stream_buffer_metric(ss, pre, "request.memory.peak.real", zend_memory_peak_usage(1), "g");
    statsd_stream_buffer_metric(ss, pre, "request.memory.current", zend_memory_usage(0), "g");
    statsd_stream_buffer_metric(ss, pre, "request.memory.current.real", zend_memory_usage(1), "g");

    statsd_stream_close(ss);
    statsd_stream_free(ss);

    return SUCCESS;
}
