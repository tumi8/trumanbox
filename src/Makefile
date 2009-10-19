CC = gcc
CFLAGS = -Wall -Werror
#CFLAGS = -I/usr/include/libipq -Wall
#LDFLAGS = -lipq
#LDFLAGS = -lpthread

SOURCES = $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SOURCES))

truman: $(OBJS)
	${CC} ${CFLAGS} -o trumanbox $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

#wrapper.o: wrapper.c
#	${CC} ${CFLAGS} -c wrapper.c

deps: $(SOURCES)
	$(CC) -MD -E $(CFLAGS) $(SOURCES) > /dev/null

clean:
	rm -f *.o *.d
	rm -f ./trumanbox
#	sudo rm -rf /tmp/truman-box

install: uninstall
	mkdir /etc/trumanbox
	cp trumanbox /usr/sbin/
	cp ../etc/trumanbox/bridge_config.sh /etc/trumanbox/
	cp ../etc/trumanbox/clean_iptables.sh /etc/trumanbox/
	cp ../etc/trumanbox/clean_ebtables.sh /etc/trumanbox/
	cp ../etc/trumanbox/netfilter_setup.sh /etc/trumanbox/
	cp ../etc/trumanbox/netfilter_setup_transparent.sh /etc/trumanbox/
	cp ../etc/trumanbox/setting_up_the_bridge.sh /etc/trumanbox/

uninstall:
	rm -rf /etc/trumanbox
	rm -f /usr/sbin/trumanbox
