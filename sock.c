#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sock.h"

bool init_sd(Http *http)
{
  return (http->sd = socket(AF_INET, SOCK_STREAM, 0)) > -1;
}

void deinit_sd(Http *http)
{
  close(http->sd);
}

bool init_sa(Http *http, const char host[], const unsigned port)
{
  memset(&http->sa, 0, sizeof http->sa);
  http->sa.sin_family = AF_INET;
  http->sa.sin_port = htons(port);
  struct addrinfo hints, *servinfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(host, "http", &hints, &servinfo) != 0)
    return false;

  char ip[16]; 
  for(struct addrinfo *p = servinfo; p; p = p->ai_next)
  {
    struct sockaddr_in *h = (struct sockaddr_in *) p->ai_addr;
    strcpy(ip, inet_ntoa(h->sin_addr) );
  }

  freeaddrinfo(servinfo);
  http->sa.sin_addr.s_addr = inet_addr(ip);
  strcpy(http->host, host);
  return true;
}

void init_psd(Http *http)
{
  http->psd.fd = http->sd;
  http->psd.events = POLLIN;
}

bool conn(const Http *http)
{
  return connect(http->sd, (struct sockaddr *) &http->sa, sizeof http->sa) > -1;
}

bool rd(char *p, const Http *http)
{
  return read(http->sd, p, sizeof *p) > 0;
}

bool wr(Http *http, const char data[])
{
  return write(http->sd, data, strlen(data)) > 0;
}

bool po(Http *http, const int timeout_ms)
{
  return poll(&http->psd, 1, timeout_ms) > 0 &&
    (http->psd.revents & POLLIN);
}

bool init(Http *http, const char host[], const unsigned port)
{
  if (init_sd(http) && init_sa(http, host, port))
  {
    init_psd(http);
    if (conn(http))
      return true;
  }

  return false;
}

void deinit(Http *http)
{
  deinit_sd(http);
}

bool sendreq(Http *http, const char endp[])
{
  char request[512];
  sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: */*\r\n", 
    endp, http->host, AGENT);
  strcat(request, "\r\n");
  return wr(http, request);
}

void req(char req[], Http *http)
{
  char p;
  size_t i = 0;
  while (po(http, INTERNAL_TIMEOUTMS))
    if (rd(&p, http))
    {
      memcpy(req + i, &p, sizeof p);
      i++;
    }

  req[i] = '\0';
}

bool req_header(char head[], Http *http)
{
  char p;
  size_t i = 0;
  while (!strstr(head, "\r\n\r\n") && po(http, INTERNAL_TIMEOUTMS))
    if (rd(&p, http))
    {
      memcpy(head + i, &p, sizeof p);
      i++;
    }

  head[i] = '\0';
  return strstr(head, "OK");
}

void req_body(char body[], Http *http)
{
  req(body, http);
}

void performreq(char body[], char head[], Http *http, const char endp[])
{
  if (sendreq(http, endp) && req_header(head, http))
    req_body(body, http);
}
