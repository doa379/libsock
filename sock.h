#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <poll.h>
#include <libsock/ssl.h>

typedef union
{
  char p;
  tls_t tls;
} proto_t;

typedef struct tcp tcp_t;
struct tcp
{
  int sockfd;
  struct pollfd pollfd;
  char HOST[128];
  proto_t proto;
  bool (*write)(tcp_t *, const char []);
  void (*readfilter)(proto_t *, char);
  bool (*postread)(char *, proto_t *);
};

bool init(tcp_t *, const char [], const char []);
void deinit(tcp_t *);
bool pollin(tcp_t *, const int);
bool pollout(tcp_t *, const int);
bool readsock(char *, tcp_t *);
bool writesock(tcp_t *, const char []);
bool writesock_ssl(tcp_t *, const char []);
void readfilter(proto_t *, char);
void readfilter_ssl(proto_t *, char);
bool postread(char *, proto_t *);
bool postread_ssl(char *, proto_t *);
bool sendreq(tcp_t *, const char *[], const unsigned, const char []);
bool performreq(char [], char [], tcp_t *, const char *[], const unsigned, const char []);
#endif
