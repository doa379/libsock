#include <openssl/err.h>
#include <openssl/pem.h>
#include <string.h>
#include <libsock/ssl.h>

static SSL_CTX *ctx;

bool init_tls(const char CERT[], const char KEY[])
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  if (!(ctx = SSL_CTX_new(TLS_client_method()))) /* mode */
    return false;
  return (!(CERT && KEY &&
    (SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM) < 1 ||
      SSL_CTX_use_PrivateKey_file(ctx, KEY, SSL_FILETYPE_PEM) < 1 ||
        SSL_CTX_check_private_key(ctx) < 1)));
}

void init_clienttls(tls_t *tls, const int sockfd)
{
  memset(tls, 0, sizeof *tls);
  tls->ssl = SSL_new(ctx);
  tls->r = BIO_new(BIO_s_mem());
  tls->w = BIO_new(BIO_s_mem());
  tls->s = BIO_new_socket(sockfd, BIO_NOCLOSE);
  SSL_set_connect_state(tls->ssl); /* ssl client mode */
}

void deinit_clienttls(tls_t *tls)
{
  SSL_free(tls->ssl);
}

bool write_ssl(tls_t *tls, const char S[])
{
  SSL_set_bio(tls->ssl, tls->s, tls->s);
  const size_t NS = strlen(S);
  return SSL_write(tls->ssl, S, NS) == NS;
}

void bio_write(tls_t *tls, char p)
{
  BIO_write(tls->r, &p, sizeof p);
}

bool read_ssl(char *p, tls_t *tls)
{
  SSL_set_bio(tls->ssl, tls->r, tls->r);
  return SSL_read(tls->ssl, p, sizeof *p) > 0;
}
