#!/bin/sh
#
# clean all ebtables entries
#

ebtables -F
ebtables -t nat -F
ebtables -t broute -F
