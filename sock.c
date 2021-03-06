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


#define _POSIX_C_SOURCE 200112L

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libsock/sock.h>

static const char AGENT[] = "TCPRequest";
static const unsigned INTERNAL_TIMEOUTMS = 100;
static const unsigned ENCBUFFER_SIZE = 16384;

void init_poll(tcp_t *tcp) {
  tcp->pollfd.fd = tcp->sockfd;
}

bool init(tcp_t *tcp, const char HOST[], const char PORT[]) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  struct addrinfo *result;
  if (getaddrinfo(HOST, PORT, &hints, &result))
    return false;

  for (struct addrinfo *rp = result; rp; rp = rp->ai_next) {
    memset(tcp, 0, sizeof *tcp);
    if ((tcp->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) > -1 &&
      connect(tcp->sockfd, rp->ai_addr, rp->ai_addrlen) > -1) {
      init_poll(tcp);
      strcpy(tcp->HOST, HOST);
      tcp->write = writesock;
      tcp->readfilter = readfilter;
      tcp->postread = postread;
      if (!strcmp(PORT, "https") || strstr(PORT, "443")) {
        init_tls();
        init_client(&tcp->proto.tls, tcp->sockfd);
        tcp->write = writesock_ssl;
        tcp->readfilter = readfilter_ssl;
        tcp->postread = postread_ssl;
      }

      freeaddrinfo(result);
      return true;
    }
    
    deinit(tcp);
  }

  freeaddrinfo(result);
  return false;
}

void deinit(tcp_t *tcp) {
  if (tcp->proto.tls.ssl)
    deinit_client(&tcp->proto.tls);
  if (close(tcp->sockfd) > -1)
    tcp->sockfd = -1;
}

bool pollin(tcp_t *tcp, const int timeout_ms) {
  tcp->pollfd.events = POLLIN;
  tcp->pollfd.revents = 0;
  return poll(&tcp->pollfd, 1, timeout_ms) > 0 &&
    (tcp->pollfd.revents & POLLIN);
}

bool pollout(tcp_t *tcp, const int timeout_ms) {
  tcp->pollfd.events = POLLOUT;
  tcp->pollfd.revents = 0;
  return poll(&tcp->pollfd, 1, timeout_ms) > 0 &&
    (tcp->pollfd.revents & POLLOUT);
}

bool pollerr(tcp_t *tcp, const int timeout_ms) {
  short err = POLLERR | POLLHUP | POLLNVAL;
  tcp->pollfd.events = err;
  tcp->pollfd.revents = 0;
  return poll(&tcp->pollfd, 1, timeout_ms) > 0 &&
    (tcp->pollfd.revents & err);
}

bool readsock(char *p, tcp_t *tcp) {
  return read(tcp->sockfd, p, sizeof *p) > 0;
}

bool writesock(tcp_t *tcp, char S[], const size_t NS) {
  return write(tcp->sockfd, S, NS) == NS;
}

bool writesock_ssl(tcp_t *tcp, char S[], const size_t NS) {
  tls_t *tls = &tcp->proto.tls;
  if (write_ssl(tls, S, NS)) {
    ssize_t Nenc = bio_read(tls, S, ENCBUFFER_SIZE);
    if (Nenc > 0)
      S[Nenc] = '\0';
    return Nenc > 0 && writesock(tcp, S, Nenc);
  }
  
  return false;
}

void readfilter(proto_t *proto, char p) {
  proto->p = p;
}

void readfilter_ssl(proto_t *proto, char p) {
  tls_t *tls = &proto->tls;
  bio_write(tls, p);
}

bool postread(char *p, proto_t *proto) {
  if (proto->p) {
    *p = proto->p;
    proto->p = '\0';
    return true;
  }

  return false;
}

bool postread_ssl(char *p, proto_t *proto) {
  return read_ssl(p, &proto->tls);
}

bool req_head(char R[], tcp_t *tcp) {
  char p;
  size_t i = 0;
  while (pollin(tcp, INTERNAL_TIMEOUTMS) && readsock(&p, tcp)) {
    tcp->readfilter(&tcp->proto, p);
    while (tcp->postread(&p, &tcp->proto)) {
      R[i++] = p;
      if (i > 3 && R[i - 3] == '\n' && R[i - 2] == '\r' && R[i - 1] == '\n') {
        R[i] = '\0';
        return true;
      }
    }
  }

  return false;
}

bool req_body(char R[], tcp_t *tcp, size_t l) {
  char p;
  size_t i = 0;
  // Pick up any slack from current SSL frame
  while (i < l && tcp->postread(&p, &tcp->proto))
    R[i++] = p;

  if (i > l - 1) {
    R[l] = '\0';
    return true;
  }

  while (pollin(tcp, INTERNAL_TIMEOUTMS) && readsock(&p, tcp)) {
    tcp->readfilter(&tcp->proto, p);
    while (tcp->postread(&p, &tcp->proto)) {
      R[i++] = p;
      if (i > l - 1) {
        R[l] = '\0';
        return true;
      }
    }
  }

  return false;
}

size_t parse_cl(const char S[]) {
  char *cl;
  if ((cl = strstr(S, "Content-Length:")) || 
      (cl = strstr(S, "Content Length:")) ||
      (cl = strstr(S, "content-length:")) ||
      (cl = strstr(S, "content length:"))) {
    char L[16] = { 0 }, *l = L;
    for (unsigned i = 16; cl[i] != '\n'; i++)
      L[i - 16] = cl[i];

    return strtoull(L, &l, 10);
  }

  return 0;
}

bool sendreq(tcp_t *tcp, const char *H[], const unsigned NH, const char ENDP[]) {
  char R[ENCBUFFER_SIZE];
  sprintf(R, 
    "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: */*\r\n", 
      ENDP, tcp->HOST, AGENT);
  for (unsigned i = 0; i < NH; i++) {
    strcat(R, H[i]);
    strcat(R, "\r\n");
  }
  
  strcat(R, "\r\n");
  return tcp->write && tcp->write(tcp, R, strlen(R));
}

bool performreq(char BODY[], char HEAD[], tcp_t *tcp, const char *H[], const unsigned NH, const char ENDP[]) {
  size_t cl;
  return sendreq(tcp, H, NH, ENDP) && 
    req_head(HEAD, tcp) && (cl = parse_cl(HEAD)) &&
      req_body(BODY, tcp, cl);
}
