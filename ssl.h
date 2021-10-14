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
  BIO *r, *w, *s;
} tls_t;

bool init_tls(const char [], const char []);
void init_clienttls(tls_t *, const int);
void deinit_clienttls(tls_t *);
bool write_ssl(tcp_t *, const char []);
void bio_write(tcp_t *, char);
bool read_ssl(char *, tcp_t *);
#endif
