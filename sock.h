/*****************************************************************************
MIT License

Copyright (c) 2021-22 doa379

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef SOCK_H
#define SOCK_H

#include <stdbool.h>
#include <poll.h>
#include <libsock/ssl.h>

typedef union {
  char p;
  tls_t tls;
} proto_t;

typedef struct tcp tcp_t;
struct tcp {
  int sockfd;
  struct pollfd pollfd;
  char HOST[128];
  proto_t proto;
  bool (*write)(tcp_t *, char [], const size_t);
  void (*readfilter)(proto_t *, char);
  bool (*postread)(char *, proto_t *);
};

bool init(tcp_t *, const char [], const char []);
void deinit(tcp_t *);
bool readsock(char *, tcp_t *);
bool writesock(tcp_t *, char [], const size_t);
bool writesock_ssl(tcp_t *, char [], const size_t);
void readfilter(proto_t *, char);
void readfilter_ssl(proto_t *, char);
bool postread(char *, proto_t *);
bool postread_ssl(char *, proto_t *);
bool performreq(char [], char [], tcp_t *, const char *[], const unsigned, const char []);
#endif
