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

static php_stream *g_stream = NULL;
static char g_persistent_id[64] = {0};

PHP_RSHUTDOWN_FUNCTION(autostatsd)
{
	char *pData = "test.value.counter:1|c";

    int port = 8125;
	char *host = "127.0.0.1";
	char *transport = NULL;
	int transportLen = spprintf(&transport, 0, "%s%s:%d","udp://" , host, port);

	int options = ENFORCE_SAFE_MODE;
	char * errstr;
	int errcode;
	int flags = STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT;

	g_stream = php_stream_xport_create(transport, transportLen,options, flags,g_persistent_id, NULL, NULL, &errstr, &errcode);
	int writeSize = php_stream_write(g_stream,pData ,strlen(pData));

	return SUCCESS;
}