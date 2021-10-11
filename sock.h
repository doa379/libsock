#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <poll.h>
#include <libsock/ssl.h>

static const char AGENT[] = "TCPRequest";
static const unsigned INTERNAL_TIMEOUTMS = 250;

struct tcp
{
  int sockfd;
  struct pollfd pollfd;
  char HOST[128];
  tls_t tls;
  bool (*write)(tcp_t *, const char []);
  void (*readfilter)(char [], tcp_t *);
  size_t n;
  bool ssl;
};

bool init(tcp_t *, const char [], const char []);
void deinit(tcp_t *);
bool pollin(tcp_t *, const int);
bool pollout(tcp_t *, const int);
bool writesock(tcp_t *, const char []);
bool readsock(char *, tcp_t *);
void readfilter(char [], tcp_t *);
bool sendreq(tcp_t *, const char *[], const unsigned, const char []);
bool performreq(char [], char [], tcp_t *, const char *[], const unsigned, const char []);
void req(char [], tcp_t *);
void req_header(char [], tcp_t *);
#endif
