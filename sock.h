#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <netinet/in.h>
#include <poll.h>

static const char AGENT[] = "HttpClient";
static const unsigned INTERNAL_TIMEOUTMS = 250;

typedef struct
{
  int sd;
  struct sockaddr_in sa;
  struct pollfd psd;
  char host[128];
} Http;

bool init(Http *, const char [], const unsigned);
void deinit(Http *);
bool conn(const Http *);
bool performreq(char [], char [], Http *, const char []);
#endif
