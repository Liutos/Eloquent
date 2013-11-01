AC_INIT([233-Lisp], [0.1], [mat.liutos@gmail.com])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
# Checks for libraries
AC_CHECK_LIB([gc], [GC_MALLOC])
AC_CHECK_LIB([gmp], [mpz_init])
AC_CHECK_HEADERS([stdint.h stdlib.h string.h strings.h unistd.h])
AC_HEADER_STDBOOL
AC_TYPE_UINT32_T
AC_CHECK_FUNCS([getcwd strdup strerror strndup])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
