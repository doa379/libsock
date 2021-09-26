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

  Http http;
  if (init(&http, argv[1], 80))
  {
    char head[256], body[1024];
    for (unsigned i = 0; i < 10; i++)
    {
      if (sendreq(&http, argv[2]))
      {
        req_header(head, &http);
        req(body, &http);
        fprintf(stdout, "%s\n", body);
      }

      sleep(1);
    }

    deinit(&http);
  }

  return 0;
}
