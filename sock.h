#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <netinet/in.h>
#include <poll.h>

static const char AGENT[] = "TCPClient";
static const unsigned INTERNAL_TIMEOUTMS = 250;

typedef struct
{
  int sockfd;
  struct pollfd psockfd;
  char host[128];
} Http;

bool init(Http *, const char []);
void deinit(Http *);
bool sendreq(Http *, const char []);
bool performreq(char [], char [], Http *, const char []);
void req(char [], Http *);
void req_header(char [], Http *);
#endif
