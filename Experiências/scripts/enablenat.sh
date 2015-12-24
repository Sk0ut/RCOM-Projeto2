#Translate addresses going out through interface eth1
iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE 
#Drop NEW and INVALID packets entering through interface eth1
iptables -A FORWARD -i eth1 -m state --state NEW,INVALID -j DROP
