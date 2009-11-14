AC_INIT
AC_PREREQ(2.59)
AM_INIT_AUTOMAKE(TrumanBox, 0.2.01)
AC_PROG_RANLIB

AC_LANG([C])

AC_CHECK_HEADERS(netdb.h netinet/in.h netinet/ip.h netinet/tcp.h netinet/udp.h)
AC_CHECK_HEADERS(stdarg.h stdint.h stdio.h stdlib.h string.h sys/ipc.h)
AC_CHECK_HEADERS(sys/sem.h sys/socket.h sys/stat.h sys/time.h sys/types.h)
AC_CHECK_HEADERS(sys/wait.h time.h unistd.h)

AM_PATH_XML2(2.4.0,,AC_MSG_ERROR([*** LibXML version $XML_VERSION not found!]))

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
