#define _POSIX_C_SOURCE 200112L

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libsock/sock.h>

void init_poll(Http *http)
{
  http->psockfd.fd = http->sockfd;
  http->psockfd.events = POLLIN;
  http->psockfd.revents = 0;
}

bool init(Http *http, const char host[])
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;
  struct addrinfo *result;
  if (getaddrinfo(host, "http", &hints, &result))
    return false;

  for (struct addrinfo *rp = result; rp; rp = rp->ai_next)
  {
    if ((http->sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) > -1 &&
          connect(http->sockfd, rp->ai_addr, rp->ai_addrlen) > -1)
    {
      init_poll(http);
      strcpy(http->host, host);
      freeaddrinfo(result);
      return true;
    }
    
    deinit(http);
  }

  freeaddrinfo(result);
  return false;
}

void deinit(Http *http)
{
  shutdown(http->sockfd, SHUT_RDWR);
  close(http->sockfd);
}

bool pollin(Http *http, const int timeout_ms)
{
  return poll(&http->psockfd, 1, timeout_ms) > 0 &&
    (http->psockfd.revents & POLLIN);
}

bool rd(char *p, const Http *http)
{
  return read(http->sockfd, p, sizeof *p) > 0;
}

bool wr(Http *http, const char DATA[])
{
  return write(http->sockfd, DATA, strlen(DATA)) > 0;
}

bool sendreq(Http *http, const char ENDP[])
{
  char R[512];
  sprintf(R, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: */*\r\n", 
    ENDP, http->host, AGENT);
  strcat(R, "\r\n");
  return wr(http, R);
}

void req(char R[], Http *http)
{
  char p;
  size_t i = 0;
  while (pollin(http, INTERNAL_TIMEOUTMS) && rd(&p, http))
  {
    memcpy(R + i, &p, sizeof p);
    i++;
  }

  R[i] = '\0';
}

void req_head(char R[], Http *http)
{
  char p;
  size_t i = 0;
  while (!strstr(R, "\r\n\r\n") &&
      pollin(http, INTERNAL_TIMEOUTMS) &&
        rd(&p, http))
  {
    memcpy(R + i, &p, sizeof p);
    i++;
  }

  R[i] = '\0';
}

void req_body(char R[], Http *http, size_t l)
{
  char p;
  size_t i = 0;
  while (i < l && 
    pollin(http, INTERNAL_TIMEOUTMS) && 
      rd(&p, http))
  {
    memcpy(R + i, &p, sizeof p);
    i++;
  }

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
    char L[64];
    unsigned i;
    for (i = 16; cl[i] != '\n'; i++);
    strncpy(L, cl, i);
    char *t = strtok(L, " ");
    t = strtok(NULL, " ");
    return atoi(t);
  }

  return 0;
}

bool performreq(char BODY[], char HEAD[], Http *http, const char ENDP[])
{
  if (sendreq(http, ENDP) && pollin(http, INTERNAL_TIMEOUTMS))
  {
    req_head(HEAD, http);
    size_t len = parse_cl(HEAD);
    req_body(BODY, http, len);
    return true;
  }

  return false;
}
