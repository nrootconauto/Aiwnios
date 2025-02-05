#pragma once
struct CNetAddr;
#include <stdint.h>
extern int64_t NetPollForHangup(int64_t argc, int64_t *argv);
extern int64_t NetPollForWrite(int64_t argc, int64_t *argv);
extern int64_t NetPollForRead(int64_t argc, int64_t *argv);
extern int64_t NetWrite(int64_t s, char *data, int64_t len);
extern int64_t NetRead(int64_t s, char *data, int64_t len);
extern void NetClose(int64_t s);
extern int64_t NetAccept(int64_t socket, struct CNetAddr **addr);
extern void NetListen(int64_t socket, int64_t max);
extern void NetBindIn(int64_t socket, struct CNetAddr *);
extern void NetConnect(int64_t socket, struct CNetAddr *);
// ipv is 4 or 6
extern int64_t NetSocketNew(int64_t ipv);
// ipv is 4 or 6
extern struct CNetAddr *NetAddrNew(char *host, int64_t port, int64_t ipv);
extern void NetAddrDel(struct CNetAddr *);
struct CInAddr;
extern int64_t NetUDPSendTo(int64_t s, char *buf, int64_t len,
                            struct CInAddr *to);
extern int64_t NetUDPRecvFrom(int64_t s, char *buf, int64_t len,
                              struct CInAddr **from);
extern int64_t NetUDPSocketNew(int64_t ipv);
extern struct CInAddr *NetUDPAddrNew(char *host, int64_t port, int64_t ipv);
extern void NetUDPAddrDel(struct CInAddr *);
extern int64_t NetIP4ByHost(char *host);
