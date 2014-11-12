PHP_ARG_ENABLE(autostatsd, whether to enable autostatsd support,
[  --enable-autostatsd           Enable autostatsd support])

if test "$PHP_AUTOSTATSD" != "no"; then
  PHP_NEW_EXTENSION(autostatsd, autostatsd.c statsd_stream.c, $ext_shared)
fi