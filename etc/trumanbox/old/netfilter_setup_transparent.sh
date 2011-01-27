#!/bin/sh

## include the configuration parameter
. /etc/trumanbox/bridge_config.sh

# if we want some logging...
#iptables -t mangle -A PREROUTING -i $BRIDGE_DEV -d ! $BRIDGE_IP -j LOG

## we send certain incoming packages to the QUEUE
iptables -t mangle -A PREROUTING -i $BRIDGE_DEV -d ! $BRIDGE_IP -j QUEUE

## redirecting the dns requests
#iptables -t nat -A PREROUTING -i $BRIDGE_DEV -m physdev --physdev-in $IF_CLIENT -p udp --dport 53 -j REDIRECT --to-port 53


