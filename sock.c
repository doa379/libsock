#define _POSIX_C_SOURCE 200112L

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libsock/sock.h>

void init_poll(tcp_t *tcp)
{
  tcp->pollfd.fd = tcp->sockfd;
  tcp->pollfd.events = POLLIN;
  tcp->pollfd.revents = 0;
}

bool init(tcp_t *tcp, const char HOST[], const char PORT[])
{
  memset(tcp, 0, sizeof *tcp);
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  struct addrinfo *result;
  if (getaddrinfo(HOST, PORT, &hints, &result))
    return false;

  for (struct addrinfo *rp = result; rp; rp = rp->ai_next)
  {
    if ((tcp->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) > -1 &&
          connect(tcp->sockfd, rp->ai_addr, rp->ai_addrlen) > -1)
    {
      init_poll(tcp);
      strcpy(tcp->HOST, HOST);
      tcp->write = writesock;
      tcp->read = readsock;
      if (!strcmp("https", PORT) || !strcmp("443", PORT))
      {
        init_tls(0, 0);
        init_clienttls(&tcp->tls, tcp->sockfd);
        tcp->write = writesock_ssl;
        tcp->read = readsock_ssl;
        tcp->ssl = 1;
      }

      freeaddrinfo(result);
      return true;
    }
    
    deinit(tcp);
  }

  freeaddrinfo(result);
  return false;
}

void deinit(tcp_t *tcp)
{
  if (tcp->ssl)
    deinit_clienttls(&tcp->tls);
  if (close(tcp->sockfd) > -1)
    tcp->sockfd = -1;
}

bool pollin(tcp_t *tcp, const int timeout_ms)
{
  return poll(&tcp->pollfd, 1, timeout_ms) > 0 &&
    (tcp->pollfd.revents & POLLIN);
}

bool pollout(tcp_t *tcp, const int timeout_ms)
{
  return poll(&tcp->pollfd, 1, timeout_ms) > 0 &&
    (tcp->pollfd.revents & POLLOUT);
}

bool writesock(tcp_t *tcp, const char S[])
{
  size_t s = strlen(S);
  return write(tcp->sockfd, S, s) == s;
}

bool readsock(char *p, tcp_t *tcp)
{
  return read(tcp->sockfd, p, sizeof *p) > 0;
}

bool sendreq(tcp_t *tcp, const char ENDP[])
{
  char R[512];
  sprintf(R, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: */*\r\n", 
    ENDP, tcp->HOST, AGENT);
  strcat(R, "\r\n");
  return tcp->write(tcp, R);
}

void req(char R[], tcp_t *tcp)
{
  char p;
  size_t i = 0;
  while (pollin(tcp, INTERNAL_TIMEOUTMS) && tcp->read(&p, tcp))
    R[i++] = p;

  R[i] = '\0';
}

void req_head(char R[], tcp_t *tcp)
{
  char p;
  size_t i = 0;
  while (!strstr(R, "\r\n\r\n") &&
      pollin(tcp, INTERNAL_TIMEOUTMS) &&
        tcp->read(&p, tcp))
    R[i++] = p;

  R[i] = '\0';
}

void req_body(char R[], tcp_t *tcp, size_t l)
{
  char p;
  size_t i = 0;
  while (i < l && 
    pollin(tcp, INTERNAL_TIMEOUTMS) && 
      tcp->read(&p, tcp))
    R[i++] = p;

  R[i] = '\0';
}

size_t parse_cl(const char R[])
{
  char *cl;
  if ((cl = strstr(R, "Content-Length:")) || 
      (cl = strstr(R, "Content Length:")) ||
      (cl = strstr(R, "content-length:")) ||
      (cl = strstr(R, "content length:")))
  {
    char L[64], *l = L;
    unsigned i;
    for (i = 16; cl[i] != '\n'; i++);
    strncpy(L, cl + 16, i);
    return strtoull(l, &l, 10);
  }

  return 0;
}

bool performreq(char BODY[], char HEAD[], tcp_t *tcp, const char ENDP[])
{
  if (sendreq(tcp, ENDP) && pollin(tcp, INTERNAL_TIMEOUTMS))
  {
    req_head(HEAD, tcp);
    size_t len = parse_cl(HEAD);
    req_body(BODY, tcp, len);
    return true;
  }

  return false;
}
