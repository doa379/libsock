#include <openssl/err.h>
#include <openssl/pem.h>
#include <string.h>
#include <unistd.h>
#include <libsock/ssl.h>

bool init_tls(const char CERT[], const char KEY[])
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  if (!(ctx = SSL_CTX_new(TLS_client_method()))) /* mode */
    return false;

  if (CERT && KEY &&
    (SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM) != 1 ||
      SSL_CTX_use_PrivateKey_file(ctx, KEY, SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_check_private_key(ctx) != 1))
      return false;

  return true;
}

void init_clienttls(tls_t *tls, tcp_t *tcp)
{
  memset(tls, 0, sizeof *tls);
  tls->tcp = tcp;
  tls->ssl = SSL_new(ctx);
  tls->rbio = BIO_new(BIO_s_mem());
  tls->wbio = BIO_new(BIO_s_mem());
  tls->sbio = BIO_new_socket(tcp->sockfd, BIO_NOCLOSE);
  SSL_set_connect_state(tls->ssl); /* ssl client mode */
}

void deinit_clienttls(tls_t *tls)
{
  SSL_free(tls->ssl);
}

void write_sock(tls_t *tls, const char req[], const size_t l)
{
  SSL_set_bio(tls->ssl, tls->sbio, tls->sbio);
  SSL_write(tls->ssl, req, l);
}

bool read_sock(char R[], size_t r, tls_t *tls)
{
  SSL_set_bio(tls->ssl, tls->rbio, tls->rbio);
  char p;
  while (pollin(tls->tcp, 100) && readsock(&p, tls->tcp))
  {
    tls->W[tls->w] = p;
    tls->W[tls->w + 1] = '\0';
    int n = BIO_write(tls->rbio, tls->W + tls->w++, sizeof p);
    tls->n += n;
    if (n && SSL_get_error(tls->ssl, tls->n) == SSL_ERROR_WANT_READ)
      continue;
    if (SSL_read(tls->ssl, R, tls->n < r ? tls->n : r) > 0)
      return true;
  }
  return false;
}

