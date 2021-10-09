#include <libsock/sock.h>
#include <string.h>

int main(const int argc, const char *argv[])
{
  tcp_t tcp;
  if (!init(&tcp, argv[1], "https"))
  {
    fprintf(stdout, "Unable to init tcp\n");
    return -1;
  }

  char REQ[16384];
  sprintf(REQ, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", argv[1]);
  if (writesock_ssl(&tcp, REQ) && readsock_ssl(REQ, &tcp))
  {
    fprintf(stdout, "%s\n", REQ);
    fprintf(stdout, "Receive complete\n");
  }
  
  deinit(&tcp);
  return 0;
}
