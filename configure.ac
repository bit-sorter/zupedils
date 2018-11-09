#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([zupedils], [0.9], [craig.j.mcp@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/zupedils.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

PKG_CHECK_MODULES([GTK], [gtk+-3.0], [], [
                   AC_MSG_ERROR([gtk+-3 required.])])

AC_CONFIG_FILES([Makefile src/Makefile])

CFLAGS="$CFLAGS $GTK_CFLAGS -Wall"
LIBS="$LIBS $GTK_LIBS"

AC_OUTPUT
