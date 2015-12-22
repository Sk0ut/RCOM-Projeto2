if [ -n "$1" ]; then
ifconfig eth0 up
ifconfig eth0 172.16.$10.1/24
route add default gw 172.16.$10.254
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
echo Finished tux1 configs.
cp resolv.conf /etc/
echo Copied resolv.conf file.
else
	echo Missing argument.
fi
