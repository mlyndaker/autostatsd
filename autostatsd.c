#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_autostatsd.h"
#include "php_network.h"

zend_function_entry autostatsd_functions[] = {
	{NULL, NULL, NULL}
};


zend_module_entry autostatsd_module_entry = {
	STANDARD_MODULE_HEADER,
	"autostatsd",
	autostatsd_functions,
	NULL,
	NULL,
	PHP_RINIT(autostatsd),
	PHP_RSHUTDOWN(autostatsd),
	NULL,
	PHP_AUTOSTATSD_VERSION,
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_AUTOSTATSD
ZEND_GET_MODULE(autostatsd)
#endif


PHP_RINIT_FUNCTION(autostatsd)
{
	return SUCCESS;
}

static php_stream *stream = NULL;
static char persistent_id[64] = {0};

static char *host = "127.0.0.1";
static int port = 8125;
static int options = ENFORCE_SAFE_MODE;
static int flags = STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT;

void open_statsd_stream()
{
	char *url = NULL;
	int url_len = spprintf(&url, 0, "%s%s:%d", "udp://", host, port);

	char *errstr;
	int errcode;

	stream = php_stream_xport_create(url, url_len, options, flags, persistent_id, NULL, NULL, &errstr, &errcode);
}

void send_statsd_datagram(char *metric, int value, char *type)
{
	char *data = NULL;
	int data_len = spprintf(&data, 0, "%s:%d|%s", metric, value, type);

    php_stream_write(stream, data, data_len);
}

float get_request_time()
{

}

PHP_RSHUTDOWN_FUNCTION(autostatsd)
{
	open_statsd_stream();
	send_statsd_datagram("php.request.count", 1, "c");
	send_statsd_datagram("php.request.memory.peak", zend_memory_peak_usage(0), "h");
	send_statsd_datagram("php.request.memory.peak.real", zend_memory_peak_usage(1), "h");
	send_statsd_datagram("php.request.memory.current", zend_memory_usage(0), "h");
	send_statsd_datagram("php.request.memory.current.real", zend_memory_usage(1), "h");


	return SUCCESS;
}