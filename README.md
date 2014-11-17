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

## Configuration

The extension can be configured by setting these values in your PHP ini file.

| Option                    | Default   | Description
| ------------------------- | --------- | -----------
| autostatsd.host           | 127.0.0.1 | StatsD collector hostname or IP address
| autostatsd.port           | 8125      | StatsD collector port number
| autostatsd.buffer_size    | 512       | Maximum UDP payload size in bytes for buffering metrics

## Author

Matt Lyndaker - mlyndaker@gmail.com
