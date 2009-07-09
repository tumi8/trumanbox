# change the following variables according to your
# configuration/setup
#

# for bridging set BRIDGE=1, otherwise we provide 
# emulation with only one NIC (BRIDGE=0) 
BRIDGE=0

# if you want your bridge to be invisible
STEALTHY=0

# with this flag, all DNS requests are redirected
# to the local running DNS server. in case you are
# using an offline configuration, remember to change 
# the DNS server configuration accordingly (see 
# example bind configuration files in etc/bind in 
# this package
NO_OUTGOING_DNS=0

# the nameserver is only used if the TrumanBox itself
# needs to resolve domainnames
NAMESERVER=10.211.55.1

# gateway used by the TrumanBox
GW_IP=10.211.55.1

# IF_CLIENT is the interface pointing to the client(s) 
# you want to provide with simulation
IF_CLIENT=eth1
# MAC address of the corresponding interface
IF_CLIENT_MAC=

# IF_OUT is pointing to the Internet (only for bridge
# mode)
IF_OUT=eth0

# name of the bridge device
BRIDGE_DEV=br0

# ip of the bridge 
BRIDGE_IP=10.211.55.3

# udp ports we do not handle
UNHANDLED_UDP_PORTS=53,67,5353
