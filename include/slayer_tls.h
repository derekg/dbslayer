#ifndef _SLAYER_TLS_H_
#define _SLAYER_TLS_H_

#include <apr_network_io.h>

typedef struct slayer_tls_ctx slayer_tls_ctx;

slayer_tls_ctx *slayer_tls_init(const char *cert_path, const char *key_path);
void slayer_tls_ctx_free(slayer_tls_ctx *ctx);

/* Perform TLS handshake on an accepted socket. Returns NULL on failure. */
void *slayer_tls_accept(slayer_tls_ctx *ctx, apr_socket_t *sock);

/* Read/write through TLS session. Return bytes or -1 on error. */
int slayer_tls_recv(void *tls, char *buf, int len);
int slayer_tls_send(void *tls, const char *buf, int len);
void slayer_tls_close(void *tls);

#endif
