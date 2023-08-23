#include "aiwn.h"
#if !defined(_WIN32) && !defined(WIN32)
  #include <netdb.h>
  #include <netinet/in.h>
  #include <poll.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <unistd.h>
#else
  #define _WIN32_WINNT 0x603
  #include <winsock2.h>
  #include <windows.h>
  #include <iphlpapi.h>
  #include <ws2tcpip.h>
static int64_t was_init = 0;
WSADATA        ws_data;

static void InitWS2() {
  WSAStartup(MAKEWORD(2, 2), &ws_data);
  was_init = 1;
}
#endif
typedef struct CInAddr {
  char              *address;
  int64_t            port;
  struct sockaddr_in sa;
} CInAddr;

typedef struct CNetAddr {
  struct addrinfo *ai;
} CNetAddr;
int64_t NetSocketNew() {
#if defined(_WIN32) || defined(WIN32)
  if (!was_init)
    InitWS2();
#endif
  int64_t s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if defined(__FreeBSD__) || defined(__linux__)
  int yes = 1;
  // On FreeBSD the port will stay in use for awhile after death,so reuse the
  // address
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
  return s;
}

CNetAddr *NetAddrNew(char *host, int64_t port) {
  CNetAddr       *ret = A_CALLOC(sizeof(CNetAddr), NULL);
  char            buf[1024];
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;
  sprintf(buf, "%d", port);
  getaddrinfo(host, buf, &hints, &ret->ai);
  return ret;
}

void NetAddrDel(CNetAddr *addr) {
  freeaddrinfo(addr->ai);
  A_FREE(addr);
}

void NetConnect(int64_t socket, CNetAddr *address) {
  connect(socket, address->ai->ai_addr, address->ai->ai_addrlen);
}

void NetBindIn(int64_t socket, CNetAddr *address) {
  bind(socket, address->ai->ai_addr, address->ai->ai_addrlen);
}
void NetListen(int64_t socket, int64_t max) {
  listen(socket, max);
}
int64_t NetAccept(int64_t socket, CNetAddr **addr) {
  struct sockaddr sa;
  socklen_t       ul  = sizeof(sa);
  int64_t         con = accept(socket, &sa, &ul);
  if (addr)
    *addr = NULL;
  /*	if(addr) {
      *addr=A_CALLOC(sizeof(CNetAddr),NULL);
      addr[0]->sa=sa;
    }*/
  return con;
}
void NetClose(int64_t s) {
#if !(defined(_WIN32) || defined(WIN32))
  // https://stackoverflow.com/questions/48208236/tcp-close-vs-shutdown-in-linux-os
  shutdown(s, SHUT_WR);
  shutdown(s, SHUT_RD);
  close(s);
#else
  closesocket(s);
#endif
}
int64_t NetWrite(int64_t s, char *data, int64_t len) {
#if defined(_WIN32) || defined(WIN32)
  return send(s, data, len, 0);
#else
  return write(s, data, len);
#endif
}
int64_t NetRead(int64_t s, char *data, int64_t len) {
#if defined(_WIN32) || defined(WIN32)
  return recv(s, data, len, 0);
#else
  return read(s, data, len);
#endif
}
static int64_t _PollFor(int64_t _for, int64_t argc, int64_t *argv) {
  struct pollfd poll_for[argc];
  int64_t       idx;
  for (idx = 0; idx != argc; idx++) {
    poll_for[idx].fd      = argv[0];
    poll_for[idx].events  = _for;
    poll_for[idx].revents = 0;
  }
#if defined(_WIN32) || defined(WIN32)
  WSAPoll(poll_for, argc, 0);
#else
  poll(poll_for, argc, 0);
#endif
  for (idx = 0; idx != argc; idx++) {
    if (poll_for[idx].revents & POLLHUP) {
      if (_for & POLLHUP)
        return idx;
      continue;
    }
    if (poll_for[idx].revents & _for)
      return idx;
  }
  return -1;
}
int64_t NetPollForRead(int64_t argc, int64_t *argv) {
  return _PollFor(POLLIN, argc, argv);
}
int64_t NetPollForWrite(int64_t argc, int64_t *argv) {
  return _PollFor(POLLOUT, argc, argv);
}
int64_t NetPollForHangup(int64_t argc, int64_t *argv) {
  return _PollFor(POLLHUP, argc, argv);
}

//
// UDP section
//

//http://matrixsust.blogspot.com/2011/10/udp-server-client-in-c.html
int64_t NetUDPSocketNew() {
#if defined(_WIN32) || defined(WIN32)
  if (!was_init)
    InitWS2();
#endif
  int64_t s = socket(AF_INET, SOCK_DGRAM, 0);
#if defined(_WIN32) || defined(WIN32)
//https://stackoverflow.com/questions/17227092/how-to-make-send-non-blocking-in-winsock
  u_long mode = 1;  // 1 to enable non-blocking socket
  ioctlsocket(s, FIONBIO, &mode);
#endif
#if defined(__FreeBSD__) || defined(__linux__)
  int yes = 1;
  //https://stackoverflow.com/questions/15941005/making-recvfrom-function-non-blocking
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 15000;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
  // On FreeBSD the port will stay in use for awhile after death,so reuse the
  // address
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
  return s;
}

int64_t NetUDPRecvFrom(int64_t s,char *buf,int64_t len,CInAddr **from) {
	CInAddr tmp;
	int alen=sizeof(tmp.sa);
	int64_t r=recvfrom(s,buf,len,0,&tmp.sa,&alen);
	tmp.address=A_STRDUP("something",NULL);
	tmp.port=ntohs(tmp.sa.sin_port);
	if(from) {
		*from=A_CALLOC(sizeof(CInAddr),NULL);
		**from=tmp;
	}
	return r;
}

CInAddr *NetUDPAddrNew(char *host,int64_t port) {
	CInAddr *ret=A_CALLOC(sizeof(CInAddr),NULL);
	struct hostent *hoste=gethostbyname(host);
	ret->sa.sin_family=AF_INET;
	ret->sa.sin_port=htons(port);
	ret->sa.sin_addr=*((struct in_addr *)hoste->h_addr);
	ret->port=port;
	ret->address=A_STRDUP(host,NULL);
	memset(&ret->sa.sin_zero,0,8);
	return ret;
}

int64_t NetUDPSendTo(int64_t s,char *buf,int64_t len,CInAddr *to) {
	sendto(s,buf,len,0,&to->sa,sizeof(to->sa));
}

void NetUDPAddrDel(CInAddr *a) {
	A_FREE(a->address);
	A_FREE(a);
}
