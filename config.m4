PHP_ARG_ENABLE(autostatsd, whether to enable autostatsd support,
[  --enable-autostatsd           Enable autostatsd support])

if test "$PHP_TWIG" != "no"; then
  PHP_NEW_EXTENSION(autostatsd, autostatsd.c, $ext_shared)
fi