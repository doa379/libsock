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
} http_t;

bool init(http_t *, const char []);
void deinit(http_t *);
bool sendreq(http_t *, const char []);
bool performreq(char [], char [], http_t *, const char []);
void req(char [], http_t *);
void req_header(char [], http_t *);
#endif
