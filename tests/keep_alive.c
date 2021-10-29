#include <libsock/sock.h>
#include <stdio.h>
#include <unistd.h>

int main(const int ARGC, const char *ARGV[])
{
  if (ARGC != 4)
  {
    fprintf(stderr, "Usage: keep_alive <http|https> <hostname> <endp>\n");
    return -1;
  }

  tcp_t tcp;
  if (init(&tcp, ARGV[2], ARGV[1]))
    fprintf(stdout, "Connected\n");
  else
    return -1;
  for (unsigned i = 0; i < 100; i++)
  {
    fprintf(stdout, "-----\nPASS %d\n-----\n", i);
    char HEAD[256] = { 0 }, BODY[1024] = { 0 };
    if (performreq(BODY, HEAD, &tcp, NULL, 0, ARGV[3]))
    {
      /* fprintf(stdout, "%s\n", HEAD); */
      fprintf(stdout, "%s\n", BODY);
    }
    else
      fprintf(stdout, "Unable to performreq()\n");
    sleep(1);
  }

  deinit(&tcp);
  return 0;
}
