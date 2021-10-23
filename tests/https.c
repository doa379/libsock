#include <libsock/sock.h>
#include <string.h>

int main(const int ARGC, const char *ARGV[])
{
  tcp_t tcp;
  if (!init(&tcp, ARGV[1], "https"))
  {
    fprintf(stdout, "Unable to init tcp\n");
    return -1;
  }

  const char *HEADER[1] = { "Connection: close" };
  char HEAD[32768], BODY[32768];
  if (performreq(BODY, HEAD, &tcp, HEADER, 0, "/"))
  {
    fprintf(stdout, "HEAD:\n%s\n", HEAD);
    fprintf(stdout, "BODY:\n%s\n", BODY);
    fprintf(stdout, "Receive complete\n");
  }

  deinit(&tcp);
  return 0;
}
