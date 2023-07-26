#if !defined(_WIN32) && !defined(WIN32)
#include "aiwn.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
typedef struct CInAddr {
	char *address;
	int64_t port;
	struct sockaddr_in sa;
} CInAddr;
typedef struct CNetAddr {
	struct addrinfo *ai;
} CNetAddr;
int64_t NetSocketNew() {
	int64_t s=socket(AF_INET,SOCK_STREAM,0);
	int yes=1;
	//On FreeBSD the port will stay in use for awhile after death,so reuse the address
	setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
	return s;
}

CNetAddr *NetAddrNew(char *host,int64_t port) {
	CNetAddr *ret=A_CALLOC(sizeof(CNetAddr),NULL);
	char buf[1024];
	struct addrinfo hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;
	sprintf(buf,"%d",port);
	getaddrinfo(host,buf,&hints,&ret->ai);
	return ret;
}

void NetAddrDel(CNetAddr *addr) {
	freeaddrinfo(addr->ai);
	A_FREE(addr);
}

void NetBindIn(int64_t socket,CNetAddr *address) {
	bind(socket,address->ai->ai_addr,address->ai->ai_addrlen);
}
void NetListen(int64_t socket,int64_t max) {
	listen(socket,max);
}
int64_t NetAccept(int64_t socket,CNetAddr **addr) {
	struct sockaddr sa;
	socklen_t ul=sizeof(sa);
	int64_t con=accept(socket,&sa,&ul);
	if(addr) *addr=NULL;
/*	if(addr) {
		*addr=A_CALLOC(sizeof(CNetAddr),NULL);
		addr[0]->sa=sa;
	}*/
	return con;
}
void NetClose(int64_t s) {
	//https://stackoverflow.com/questions/48208236/tcp-close-vs-shutdown-in-linux-os
	shutdown(s,SHUT_WR);
	shutdown(s,SHUT_RD);
	close(s);
}
int64_t NetWrite(int64_t s,char *data,int64_t len) {
	return write(s,data,len);
} 
int64_t NetRead(int64_t s,char *data,int64_t len) {
	return read(s,data,len);
}
static int64_t _PollFor(int64_t _for,int64_t argc,int64_t *argv) {
	struct pollfd poll_for[argc];
	int64_t idx;
	for(idx=0;idx!=argc;idx++) {
		poll_for[idx].fd=argv[0];
		poll_for[idx].events=_for;
		poll_for[idx].revents=0;
	}
	poll(poll_for,argc,0);
	for(idx=0;idx!=argc;idx++) {
		if(poll_for[idx].revents&POLLHUP) {
			if(_for&POLLHUP)
				return idx;
			continue;
		}
		if(poll_for[idx].revents&_for)
			return idx;
	}
	return -1;
}
int64_t NetPollForRead(int64_t argc,int64_t *argv) {
	return _PollFor(POLLIN,argc,argv);
}
int64_t NetPollForWrite(int64_t argc,int64_t *argv) {
	return _PollFor(POLLOUT,argc,argv);
}
int64_t NetPollForHangup(int64_t argc,int64_t *argv) {
	return _PollFor(POLLHUP,argc,argv);
}
#endif
