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

void init_tls(void);
bool configure_ctx(const char [], const char []);
void init_client(tls_t *, const int);
void deinit_client(tls_t *);
bool write_ssl(tls_t *, char []);
ssize_t bio_read(tls_t *tls, char S[], const size_t);
void bio_write(tls_t *, char);
bool read_ssl(char *, tls_t *);
#endif
