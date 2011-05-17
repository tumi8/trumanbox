noinst_LIBRARIES=libprotocols.a

libprotocols_a_SOURCES = proto_ident.h proto_ident.cpp proto_handler.h \
	proto_handler.cpp proto_ident_truman.cpp \
	proto_ident_truman.h proto_ident_opendpi.h proto_ident_opendpi.cpp \
	ftp.h ftp.cpp http.h http.cpp \
	ftp_data.h ftp_data.cpp ssl.h ssl.cpp \
	smtp.h smtp.cpp irc.h irc.cpp \
	unknown.h unknown.cpp unknown_udp.h unknown_udp.cpp

AM_CFLAGS=-Wall -Werror -I$(top_srcdir)/src
AM_CXXFLAGS=-I$(srcdir) -I$(top_srcdir)/src -Wall -Werror
