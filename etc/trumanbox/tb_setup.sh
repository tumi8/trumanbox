#!/bin/sh

MALWARE_IF_IP=192.168.27.254
MALWARE_IF=eth1
INTERNET_IF=eth0

iptables -F 
iptables -t nat -F

echo 0 > /proc/sys/net/ipv4/ip_forward

## include the configuration parameter
## then we redirect the packages after being enqueued again
echo "Redirect TCP to 400"
iptables -t nat -A PREROUTING -i $MALWARE_IF -p tcp ! -d $MALWARE_IF_IP -m state --state NEW -j REDIRECT --to-port 400
echo "Redirect UDP to 400"
iptables -t nat -A PREROUTING -i $MALWARE_IF -p udp ! -d $MALWARE_IF_IP -m multiport ! --dports  53,67 -m state --state NEW -j REDIRECT --to-port 400 


# and we redirect all ICMP echo requests
echo "Redirect ICMP"
iptables -t nat -A PREROUTING -m physdev --physdev-in $MALWARE_IF -p icmp ! -d $MALWARE_IF_IP -m state --state NEW -j REDIRECT

# inet -> client: allow everything
echo "Allow forwarding  from internet to malware..."
iptables -A FORWARD -m physdev --physdev-in $INTERNET_IF --physdev-out $MALWARE_IF -j ACCEPT

echo "Drop rest..."
## drop all other packets, that would be forwarded otherwise
iptables -P FORWARD DROP
iptables -A FORWARD -j DROP

