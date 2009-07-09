#!/bin/sh

## include the configuration parameter
. /etc/trumanbox/bridge_config.sh

## first we send certain incoming packages to the QUEUE
#iptables -t mangle -A PREROUTING -m physdev --physdev-in $IF_CLIENT -p tcp -d ! $BRIDGE_IP -m state --state NEW -j QUEUE
#iptables -t mangle -A PREROUTING -m physdev --physdev-in $IF_CLIENT -p udp -d ! $BRIDGE_IP -m multiport --dports ! $UNHANDLED_UDP_PORTS -m state --state NEW -j QUEUE

## then we redirect the packages after being enqueued again
iptables -t nat -A PREROUTING -m physdev --physdev-in $IF_CLIENT -p tcp -d ! $BRIDGE_IP -m state --state NEW -j REDIRECT --to-port 400
iptables -t nat -A PREROUTING -m physdev --physdev-in $IF_CLIENT -p udp -d ! $BRIDGE_IP -m multiport --dports ! $UNHANDLED_UDP_PORTS -m state --state NEW -j REDIRECT --to-port 400

# and we redirect all ICMP echo requests
iptables -t nat -A PREROUTING -m physdev --physdev-in $IF_CLIENT -p icmp -d ! $BRIDGE_IP -m state --state NEW -j REDIRECT

## redirecting the dns requests
if [ $NO_OUTGOING_DNS == 1 ] || [ $BRIDGE != 1 ]; then
	iptables -t nat -A PREROUTING -m physdev --physdev-in $IF_CLIENT -p udp --dport 53 -j REDIRECT
else
	iptables -A FORWARD -m physdev --physdev-in $IF_CLIENT -p udp --dport 53 -j ACCEPT
fi

# inet -> client: allow everything
iptables -A FORWARD -m physdev --physdev-in $IF_OUT --physdev-out $IF_CLIENT -j ACCEPT

## drop all other packets, that would be forwarded otherwise
iptables -A FORWARD -j DROP

