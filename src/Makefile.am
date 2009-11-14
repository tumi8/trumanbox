bin_PROGRAMS=trumanbox

trumanbox_SOURCES=config.h definitions.h dns_resolver.c helper_file.h iniparser.c  \
	main.h payload_alter_log.c payload_ident.h signals.c tcp_handler.h wrapper.c \
	configuration.c dispatching.c dns_resolver.h helper_net.c iniparser.h  msg.c \
	payload_alter_log.h semaphore.c signals.h udp_handler.c  wrapper.h \
	configuration.h dispatching.h helper_file.c helper_net.h main.c msg.h payload_ident.c \
	semaphore.h tcp_handler.c udp_handler.h process_manager.h process_manager.c

AM_CFLAGS=`xml2-config --cflags` $(MYSQL_CFLAGS) -Wall -Werror
AM_LDFLAGS=`xml2-config --libs`
