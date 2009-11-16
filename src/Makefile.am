bin_PROGRAMS=trumanbox

trumanbox_SOURCES=config.h definitions.h dns_resolver.c helper_file.h iniparser.c  \
	main.h payload_alter_log.c proto_ident.h signals.c tcp_handler.h wrapper.c \
	configuration.c dispatching.c dns_resolver.h helper_net.c iniparser.h  msg.c \
	payload_alter_log.h semaphore.c signals.h udp_handler.c  wrapper.h \
	configuration.h dispatching.h helper_file.c helper_net.h main.c msg.h payload_ident.c \
	semaphore.h tcp_handler.c udp_handler.h process_manager.h process_manager.c \
	proto_handler.h proto_handler.c proto_ident.h proto_ident.c proto_ident_truman.c \
	proto_ident_truman.h proto_ident_opendpi.h proto_ident_opendpi.c 

AM_CFLAGS=`xml2-config --cflags` $(OPENDPI_CFLAGS) -Wall -Werror
AM_LDFLAGS=`xml2-config --libs`
LDADD=$(OPENDPI_LIBRARY)
