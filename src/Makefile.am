bin_PROGRAMS=trumanbox

trumanbox_SOURCES=config.h definitions.h dns_resolver.c helper_file.h iniparser.c  \
	main.h signals.c tcp_handler.h wrapper.c \
	configuration.c dispatching.c dns_resolver.h helper_net.c iniparser.h  msg.c \
	semaphore.c signals.h udp_handler.c  wrapper.h \
	configuration.h dispatching.h helper_file.c helper_net.h main.c msg.h \
	semaphore.h tcp_handler.c udp_handler.h process_manager.h process_manager.c \
	logger.h logger.c log_truman.h log_truman.c log_sqlite.h log_sqlite.c \
	log_postgres.h log_postgres.c \
	protocols/proto_ident.h protocols/proto_ident.c protocols/proto_handler.h \
	protocols/proto_handler.c protocols/proto_ident_truman.c \
	protocols/proto_ident_truman.h protocols/proto_ident_opendpi.h protocols/proto_ident_opendpi.c \
	protocols/ftp.h protocols/ftp.c protocols/http_get.h protocols/http_get.c \
	protocols/http_post.c protocols/http_post.h \
	protocols/http_put.c protocols/http_put.h  \
	protocols/smtp.h protocols/smtp.c protocols/irc.h protocols/irc.c \
	protocols/unknown.h protocols/unknown.c

AM_CFLAGS= -I/usr/local/include -L/usr/local/lib `xml2-config --cflags` $(OPENDPI_CFLAGS) -Wall -Werror -g -O0
AM_LDFLAGS=`xml2-config --libs` -lsqlite3 -lpq
LDADD=$(OPENDPI_LIBRARY)
