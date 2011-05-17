SUBDIRS=common logging protocols

bin_PROGRAMS=trumanbox

trumanbox_SOURCES=config.h dns_resolver.h dns_resolver.cpp helper_file.h \
	signals.cpp tcp_handler.h tcp_handler.cpp ssl_handler.h ssl_handler.cpp \
	dispatching.cpp udp_handler.h udp_handler.cpp helper_net.cpp \
	semaphore.cpp signals.h  wrapper.h \
	wrapper.cpp \
	dispatching.h helper_file.cpp helper_net.h \
	semaphore.h process_manager.h process_manager.cpp \
	main.h main.cpp

trumanbox_LDADD= $(top_builddir)/src/protocols/libprotocols.a $(top_builddir)/src/logging/liblogging.a $(top_builddir)/src/common/libcommon.a

AM_CFLAGS=   `xml2-config --cflags` $(OPENDPI_CFLAGS) $(POSTGRESQL_CFLAGS) $(OPENSSL_CFLAGS) -Wall -Werror -g -O0
AM_LDFLAGS=`xml2-config --libs` -lsqlite3 $(POSTGRESQL_LDFLAGS) $(OPENSSL_LDFLAGS)
LDADD=$(OPENDPI_LIBRARY)
