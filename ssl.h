#ifndef SSL_H
#define SSL_H

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <stdbool.h>

typedef struct
{
  SSL *ssl;
  BIO *r, *w, *s;
} tls_t;

bool init_tls(const char [], const char []);
void init_clienttls(tls_t *, const int);
void deinit_clienttls(tls_t *);
bool write_ssl(tls_t *, const char []);
void bio_write(tls_t *, char);
bool read_ssl(char *, tls_t *);
#endif
