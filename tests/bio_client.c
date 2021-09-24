#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

int main(const int argc, const char *argv[])
{
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  OpenSSL_add_all_algorithms();
  SSL_library_init();
  BIO *bio;
  char host[128];
  sprintf(host, "%s:%d", argv[1], atoi(argv[2]));
  bio = BIO_new_connect(host);
  if (!bio)
  {
    fprintf(stdout, "Failed to init\n");
    return -1;
  }

  else if (BIO_do_connect(bio) < 1)
  {
    fprintf(stdout, "Failed to connect\n");
    return -1;
  }

  char req[1024];
  sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", argv[3], argv[1]);
  if (BIO_write(bio, req, strlen(req)) < 1)
  {
    fprintf(stdout, "Failed to send request\n");
    BIO_free_all(bio);
    return -1;
  }

  char buf[1024];
  memset(buf, 0, sizeof buf);
  int err = BIO_read(bio, buf, sizeof buf);
  if (err == 0)
  {
    fprintf(stdout, "Connection closed\n");
  }

  else if(err < 0 )
  {
    fprintf(stdout, "Connection error\n");
    if (!BIO_should_retry(bio))
      fprintf(stdout, "Retry error\n");
  }

  else
    fprintf(stdout, "%s\n" , buf);

  BIO_free_all(bio);
  return 0;
}

