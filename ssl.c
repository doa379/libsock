#include <string.h>
#include <libsock/ssl.h>
#include <unistd.h>

static SSL_CTX *ctx;

bool init_tls(void)
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  return (ctx = SSL_CTX_new(TLS_client_method())); /* mode */
}

bool configure_ctx(const char CERT[], const char KEY[])
{
  return CERT && KEY &&
    SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM) > 0 &&
      SSL_CTX_use_PrivateKey_file(ctx, KEY, SSL_FILETYPE_PEM) > 0 &&
        SSL_CTX_check_private_key(ctx) > 0;
}

void init_client(tls_t *tls, const int sockfd)
{
  tls->r = BIO_new(BIO_s_mem());
  tls->w = BIO_new(BIO_s_mem());
  tls->s = BIO_new_socket(sockfd, BIO_NOCLOSE);
  tls->ssl = SSL_new(ctx);
  /*SSL_set_bio(tls->ssl, tls->r, tls->w); */
  SSL_set_connect_state(tls->ssl); /* client mode */
}

void deinit_client(tls_t *tls)
{
  SSL_free(tls->ssl);
}

bool write_ssl(tls_t *tls, char S[])
{
  SSL_set_bio(tls->ssl, tls->s, tls->s);
  size_t NW = strlen(S);
  return SSL_write(tls->ssl, S, NW) == NW;
}

ssize_t bio_read(tls_t *tls, char S[], const size_t NS)
{
  return BIO_read(tls->w, S, NS);
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
