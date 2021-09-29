#include <libsock/sock.h>
#include <stdio.h>
#include <unistd.h>

int main(const int argc, const char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: http <hostname> <endp>\n");
    return -1;
  }

  http_t http;
  if (init(&http, argv[1]))
    fprintf(stdout, "Connected\n");
  else
    return -1;
  for (unsigned i = 0; i < 100; i++)
  {
    char head[256] = { 0 }, body[1024] = { 0 };
    if (performreq(body, head, &http, argv[2]))
      fprintf(stdout, "%s\n", body);
    else
      fprintf(stdout, "Unable to performreq()\n");
    sleep(1);
  }

  deinit(&http);
  return 0;
}
