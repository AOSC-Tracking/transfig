# Version information
# This file is included by configure.ac.

m4_define([FIGFILEVERSION], [3.2])
m4_define([FIGPATCHLEVEL], [6-beta])

dnl m4_define must be called before AC_INIT - but, I believe,
dnl shell-variables can only be defined after a call to AC_INIT.
dnl Therefore, define RELEASE_DATE as a macro.
m4_define([RELEASEDATE], [May 2016])
