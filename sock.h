#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <poll.h>
#include <libsock/ssl.h>

static const char AGENT[] = "TCPRequest";
static const unsigned INTERNAL_TIMEOUTMS = 1250;

/*
typedef struct tcp tcp_t;
union proto
{
  char p;
  tls_t tls;
};
*/
struct tcp
{
  int sockfd;
  struct pollfd pollfd;
  char HOST[128];
  char p;
  tls_t tls;
  /* proto_t proto; */
  bool (*write)(tcp_t *, const char []);
  void (*readfilter)(tcp_t *, char);
  bool (*postread)(char *, tcp_t *);
};

bool init(tcp_t *, const char [], const char []);
void deinit(tcp_t *);
bool pollin(tcp_t *, const int);
bool pollout(tcp_t *, const int);
bool writesock(tcp_t *, const char []);
bool readsock(char *, tcp_t *);
void readfilter(tcp_t *, char);
bool postread(char *, tcp_t *);
bool sendreq(tcp_t *, const char *[], const unsigned, const char []);
bool performreq(char [], char [], tcp_t *, const char *[], const unsigned, const char []);
#endif
