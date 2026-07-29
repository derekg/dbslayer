#include <stdlib.h>

#include "slayer_tls.h"

#ifdef HAVE_OPENSSL
#include <apr_portable.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

struct slayer_tls_ctx {
	SSL_CTX *ctx;
};

slayer_tls_ctx *slayer_tls_init(const char *cert_path, const char *key_path) {
	slayer_tls_ctx *ctx;

	if (cert_path == NULL || key_path == NULL) return NULL;
	ctx = calloc(1, sizeof(slayer_tls_ctx));
	if (ctx == NULL) return NULL;
	ctx->ctx = SSL_CTX_new(TLS_server_method());
	if (ctx->ctx == NULL) {
		free(ctx);
		return NULL;
	}
	SSL_CTX_set_options(ctx->ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
	if (SSL_CTX_use_certificate_chain_file(ctx->ctx, cert_path) != 1 ||
	    SSL_CTX_use_PrivateKey_file(ctx->ctx, key_path, SSL_FILETYPE_PEM) != 1 ||
	    SSL_CTX_check_private_key(ctx->ctx) != 1) {
		SSL_CTX_free(ctx->ctx);
		free(ctx);
		return NULL;
	}
	return ctx;
}

void slayer_tls_ctx_free(slayer_tls_ctx *ctx) {
	if (ctx == NULL) return;
	SSL_CTX_free(ctx->ctx);
	free(ctx);
}

void *slayer_tls_accept(slayer_tls_ctx *ctx, apr_socket_t *sock) {
	apr_os_sock_t fd;
	SSL *ssl;

	if (ctx == NULL || sock == NULL ||
	    apr_os_sock_get(&fd, sock) != APR_SUCCESS) return NULL;
	ssl = SSL_new(ctx->ctx);
	if (ssl == NULL) return NULL;
	if (SSL_set_fd(ssl, fd) != 1 || SSL_accept(ssl) != 1) {
		SSL_free(ssl);
		return NULL;
	}
	return ssl;
}

int slayer_tls_recv(void *tls, char *buf, int len) {
	int received;

	if (tls == NULL || buf == NULL || len <= 0) return -1;
	received = SSL_read((SSL *)tls, buf, len);
	return received > 0 ? received : -1;
}

int slayer_tls_send(void *tls, const char *buf, int len) {
	int sent;

	if (tls == NULL || buf == NULL || len <= 0) return -1;
	sent = SSL_write((SSL *)tls, buf, len);
	return sent > 0 ? sent : -1;
}

void slayer_tls_close(void *tls) {
	if (tls == NULL) return;
	SSL_shutdown((SSL *)tls);
	SSL_free((SSL *)tls);
}

#else

struct slayer_tls_ctx {
	int unavailable;
};

slayer_tls_ctx *slayer_tls_init(const char *cert_path, const char *key_path) {
	(void)cert_path;
	(void)key_path;
	return NULL;
}

void slayer_tls_ctx_free(slayer_tls_ctx *ctx) {
	(void)ctx;
}

void *slayer_tls_accept(slayer_tls_ctx *ctx, apr_socket_t *sock) {
	(void)ctx;
	(void)sock;
	return NULL;
}

int slayer_tls_recv(void *tls, char *buf, int len) {
	(void)tls;
	(void)buf;
	(void)len;
	return -1;
}

int slayer_tls_send(void *tls, const char *buf, int len) {
	(void)tls;
	(void)buf;
	(void)len;
	return -1;
}

void slayer_tls_close(void *tls) {
	(void)tls;
}

#endif
