#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sock.h"

bool init_sd(Http *http)
{
  return (http->sd = socket(AF_INET, SOCK_STREAM, 0) > -1);
}

void deinit_sd(Http *http)
{
  close(http->sd);
}

void init_sa(Http *http, const char host[], const unsigned port)
{
  memset(&http->sa, 0, sizeof http->sa);
  http->sa.sin_family = AF_INET;
  http->sa.sin_port = htons(port);
  struct hostent *h = gethostbyname(host);
  //http->sa.sin_addr.s_addr = h ? *(long *) h->h_addr_list[0] : htonl(INADDR_ANY);
  http->sa.sin_addr = h ? *(long *) (h->h_addr) : htonl(INADDR_ANY);
  strcpy(http->host, host);
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

bool wr(const Http *http, const char data[])
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
  if (init_sd(http))
  {
    init_sa(http, host, port);
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

bool sendreq(const Http *http, const char endp[])
{
  char request[512];
  sprintf(request, "GET %s HTTP/1.1\r\n\
    Host: %s\r\n\
    User-Agent: %s\r\n\
    Accept: */*\r\n", endp, http->host, AGENT);

  strcat(request, "\r\n");
  return wr(http, request);
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
  return strstr(head, "OK");
}

bool req_body(char body[], char head[], Http *http)
{
  char *cl;
  if ((cl = strstr(head, "Content-Length:")) || 
      (cl = strstr(head, "Content Length:")) ||
      (cl = strstr(head, "content-length:")) ||
      (cl = strstr(head, "content length:")))
  {
    char L[64];
    strcpy(L, cl);
    char *t = strtok(L, " ");
    t = strtok(NULL, " ");
    size_t l = atoi(t), i = 0;
    char p;
    while (i < l && po(http, INTERNAL_TIMEOUTMS))
      if (rd(&p, http))
      {
        memcpy(body + i, &p, sizeof p);
        i++;
      }

    return true;
  }

  return false;
}

bool performreq(char body[], char head[], Http *http, const char endp[])
{
  return sendreq(http, endp) && 
    req_header(head, http) &&
      req_body(body, head, http);
}
