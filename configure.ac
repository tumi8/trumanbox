AC_INIT
AM_INIT_AUTOMAKE(TrumanBox, 0.2.99)
AC_PREREQ(2.59)

AC_LANG([C])

AC_CHECK_HEADERS(netdb.h netinet/in.h errno.h)
AC_CHECK_HEADERS(stdarg.h stdint.h stdio.h stdlib.h string.h sys/ipc.h)
AC_CHECK_HEADERS(sys/sem.h sys/socket.h sys/stat.h sys/time.h sys/types.h)
AC_CHECK_HEADERS(sys/wait.h time.h unistd.h dirent.h)

AM_PATH_XML2(2.4.0,,AC_MSG_ERROR([*** LibXML version $XML_VERSION not found!]))
AX_LIB_SQLITE3(3.0)
AX_LIB_POSTGRESQL([8.3])


#AC_CHECK_LIB([ssl], [SSL_library_init], [], [AC_MSG_FAILURE([could not find 
#ssl])])
AX_CHECK_OPENSSL()

# new argument --with-opendpi:
#	if used: use opendpi for protocol payload identification
#                The argument is expected to be the path to an extracted and compiled opendpi directory source package.
#                As for the time of writing, there is no install target with opendpi (version 1.0.1). Therefore the directory layout
#                is taken from the opendpi source package: ARG/include for includes ARG/libOpenDPI.a for the compiled library
AC_DEFUN([CHECK_OPENDPI], [AC_MSG_NOTICE([Looking for OpenDPI files...])
                           AC_CHECK_HEADER(["$with_opendpi/include/ipq_api.h"], [], [AC_MSG_ERROR(" ****   Could not find ipq_api.h")])
                           OPENDPI_CFLAGS="-I$with_opendpi/include"
                           OPENDPI_LIBRARY="$with_opendpi/libOpenDPI.a"
                           AC_SUBST(OPENDPI_CFLAGS)
                           AC_SUBST(OPENDPI_LIBRARY)
                           AC_CHECK_FILE([$OPENDPI_LIBRARY], [], [AC_MSG_ERROR(Could not find $OPENDPI_LIBRARY. Please compile your sources.)])
                          ]
        )
AC_ARG_WITH([opendpi],
            [AS_HELP_STRING([--with-opendpi=/path/to/opendpi/directory],
                            [Directory which contains the compiled OpenDPI sources])],
            [],
            [with_opendpi=disabled])
AS_IF([test "x$with_opendpi" = "xdisabled"],
       [AC_MSG_NOTICE([OpenDPI support is disabled!])],
       [test "x$with_opendpi" != "xdisabled"],
       [CHECK_OPENDPI
        AC_DEFINE(WITH_OPENDPI)
       ]
     )

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
