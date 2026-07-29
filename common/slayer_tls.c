#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include "slayer_tls.h"

#ifdef HAVE_OPENSSL
#include <apr_portable.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

/*
 * Thread ownership: SSL_CTX is fully configured during initialization and then
 * shared read-only. Each SSL belongs to one connection; ownership moves with
 * that connection between stages and the SSL is never shared concurrently
 * across threads or between connections.
 */
struct slayer_tls_ctx {
	SSL_CTX *ctx;
};

static int slayer_tls_wait(SSL *ssl, int ssl_error) {
	struct pollfd pfd;
	int status;

	pfd.fd = SSL_get_fd(ssl);
	pfd.events = ssl_error == SSL_ERROR_WANT_WRITE ? POLLOUT : POLLIN;
	pfd.revents = 0;
	do {
		status = poll(&pfd, 1, 10000);
	} while (status < 0 && errno == EINTR);
	return status > 0 ? 0 : -1;
}

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
	if (SSL_set_fd(ssl, fd) != 1) {
		SSL_free(ssl);
		return NULL;
	}
	for (;;) {
		int accepted = SSL_accept(ssl);
		int ssl_error;
		if (accepted == 1) break;
		ssl_error = SSL_get_error(ssl, accepted);
		if ((ssl_error != SSL_ERROR_WANT_READ &&
		     ssl_error != SSL_ERROR_WANT_WRITE) ||
		    slayer_tls_wait(ssl, ssl_error) != 0) {
			ERR_print_errors_fp(stderr);
			SSL_free(ssl);
			return NULL;
		}
	}
	return ssl;
}

int slayer_tls_recv(void *tls, char *buf, int len) {
	int received;

	if (tls == NULL || buf == NULL || len <= 0) return -1;
	for (;;) {
		int ssl_error;
		received = SSL_read((SSL *)tls, buf, len);
		if (received > 0) return received;
		ssl_error = SSL_get_error((SSL *)tls, received);
		if ((ssl_error != SSL_ERROR_WANT_READ &&
		     ssl_error != SSL_ERROR_WANT_WRITE) ||
		    slayer_tls_wait((SSL *)tls, ssl_error) != 0) {
			return -1;
		}
	}
}

int slayer_tls_send(void *tls, const char *buf, int len) {
	int sent;

	if (tls == NULL || buf == NULL || len <= 0) return -1;
	for (;;) {
		int ssl_error;
		sent = SSL_write((SSL *)tls, buf, len);
		if (sent > 0) return sent;
		ssl_error = SSL_get_error((SSL *)tls, sent);
		if ((ssl_error != SSL_ERROR_WANT_READ &&
		     ssl_error != SSL_ERROR_WANT_WRITE) ||
		    slayer_tls_wait((SSL *)tls, ssl_error) != 0) {
			return -1;
		}
	}
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
