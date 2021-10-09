#ifndef SSL_H
#define SSL_H

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <stdbool.h>

static SSL_CTX *ctx;

typedef struct tcp tcp_t;
typedef struct
{
  SSL *ssl;
  BIO *rbio, *wbio, *sbio;
  char W[1000000];
  size_t w, n;
} tls_t;

bool init_tls(const char [], const char []);
void init_clienttls(tls_t *, const int);
void deinit_clienttls(tls_t *);
bool writesock_ssl(tcp_t *, const char []);
bool readsock_ssl(char [], tcp_t *);
#endif
