/*
# Transparent Proxy Helper Script
#
# Put this in front of your inetd script to have the original
# destination IP and port appended as arguments.
#

# To transparently proxy all requests for port 80 through a SOCKS proxy
# at 192.168.1.1 (using netcat-openbsd), you could use the following
# config:

# Contents of /etc/xinetd.d/tproxy
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

# .. and the following iptables rules (for both local and routed traffic)

root@hostname# iptables -t nat -A OUTPUT -p tcp --dport 80 \! -d 192.168.0.0/16 --syn -j REDIRECT --to-port 8081
root@hostname# iptables -t nat -A PREROUTING -p tcp --dport 80 \! -d 192.168.0.0/16 --syn -j REDIRECT --to-port 8081

*/
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/netfilter_ipv4.h>

int main(int argc, char **argv) {
    int client_fd;
    int proxy_fd;
    struct sockaddr_in dest_addr; // addr of intended destination
    int length;
    char *dsthost;
    char *dstport = (char *)malloc(6);
    char **nargv = (char**)malloc(sizeof(char *)*(argc+2));
    int i;

    /* */
    if(argc <= 1) {
        fprintf(stderr, "Syntax: %s <full path to command> [arg] [...]\n", argv[0]);
        fprintf(stderr, "Additionally, it must be called with an IPv4 network socket as the STDIN_FILENO (eg. from xinetd)\n");
        return 2;
    }

    /* get intended destination, or die */
    length = sizeof(dest_addr);
    if (getsockopt(STDIN_FILENO, SOL_IP, SO_ORIGINAL_DST, (char *)&dest_addr, &length) < 0) {
        return 1;
    }

    /* copy args across to our argv */
    for(i=0; i<argc-1; i++) {
        nargv[i] = argv[i+1];
    }
    /* add the original destination IP to nargv */
    nargv[i++] = inet_ntoa(dest_addr.sin_addr);
    /* add the original destination port to nargv */
    sprintf(dstport, "%d", ntohs(dest_addr.sin_port));
    nargv[i++] = dstport;
    nargv[i++] = NULL;
    /* execute command line */
    execv(nargv[0], nargv);
    //for(i=0; i<(argc+2); i++) {
    //    printf("%d: %s\n", i, nargv[i]);
    //}
}
