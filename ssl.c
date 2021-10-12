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
  SSL_set_bio(tls->ssl, tls->s, tls->s);
  SSL_set_connect_state(tls->ssl); /* ssl client mode */
}

void deinit_clienttls(tls_t *tls)
{
  SSL_free(tls->ssl);
  SSL_CTX_free(ctx);
}

bool writesock_ssl(tcp_t *tcp, const char S[])
{
  //SSL_set_bio(tls->ssl, tls->s, tls->s);
  const size_t NS = strlen(S);
  return SSL_write(tcp->tls.ssl, S, NS) == NS;
}
/*
bool readsock_ssl(char R[], tcp_t *tcp)
{
  char p;
  tls_t *tls = &tcp->tls;
  SSL_set_bio(tls->ssl, tls->r, tls->r);
  while (pollin(tls->tcp, 100) && readsock(&p, tls->tcp))
  {
    tls->W[tls->w] = p;
    tls->W[tls->w + 1] = '\0';
    tls->n += BIO_write(tls->rbio, tls->W + tls->w++, sizeof p);
  }
  
  return SSL_read(tls->ssl, R, tls->n) > 0;
}

void read_ssl(char R[], char p, tcp_t *tcp)
{
  tls_t *tls = &tcp->tls;
  R[tls->n] = p;
  R[tls->n + 1] = '\0';
  BIO_set_write_buf_size(tls->r, 1);
  tls->n += BIO_write(tls->r, R + tls->n++, sizeof p);
  SSL_set_bio(tls->ssl, tls->r, tls->r);
  if (SSL_read(tls->ssl, tls->W + tls->n++, tls->n) > 0)
    ;
    //R[0] = '\0';
}
*/

bool readsock_ssl(char *p, tcp_t *tcp)
{
  return SSL_read(tcp->tls.ssl, p, sizeof *p) > 0;
}
