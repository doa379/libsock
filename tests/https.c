#include <libsock/sock.h>
#include <libsock/ssl.h>
#include <string.h>
#include <unistd.h>

int main(const int argc, const char *argv[])
{
  tcp_t tcp;
  if (!init(&tcp, argv[1], "https"))
  {
    fprintf(stdout, "Unable to init tcp\n");
    return -1;
  }

  init_tls(0, 0);
  tls_t tls;
  init_clienttls(&tls, &tcp);
  char req[256];
  sprintf(req, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", argv[1]);
  write_sock(&tls, req, strlen(req));
  if (read_sock(req, sizeof req, &tls))
  {
    fprintf(stdout, "%s", req);
    fprintf(stdout, "Receive complete\n");
  }
  
  deinit_clienttls(&tls);
  deinit(&tcp);
  return 0;
}
