# AutoStatsD

PHP extension for sending automatic metrics to [StatsD](https://github.com/b/statsd_spec).

Record information about each request including peak memory usage and execution time.

## Build & Installation

1. Clone the [autostatsd](https://github.com/mlyndaker/autostatsd.git) repository.

2. Run the commands below to build the extension.

    ```
    cd autostatsd
    phpize
    ./configure
    make
    make install
    ```

3. Add the extension to php.ini or create a new config file for autostatsd in php.d.

    ```
    [autostatsd]
    extension="autostatsd.so"
    ```

4. Restart httpd or php-fpm if using FastCGI.

## Configuration

The extension can be configured by setting these values in your PHP ini file.

| Option                    | Default   | Description
| ------------------------- | --------- | -----------
| autostatsd.host           | 127.0.0.1 | StatsD collector hostname or IP address
| autostatsd.port           | 8125      | StatsD collector port number
| autostatsd.buffer_size    | 512       | Maximum UDP payload size in bytes for buffering metrics
| autostatsd.metric_prefix  | php       | Prefix prepended to all metrics

## Usage

Once the extension has been installed, PHP will report the following metrics on every request.

Data types are defined [here](https://github.com/etsy/statsd/blob/master/docs/metric_types.md).

| Metric                                    | Type | Description
| ----------------------------------------- | ---- | -----------
| {pref}.request.count                      | c    | Increment the request counter
| {pref}.request.time                       | ms   | Total request time in milliseconds
| {pref}.request.memory.peak                | g    | Peak memory used by the script in bytes. [memory_get_peak_usage(false)](http://php.net/manual/en/function.memory-get-peak-usage.php)
| {pref}.request.memory.peak.real           | g    | Peak memory allocated by the system in bytes. [memory_get_peak_usage(true)](http://php.net/manual/en/function.memory-get-peak-usage.php)
| {pref}.request.memory.current             | g    | Current memory used by the script in bytes. [memory_get_usage(false)](http://php.net/manual/en/function.memory-get-usage.php)
| {pref}.request.memory.current.real        | g    | Current memory allocated by the system in bytes. [memory_get_usage(true)](http://php.net/manual/en/function.memory-get-usage.php)
| {pref}.response.status_code.{code}.count  | c    | Increment the HTTP status code count

## Author

Matt Lyndaker - mlyndaker@gmail.com
