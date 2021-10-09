#include <openssl/err.h>
#include <openssl/pem.h>
#include <string.h>
#include <unistd.h>
#include <libsock/sock.h>
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

void init_clienttls(tls_t *tls, const int sockfd)
{
  memset(tls, 0, sizeof *tls);
  tls->ssl = SSL_new(ctx);
  tls->rbio = BIO_new(BIO_s_mem());
  tls->wbio = BIO_new(BIO_s_mem());
  tls->sbio = BIO_new_socket(sockfd, BIO_NOCLOSE);
  SSL_set_connect_state(tls->ssl); /* ssl client mode */
}

void deinit_clienttls(tls_t *tls)
{
  SSL_free(tls->ssl);
}

bool writesock_ssl(tcp_t *tcp, const char S[])
{
  tls_t *tls = &tcp->tls;
  SSL_set_bio(tls->ssl, tls->sbio, tls->sbio);
  const size_t s = strlen(S);
  return SSL_write(tls->ssl, S, s) == s;
}
/*
bool readsock_ssl(char R[], size_t r, tls_t *tls)
{
  SSL_set_bio(tls->ssl, tls->rbio, tls->rbio);
  char p;
  while (pollin(tls->tcp, 100) && readsock(&p, tls->tcp))
  {
    tls->W[tls->w] = p;
    tls->W[tls->w + 1] = '\0';
    int n = BIO_write(tls->rbio, tls->W + tls->w++, sizeof p),
      err = SSL_get_error(tls->ssl, n);
      tls->n += n;

    if (err == SSL_ERROR_WANT_READ ||
      err == SSL_ERROR_WANT_WRITE)
    {
      fprintf(stdout, "Want IO n = %zu\n", tls->n);
    }
    if (err == SSL_ERROR_ZERO_RETURN)
    {
      fprintf(stdout, "Zero return n = %zu\n", tls->n);
    }
    if (err == SSL_ERROR_ZERO_RETURN)
    {
      fprintf(stdout, "Error none n = %zu\n", tls->n);
    }
    if (err == SSL_ERROR_ZERO_RETURN)
    {
      fprintf(stdout, "Error syscall n = %zu\n", tls->n);
    }
    if (err == SSL_ERROR_WANT_READ || 
      err == SSL_ERROR_WANT_WRITE  || 
      err == SSL_ERROR_ZERO_RETURN ||
      err == SSL_ERROR_NONE || 
      err == SSL_ERROR_SYSCALL)
      continue;
    fprintf(stdout, "%zu %zu\n", tls->n, r);
    r = tls->n < r ? tls->n : r;
    fprintf(stdout, "%zu %zu\n", tls->n, r);
    //return SSL_read(tls->ssl, R, r) == r;
    if (SSL_read(tls->ssl, R, r) == r)
      return true;
  }

  return false;
}

bool readsock_ssl(char R[], void *proto)
{
  char p;
  tls_t *tls = proto;
  SSL_set_bio(tls->ssl, tls->rbio, tls->rbio);
  while (pollin(tls->tcp, 100) && readsock(&p, tls->tcp))
  {
    tls->W[tls->w] = p;
    tls->W[tls->w + 1] = '\0';
    tls->n += BIO_write(tls->rbio, tls->W + tls->w++, sizeof p);
  }
  
  return SSL_read(tls->ssl, R, tls->n) == tls->n;
}
*/
bool readsock_ssl(char *R, tcp_t *tcp)
{
  char p;
  tls_t *tls = &tcp->tls;
  SSL_set_bio(tls->ssl, tls->rbio, tls->rbio);
  while (pollin(tcp, 100) && readsock(&p, tcp))
  {
    tls->W[tls->w] = p;
    tls->W[tls->w + 1] = '\0';
    tls->n += BIO_write(tls->rbio, tls->W + tls->w++, sizeof p);
  }
  
  return SSL_read(tls->ssl, R, tls->n) > 0;
}
