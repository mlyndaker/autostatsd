#include "statsd_stream.h"

/**
 * Create a connection string for the UDP transport.
 *
 * @param const char *const         [host]          statsd hostname or IP address
 * @param const int                 [port]          statsd port number
 * @return char*                                    formatted transport string
 */
char * statsd_stream_get_transport(const char *const host, const int port)
{
    char *transport = NULL;
    spprintf(&transport, 0, "%s%s:%d", "udp://", host, port);

    return transport;
}

/**
 * Free the memory held by the buffer.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @return bool                                     whether the buffer was freed
 */
bool statsd_stream_free_buffer(statsd_stream *ss)
{
    if (ss == NULL || ss->buffer == NULL) {
        return false;
    }
    efree(ss->buffer);

    return true;
}

/**
 * Initialize an empty statsd stream buffer of the given size.
 *
 * @param statsd_stream *           [ss]            uninitialized statsd stream
 * @param const size_t              [buffer_size]   maximum buffer bytes
 * @return bool                                     whether the buffer was initialized
 */
bool statsd_stream_init_buffer(statsd_stream *ss, const size_t buffer_size)
{
    if (ss == NULL) {
        return false;
    }

    // free existing buffer
    statsd_stream_free_buffer(ss);

    // keep the buffer size within min/max range
    size_t safe_size = buffer_size;
    safe_size = (safe_size < STATSD_STREAM_MIN_BUFFER_SIZE) ? STATSD_STREAM_MIN_BUFFER_SIZE : safe_size;
    safe_size = (safe_size > STATSD_STREAM_MAX_BUFFER_SIZE) ? STATSD_STREAM_MAX_BUFFER_SIZE : safe_size;

    // create an empty buffer
    ss->buffer = safe_emalloc(sizeof(char), safe_size, sizeof(char)); // include the NULL char
    ss->buffer_size = safe_size;
    strcpy(ss->buffer, "");

    return true;
}

/**
 * Open the statsd stream using the given connection string.
 * The stream will be in the READY state if it was opened or
 * in the ERROR state if something went wrong.
 *
 * @param statsd_stream *           [ss]            uninitialized statsd stream
 * @param const char *const         [transport]     connection string
 * @return bool                                     whether the stream was opened
 */
bool statsd_stream_open_stream(statsd_stream *ss, const char *const transport)
{
    if (ss == NULL) {
        return false;
    }

    int errcode = 0;
    char *errstr = NULL;
    char *hashkey = NULL;

    // key used to find the persistent connection
    spprintf(&hashkey, 0, "statsd_stream__%s", transport);

    ss->stream = php_stream_xport_create(
       transport,
       strlen(transport),
       STATSD_STREAM_OPTIONS,
       STATSD_STREAM_FLAGS,
       hashkey,
       NULL,
       NULL,
       &errstr,
       &errcode
    );

    efree(errstr);
    efree(hashkey);

    // error opening the stream
    if (errcode != 0) {
        ss->stream = NULL;
        ss->status = STATSD_STREAM_STATUS_ERROR;
        return false;
    }

    ss->status = STATSD_STREAM_STATUS_READY;

    return true;
}

/**
 * Close the php stream.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @return bool                                     whether the stream was closed
 */
bool statsd_stream_close_stream(statsd_stream *ss)
{
    if (ss == NULL) {
        return false;
    }

    php_stream_close(ss->stream);

    return true;
}

/**
 * Free the memory held by the php stream.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @return bool                                     whether the stream was freed
 */
bool statsd_stream_free_stream(statsd_stream *ss)
{
    if (ss == NULL || ss->stream == NULL) {
        return false;
    }

    // memory mgmt?

    ss->status = STATSD_STREAM_STATUS_CLOSED;
    ss->stream = NULL;

    return true;
}

/**
 * Create a new instance of the statsd struct.
 *
 * @return *statsd_stream_new                       uninitialized statsd stream
 */
statsd_stream *statsd_stream_new()
{
    statsd_stream *ss = emalloc(sizeof(statsd_stream));

    ss->status = STATSD_STREAM_STATUS_NEW;
    ss->buffer_size = 0;
    ss->buffer = NULL;
    ss->stream = NULL;

    return ss;
}

/**
 * Free the memory held by the statsd stream info instance.
 * The stream will be closed and the buffer and stream will be freed.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @return bool                                     whether the instance was freed
 */
bool statsd_stream_free(statsd_stream *ss)
{
    if (ss == NULL) {
        return false;
    }

    statsd_stream_close(ss);
    statsd_stream_free_stream(ss);
    statsd_stream_free_buffer(ss);
    efree(ss);

    return true;
}

/**
 * Create a new statsd stream with the given buffer size and open
 * a connection to the given host and port. NULL is returned on error.
 *
 * @param const char *const         [host]          statsd hostname or IP address
 * @param const int                 [port]          statsd port
 * @param const size_t              [buffer_size]   size of the buffer in bytes
 * @return NULL|*statsd_stream_new                  initialized statsd stream
 */
statsd_stream *statsd_stream_create(const char *const host, const int port, const size_t buffer_size)
{
    statsd_stream *ss = statsd_stream_new();
    statsd_stream_init_buffer(ss, buffer_size);

    // try to open the php stream
    char *transport = statsd_stream_get_transport(host, port);
    statsd_stream_open_stream(ss, transport);
    efree(transport);

    // error opening the stream
    if (ss->status != STATSD_STREAM_STATUS_READY) {
        return NULL;
    }

    return ss;
}

/**
 * Write the blob of data immediately to the initialized statsd stream.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @param const char *const         [data]          data to be written
 * @return bool                                     whether the data was written
 */
bool statsd_stream_write(statsd_stream *ss, const char *const data)
{
    if (ss == NULL || data == NULL || ss->status != STATSD_STREAM_STATUS_READY) {
        return false;
    }

    int data_len = strlen(data);
    int write_len = php_stream_write(ss->stream, data, data_len);

    // The socket was closed (EOF) or not all bytes were written
    if (data_len != write_len || php_stream_eof(ss->stream) != 0) {
        ss->status = STATSD_STREAM_STATUS_ERROR;
        return false;
    }

    return true;
}

/**
 * Try to place the data into the stream buffer. Return false on failure.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @param const char *const         [data]          data to be buffered
 * @return bool                                     whether the data was buffered
 */
bool statsd_stream_try_buffer(statsd_stream *ss, const char *data)
{
    if (ss == NULL || data == NULL || ss->buffer == NULL) {
        return false;
    }

    int data_len = strlen(data);
    int buff_len = strlen(ss->buffer);
    int delm_len = strlen(STATSD_STREAM_DELIMITER);

    int free_len = ss->buffer_size - buff_len;
    int req_len = (buff_len > 0) ? data_len + delm_len : data_len;

    // not enough free space in the buffer
    if (req_len > free_len) {
        return false;
    }

    // prepend the delimiter if a previous stat exists
    if (buff_len > 0) {
        strcat(ss->buffer, STATSD_STREAM_DELIMITER);
    }

    // append the data
    strcat(ss->buffer, data);

    return true;
}

/**
 * Buffers the data if possible before writing to the stream. Flushes the buffer if full.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @param const char *const         [data]          data to be buffered
 * @return bool                                     whether the data was buffered or written
 */
bool statsd_stream_buffer_data(statsd_stream *ss, const char *const data)
{
    if (ss == NULL || data == NULL) {
        return false;
    }

    // try to buffer the data
    if (statsd_stream_try_buffer(ss, data)) {
        return true;
    }

    // flush the buffer once if the data does not fit
    statsd_stream_flush(ss);

    // try to buffer the data again
    if (statsd_stream_try_buffer(ss, data)) {
        return true;
    }

    // write the data to the stream if all else fails
    statsd_stream_write(ss, data);

    return true;
}

/**
 * Buffers the metric if possible before writing to the stream. Flushes the buffer if full.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @param const char *const         [prefix]        prefix prepended to metric name
 * @param const char *const         [name]          metric name
 * @param const double              [val]           metric value
 * @param const char *const         [type]          metric type
 * @return bool                                     whether the metric was buffered or written
 */
bool statsd_stream_buffer_metric(
    statsd_stream *ss,
    const char *const prefix,
    const char *const name,
    const double val,
    const char *const type
) {
    char *data;
    int retval = 0;

    spprintf(&data, 0, "%s.%s:%g|%s", prefix, name, val, type);
    retval = statsd_stream_buffer_data(ss, data);
    efree(data);

    return retval;
}

/**
 * Flush the buffer content to the stream and reset the buffer.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @return bool                                     whether the stream was flushed
 */
bool statsd_stream_flush(statsd_stream *ss)
{
    if (ss == NULL || ss->buffer == NULL || strlen(ss->buffer) <= 0) {
        return false;
    }

    // write out the buffer and then clear its contents
    statsd_stream_write(ss, ss->buffer);
    strcpy(ss->buffer, "");

    return true;
}

/**
 * Flush any remaining buffer entries and close the statsd stream.
 *
 * @param statsd_stream *           [ss]            initialized statsd stream
 * @return bool                                     whether the statsd stream was closed
 */
bool statsd_stream_close(statsd_stream *ss)
{
    if (ss == NULL) {
        return false;
    }

    statsd_stream_flush(ss);

    return true;
}
