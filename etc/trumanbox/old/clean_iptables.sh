#!/bin/sh
#
# remove all iptable roules
#

iptables -F
iptables -t nat -F
iptables -t mangle -F
iptables -t raw -F

