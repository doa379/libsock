#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <poll.h>

static const char AGENT[] = "TCPRequest";
static const unsigned INTERNAL_TIMEOUTMS = 250;

typedef struct
{
  int sockfd;
  struct pollfd pollfd;
  char host[128];
} tcp_t;

bool init(tcp_t *, const char [], const char []);
void deinit(tcp_t *);
bool pollin(tcp_t *, const int);
bool pollout(tcp_t *, const int);
bool readsock(char *, const tcp_t *);
bool sendreq(tcp_t *, const char []);
bool performreq(char [], char [], tcp_t *, const char []);
void req(char [], tcp_t *);
void req_header(char [], tcp_t *);
#endif
