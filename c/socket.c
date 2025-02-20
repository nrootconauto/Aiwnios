#include "aiwn_mem.h"
#include "aiwn_sock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _BSD_SOURCE
#ifndef _WIN64
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <poll.h>
#  include <signal.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
static void InitSock() {
  // These are not my freind
  signal(SIGPIPE, SIG_IGN);
}
#else
#  define _WIN32_WINNT 0x603
#  include <winsock2.h>
#  include <windows.h>
#  include <iphlpapi.h>
#  include <ws2tcpip.h>
static int64_t was_init = 0;
WSADATA ws_data;

static void InitWS2() {
  WSAStartup(MAKEWORD(2, 2), &ws_data);
  was_init = 1;
}
#endif
typedef struct CInAddr {
  char *address;
  int64_t port;
  struct sockaddr_in sa;
} CInAddr;

typedef struct CNetAddr {
  struct addrinfo *ai;
} CNetAddr;
int64_t NetIP4ByHost(char *host) {
  CNetAddr *addr = NetAddrNew(host, 0, 4);
  char *a = inet_ntoa(((struct sockaddr_in *)addr->ai->ai_addr)->sin_addr);
  NetAddrDel(addr);
  return inet_addr(a);
}
// ipv is either 4 or 6
int64_t NetSocketNew(int64_t ipv) {
#ifdef _WIN32
  if (!was_init)
    InitWS2();
#else
  InitSock();
#endif
  int64_t fam;
  if (ipv == 4)
    fam = AF_INET;
  else
    fam = AF_INET6;
  int64_t s = socket(fam, SOCK_STREAM, IPPROTO_TCP);
#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
  int yes = 1;
  // On FreeBSD the port will stay in use for awhile after death,so reuse the
  // address
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
  return s;
}

int64_t BigEndianIPAddrByHostname(CNetAddr *addr) {
  int64_t ret = -1;
}

// ipv is 4 or 6
CNetAddr *NetAddrNew(char *host, int64_t port, int64_t ipv) {
#ifdef _WIN32
  if (!was_init)
    InitWS2();
#else
  InitSock();
#endif
  int64_t fam;
  if (ipv == 4)
    fam = AF_INET;
  else
    fam = AF_INET6;
  CNetAddr *ret = A_CALLOC(sizeof(CNetAddr), NULL);
  char buf[1024];
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = fam;
  hints.ai_socktype = 0;
  hints.ai_flags = AI_PASSIVE;
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
  socklen_t ul = sizeof(sa);
  int64_t con = accept(socket, &sa, &ul);
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
  int64_t idx;
  for (idx = 0; idx != argc; idx++) {
    poll_for[idx].fd = argv[0];
    poll_for[idx].events = _for;
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

// http://matrixsust.blogspot.com/2011/10/udp-server-client-in-c.html
int64_t NetUDPSocketNew(int64_t ipv) {
#if defined(_WIN32) || defined(WIN32)
  if (!was_init)
    InitWS2();
#else
  InitSock();
#endif
  int64_t fam;
  if (ipv == 4)
    fam = AF_INET;
  if (ipv == 6)
    fam = AF_INET6;
  int64_t s = socket(fam, SOCK_DGRAM, 0);
#if defined(_WIN32) || defined(WIN32)
  // https://stackoverflow.com/questions/17227092/how-to-make-send-non-blocking-in-winsock
  u_long mode = 1; // 1 to enable non-blocking socket
  ioctlsocket(s, FIONBIO, &mode);
#endif
#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
  int yes = 1;
  // https://stackoverflow.com/questions/15941005/making-recvfrom-function-non-blocking
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 3000;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
  // On FreeBSD the port will stay in use for awhile after death,so reuse the
  // address
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif
  return s;
}

int64_t NetUDPRecvFrom(int64_t s, char *buf, int64_t len, CInAddr **from) {
  CInAddr tmp;
  int alen = sizeof(tmp.sa);
  int64_t r = recvfrom(s, buf, len, 0, &tmp.sa, &alen);
#if defined(WIN32) || defined(_WIN32)
  char buf2[2048];
  inet_ntop(tmp.sa.sin_family, &tmp.sa.sin_addr, buf2, 2048);
  tmp.address = A_STRDUP(buf2, NULL);
#else
  tmp.address = A_STRDUP(inet_ntoa(tmp.sa.sin_addr), NULL);
#endif
  tmp.port = ntohs(tmp.sa.sin_port);
  if (from) {
    *from = A_CALLOC(sizeof(CInAddr), NULL);
    **from = tmp;
  } else
    A_FREE(tmp.address);
  return r;
}

CInAddr *NetUDPAddrNew(char *host, int64_t port, int64_t ipv) {
#if defined(_WIN32) || defined(WIN32)
  if (!was_init)
    InitWS2();
#else
  InitSock();
#endif
  int64_t fam;
  if (ipv == 4)
    fam = AF_INET;
  if (ipv == 6)
    fam = AF_INET6;
  CInAddr *ret = A_CALLOC(sizeof(CInAddr), NULL);
  struct hostent *hoste = gethostbyname(host);
  ret->sa.sin_family = fam;
  ret->sa.sin_port = htons(port);
  ret->sa.sin_addr = *((struct in_addr *)hoste->h_addr);
  ret->port = port;
  ret->address = A_STRDUP(host, NULL);
  memset(&ret->sa.sin_zero, 0, 8);
  return ret;
}

int64_t NetUDPSendTo(int64_t s, char *buf, int64_t len, CInAddr *to) {
  sendto(s, buf, len, 0, &to->sa, sizeof(to->sa));
}

void NetUDPAddrDel(CInAddr *a) {
  A_FREE(a->address);
  A_FREE(a);
}
