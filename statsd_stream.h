#ifndef STATSD_STREAM_H
#define STATSD_STREAM_H

#include "stdbool.h"
#include "php_network.h"

#define STATSD_STREAM_DELIMITER "\n"
#define STATSD_STREAM_OPTIONS ENFORCE_SAFE_MODE
#define STATSD_STREAM_FLAGS (STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT)

#define STATSD_STREAM_MIN_BUFFER_SIZE 1
#define STATSD_STREAM_MAX_BUFFER_SIZE (UINT_MAX - 1)

#define STATSD_STREAM_STATUS_NEW 1
#define STATSD_STREAM_STATUS_READY 2
#define STATSD_STREAM_STATUS_CLOSED 3
#define STATSD_STREAM_STATUS_ERROR 4

typedef struct {
    int status;
    size_t buffer_size;
    char *buffer;
    php_stream *stream;
} statsd_stream;

statsd_stream *statsd_stream_create(const char *const host, const int port, const size_t buffer_size);
bool statsd_stream_flush(statsd_stream *ss);
bool statsd_stream_close(statsd_stream *ss);
bool statsd_stream_free(statsd_stream *ss);
bool statsd_stream_buffer_data(statsd_stream *ss, const char *const data);
bool statsd_stream_buffer_metric(statsd_stream *ss, const char *const name, const double val, const char *const type);

#endif
