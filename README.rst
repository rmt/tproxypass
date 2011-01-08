=============================
Transparent Proxy Passthrough
=============================

You can use this program with Linux 2.6 and iptables's transparent proxy
support and inetd/xinetd to call another program with the original destination
IP and port number appended to the command line.

*Why?* you ask.  This little bit of glue will allow you to use existing
proxying tools (eg. netcat-openbsd, connect-proxy, corkscrew, via an ssh
tunnel, etc.) to send normal TCP connections via a proxy... or you could use it
to just redirect some traffic to your own program, or intercept traffic, or..

If you're still asking *why*, then this tool probably isn't for you.

To transparently proxy all requests for port 80 through a SOCKS proxy at
192.168.1.1 (using netcat-openbsd), you could use the following config:

::

  service tproxy
  {
  	socket_type		= stream
  	protocol		= tcp
        port                    = 8081
  	wait			= no
  	user			= nobody
  	server			= /usr/local/bin/tproxypass
  	server_args		= /bin/nc -X 5 -x 192.168.1.1:1080
  	disable			= no
  	per_source		= 11
  	cps			= 100 2
  	flags			= IPv4
  }

--------------
IPtables Setup
--------------

Try the following iptables rules (for both local and routed traffic) to
redirect all port 80 traffic that isn't destined for the local 192.168/16
network through this port.

::

  root@hostname# iptables -t nat -A OUTPUT -p tcp --dport 80 \! -d 192.168.0.0/16 --syn -j REDIRECT --to-port 8081
  root@hostname# iptables -t nat -A PREROUTING -p tcp --dport 80 \! -d 192.168.0.0/16 --syn -j REDIRECT --to-port 8081

