#include <string.h>
#include <libsock/ssl.h>
#include <unistd.h>

static SSL_CTX *ctx;

void init_tls(void) {
  SSL_library_init();
  OpenSSL_add_ssl_algorithms();
}

bool configure_ctx(const char CERT[], const char KEY[]) {
  return CERT && KEY &&
    SSL_CTX_use_certificate_file(ctx, CERT, SSL_FILETYPE_PEM) > 0 &&
      SSL_CTX_use_PrivateKey_file(ctx, KEY, SSL_FILETYPE_PEM) > 0 &&
        SSL_CTX_check_private_key(ctx) > 0;
}

void init_client(tls_t *tls, const int sockfd) {
  tls->r = BIO_new(BIO_s_mem());
  tls->w = BIO_new(BIO_s_mem());
  ctx = SSL_CTX_new(TLS_client_method()); /* mode */
  tls->ssl = SSL_new(ctx);
  SSL_set_connect_state(tls->ssl); /* client mode */
  SSL_set_fd(tls->ssl, sockfd);
  SSL_do_handshake(tls->ssl);
  SSL_set_bio(tls->ssl, tls->r, tls->w);
}

void deinit_client(tls_t *tls) {
  SSL_shutdown(tls->ssl);
  SSL_free(tls->ssl);
  SSL_CTX_free(ctx);
}

bool write_ssl(tls_t *tls, const char S[], const size_t NS) {
  return SSL_write(tls->ssl, S, NS) == NS;
}

ssize_t bio_read(tls_t *tls, char S[], const size_t NS) {
  return BIO_read(tls->w, S, NS);
}

void bio_write(tls_t *tls, char p) {
  BIO_write(tls->r, &p, sizeof p);
}

bool read_ssl(char *p, tls_t *tls) {
  return SSL_read(tls->ssl, p, sizeof *p) > 0;
}
