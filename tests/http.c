#include <libsock/sock.h>
#include <stdio.h>

int main(const int argc, const char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: http <hostname> <endp>\n");
    return -1;
  }

  tcp_t tcp;
  if (init(&tcp, argv[1], "http"))
  {
    fprintf(stdout, "Connected\n");
    char head[256] = { 0 }, body[1024] = { 0 };
    if (performreq(body, head, &tcp, NULL, 0, argv[2]))
    {
      fprintf(stdout, "%s\n", head);
      fprintf(stdout, "%s\n", body);
    }
  
    deinit(&tcp);
  }

  else
    fprintf(stdout, "Failed to init tcp\n");

  return 0;
}
