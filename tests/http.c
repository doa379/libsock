#include <libsock/sock.h>
#include <stdio.h>

int main(const int argc, const char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: http <hostname> <endp>\n");
    return -1;
  }

  Http http;
  if (init(&http, argv[1], 80))
  {
    fprintf(stdout, "Connected\n");
    char head[256], body[1024];
    performreq(body, head, &http, argv[2]);
    fprintf(stdout, "%s\n", head);
    fprintf(stdout, "%s\n", body);

/*
    if (sendreq(&http, argv[2]))
    {
      char resp[1024];
      req(resp, &http);
      fprintf(stdout, "%s\n", resp);
    }
*/

    deinit(&http);
  }

  return 0;
}
