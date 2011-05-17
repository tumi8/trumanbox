SUBDIRS=common

bin_PROGRAMS=trumanbox

trumanbox_SOURCES=config.h definitions.h dns_resolver.cpp helper_file.h iniparser.c  \
	main.h signals.cpp tcp_handler.h wrapper.cpp \
	configuration.cpppp dispatching.cpp dns_resolver.h helper_net.cpp iniparser.h  msg.cpp \
	semaphore.cpp signals.h udp_handler.cpp  wrapper.h \
	configuration.h dispatching.h helper_file.cpp helper_net.h main.cpp msg.h \
	semaphore.h tcp_handler.cpp udp_handler.h process_manager.h process_manager.cpp \
	logger.h logger.cpp log_truman.h log_truman.cpp log_sqlite.h log_sqlite.cpp \
	log_postgres.h log_postgres.cpp ssl_handler.h ssl_handler.cpp\
	protocols/proto_ident.h protocols/proto_ident.cpp protocols/proto_handler.h \
	protocols/proto_handler.cpp protocols/proto_ident_truman.cpp \
	protocols/proto_ident_truman.h protocols/proto_ident_opendpi.h protocols/proto_ident_opendpi.cpp \
	protocols/ftp.h protocols/ftp.cpp protocols/http.h protocols/http.cpp \
	protocols/ftp_data.h protocols/ftp_data.cpp protocols/ssl.h protocols/ssl.cpp \
	protocols/smtp.h protocols/smtp.cpp protocols/irc.h protocols/irc.cpp \
	protocols/unknown.h protocols/unknown.cpp protocols/unknown_udp.h protocols/unknown_udp.cpp

AM_CFLAGS=   `xml2-config --cflags` $(OPENDPI_CFLAGS) $(POSTGRESQL_CFLAGS) $(OPENSSL_CFLAGS) -Wall -Werror -g -O0
AM_LDFLAGS=`xml2-config --libs` -lsqlite3 $(POSTGRESQL_LDFLAGS) $(OPENSSL_LDFLAGS)
LDADD=$(OPENDPI_LIBRARY)
