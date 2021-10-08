#ifndef SSL_H
#define SSL_H

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <stdbool.h>
#include <libsock/sock.h>

static SSL_CTX *ctx;

typedef struct tls tls_t;
struct tls
{
  tcp_t *tcp;
  SSL *ssl;
  BIO *rbio, *wbio, *sbio;
  char W[1000000];
  size_t w, n;
};

bool init_tls(const char [], const char []);
void init_clienttls(tls_t *, tcp_t *);
void deinit_clienttls(tls_t *);
void write_sock(tls_t *, const char [], const size_t);
bool read_sock(char [], size_t, tls_t *);
#endif
