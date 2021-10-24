#include <openssl/err.h>
#include <openssl/pem.h>
#include <string.h>
#include <libsock/ssl.h>
#include <unistd.h>

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
  return CERT && KEY &&
    SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM) > 0 &&
      SSL_CTX_use_PrivateKey_file(ctx, KEY, SSL_FILETYPE_PEM) > 0 &&
        SSL_CTX_check_private_key(ctx) > 0;
}

void handshake(tls_t *tls, const int sockfd)
{
  char buffer[4096] = { 0 };
  ssize_t NRW;
  while (!SSL_is_init_finished(tls->ssl))
  {
    SSL_do_handshake(tls->ssl);
    if ((NRW = BIO_read(tls->w, buffer, sizeof buffer)) > 0)
      write(sockfd, buffer, NRW);
    else if ((NRW = read(sockfd, buffer, sizeof buffer)) > 0)
      BIO_write(tls->r, buffer, NRW);
  }
}

void init_clienttls(tls_t *tls, const int sockfd)
{
  memset(tls, 0, sizeof *tls);
  tls->r = BIO_new(BIO_s_mem());
  tls->w = BIO_new(BIO_s_mem());
  tls->s = BIO_new_socket(sockfd, BIO_NOCLOSE);
  tls->ssl = SSL_new(ctx);
  //SSL_set_bio(tls->ssl, tls->r, tls->w);
  SSL_set_connect_state(tls->ssl); /* client mode */
  //handshake(tls, sockfd);
  //SSL_connect(tls->ssl);
}

void deinit_clienttls(tls_t *tls)
{
  SSL_free(tls->ssl);
}

ssize_t write_ssl(tls_t *tls, char S[], const size_t NS)
{
  SSL_set_bio(tls->ssl, tls->s, tls->s);
  return SSL_write(tls->ssl, S, strlen(S));
  //BIO_do_handshake(tls->w);
  //return BIO_read(tls->w, S, NS);
}

void bio_write(tls_t *tls, char p)
{
  //BIO_do_handshake(tls->r);
  BIO_write(tls->r, &p, sizeof p);
}

bool read_ssl(char *p, tls_t *tls)
{ 
  SSL_set_bio(tls->ssl, tls->r, tls->r);
  return SSL_read(tls->ssl, p, sizeof *p) > 0;
}
