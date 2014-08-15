#!/bin/csh -f

# PERMEATE
# Copyright (C) 2008 Jarek Paduch, Bilal Khan, Jamie Levy
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Email: jarek.paduch@gmail.com, grouptheory@gmail.com, jamie.levy@gmail.com
# The above header may not be removed or modified without contacting the authors.

echo "Welcome to PERMEATE..."
echo "NOTE: this is the server/receiver"
echo " "

setenv ME `whoami`
if ("$ME" != "root") then
    echo "You must be root to run permeate-client, exiting."
    exit
else
    echo "You are running permeate-client as root... good."
endif

setenv IPQ `/sbin/lsmod | grep ip_queue | sed -e 's/ .*//' | wc -l`
if ("$IPQ" != "1") then
    echo "You must install the ip_queue module, exiting."
else
    echo "The ip_queue module is installed."
endif

if ("$#" == "2") then
setenv REMOTE $1
setenv SEND $2
else
    echo "Usage: juggler.sh remote-IP file-to-send"
    exit
endif

/sbin/iptables -A OUTPUT -p tcp -d $REMOTE -j QUEUE

rm -f /tmp/.cleanup-$REMOTE.sh
echo "#\!/bin/csh -f" > /tmp/.cleanup-$REMOTE.sh
echo "/sbin/iptables -D OUTPUT -p tcp -d $REMOTE -j QUEUE" >> /tmp/.cleanup-$REMOTE.sh
echo "rm /tmp/.cleanup-$REMOTE.sh" >> /tmp/.cleanup-$REMOTE.sh
chmod 700 /tmp/.cleanup-$REMOTE.sh

echo "PERMEATE is ready."
./permeate c $2 $REMOTE

