# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([ABL],[1.2],[mvzaanen@uvt.nl])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/tree.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([math.h getopt.h unistd.h signal.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_HEADER_STDC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([alarm select])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([doc/Makefile src/Makefile])
AC_OUTPUT

# $Id: configure.ac 3755 2010-02-19 11:23:46Z menno $
