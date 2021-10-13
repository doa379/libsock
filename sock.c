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
      tcp->readfilter = readfilter;
      tcp->postread = postread;
      if (!strcmp(PORT, "https") || !strcmp(PORT, "443"))
      {
        init_tls(NULL, NULL); /* anon secure */
        init_clienttls(&tcp->tls, tcp->sockfd);
        tcp->write = write_ssl;
        tcp->readfilter = bio_write;
        tcp->postread = read_ssl;
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

void readfilter(char R[], tcp_t *tcp, char p)
{
  R[tcp->n++] = p;
  R[tcp->n] = '\0';
}

bool postread(char *p, tcp_t *tcp)
{
  return false;
}

bool sendreq(tcp_t *tcp, const char *H[], const unsigned NH, const char ENDP[])
{
  char R[512];
  sprintf(R, 
    "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: */*\r\n", 
      ENDP, tcp->HOST, AGENT);
  for (unsigned i = 0; i < NH; i++)
  {
    strcat(R, H[i]);
    strcat(R, "\r\n");
  }
  
  strcat(R, "\r\n");
  return tcp->write(tcp, R);
}

bool req(char R[], tcp_t *tcp)
{
  char p;
  if (readsock(&p, tcp))
  {
    tcp->readfilter(R, tcp, p);
    while (tcp->postread(&p, tcp))
    {
      R[tcp->n++] = p;
      R[tcp->n] = '\0';
    }
    
    return true;
  }

  return false;
}

size_t parse_cl(const char S[])
{
  char *cl;
  if ((cl = strstr(S, "Content-Length:")) || 
      (cl = strstr(S, "Content Length:")) ||
      (cl = strstr(S, "content-length:")) ||
      (cl = strstr(S, "content length:")))
  {
    char L[16] = { 0 }, *l = L;
    for (unsigned i = 16; cl[i] != '\n'; i++)
      L[i - 16] = cl[i];
    return strtoull(L, &l, 10);
  }

  return 0;
}

bool performreq(char BODY[], char HEAD[], tcp_t *tcp, const char *H[], const unsigned NH, const char ENDP[])
{
  if (sendreq(tcp, H, NH, ENDP))
  {
    tcp->n = 0;
    while (pollin(tcp, INTERNAL_TIMEOUTMS) && 
      req(HEAD, tcp) && 
        !strstr(HEAD, "\r\n\r\n"));
    size_t len = parse_cl(HEAD);
    tcp->n = 0;
    while (pollin(tcp, INTERNAL_TIMEOUTMS) && 
      req(BODY, tcp) &&
        tcp->n < len);
    return true;
  }

  return false;
}
