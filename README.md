# DBSlayer

A lightweight database abstraction layer written in C that exposes MySQL connections as JSON over HTTP. Originally built at the New York Times for high-traffic web infrastructure where front-end servers and database servers needed to scale independently.

**License:** Apache 2.0  
**Language:** C (POSIX, APR, MySQL client lib, OpenSSL)  
**Status:** Production — 2026 maintenance branch with security hardening, TLS, and bearer auth

---

## How it works

DBSlayer is a daemon that pools MySQL connections and serves them as an HTTP API. Clients send SQL as JSON in an HTTP GET query, and receive JSON result sets back. No client SDK needed — any language with HTTP and JSON support works.

```
[Client] --HTTP/JSON--> [DBSlayer] --MySQL protocol--> [MySQL]
                                 <--result sets----------
         <--JSON response--
```

### Example query

```bash
dbslayer -c /path/to/mysql.cnf -s servername
```

Query the standard MySQL world database:

```
http://localhost:9090/db?%7B%22SQL%22:%22select%20*%20from%20City%20limit%203%22%7D
```

Response:

```json
{
  "RESULT": {
    "TYPES": ["MYSQL_TYPE_LONG", "MYSQL_TYPE_STRING", "MYSQL_TYPE_STRING", "MYSQL_TYPE_STRING", "MYSQL_TYPE_LONG"],
    "HEADER": ["ID", "Name", "CountryCode", "District", "Population"],
    "ROWS": [
      [1, "Kabul", "AFG", "Kabol", 1780000],
      [2, "Qandahar", "AFG", "Qandahar", 237500],
      [3, "Herat", "AFG", "Herat", 186800]
    ]
  }
}
```

### Multi-statement support

DBSlayer enables `CLIENT_MULTI_STATEMENTS` by default. Send multiple semicolon-separated statements and receive an array of result sets:

```json
{"SQL": "SELECT 1; SELECT 2"}
```

Errors in later statements are reported in the response with `MYSQL_ERROR` and `MYSQL_ERRNO` fields.

### Named server selection

Use `-m server1:server2` to configure multiple MySQL backends. Clients select a backend per-request:

```json
{"SQL": "SELECT 1", "SERVER": "server2"}
```

### Output formats

Default is JSON. Request XML with:

```json
{"SQL": "SELECT 1", "FORMAT": "xml"}
```

### Metadata queries

```json
{"STAT": true}            → mysql_stat()
{"CLIENT_INFO": true}     → mysql_get_client_info()
{"HOST_INFO": true}       → mysql_get_host_info()
{"SERVER_VERSION": true}  → mysql_get_server_version()
{"SLAYER_HELP": true}     → list of available commands
```

### Error handling with rollback

```json
{"SQL": "INSERT ...", "ROLLBACK_ON_ERROR": true}
```

On error, the response includes `ROLLBACK_ON_ERROR` and `ROLLBACK_ON_ERROR_SUCCESS` fields.

---

## Build

### Prerequisites

- **APR** 1.2+ (`apr-1-config`)
- **APR-util** 1.2+ (`apu-1-config`)
- **MySQL client** 5.0+ (`mysql_config`)
- **GCC** with C99 support
- **OpenSSL** 1.1+ (optional, for TLS — `libssl-dev`)
- **Python 3.10+** (for tests only)

Install on Debian/Ubuntu:

```bash
sudo apt-get install libapr1-dev libaprutil1-dev libmysqlclient-dev libssl-dev gcc make
```

### Build

```bash
./configure
make
sudo make install
```

With TLS support:

```bash
./configure --with-openssl
make
sudo make install
```

If libraries are in non-standard locations:

```bash
./configure \
  --with-apr-1-config=/path/to/apr-1-config \
  --with-apu-1-config=/path/to/apu-1-config \
  --with-mysql-config=/path/to/mysql_config \
  --with-openssl=/path/to/openssl
```

### Verify

```bash
./server/dbslayer -v
python3 test/test_bugfixes.py
```

---

## Usage

### Basic

```bash
dbslayer -s server[:server2] -c /path/to/mysql.cnf [options]
```

### With authentication

```bash
dbslayer -s localhost -c /etc/mysql/dbslayer.cnf -k my-secret-token
```

Clients must send `Authorization: Bearer my-secret-token` header.

### With TLS

```bash
dbslayer -s localhost -c /etc/mysql/dbslayer.cnf \
  --tls-cert /etc/ssl/dbslayer.crt \
  --tls-key /etc/ssl/dbslayer.key \
  --tls-port 9443
```

Plaintext HTTP on `-p` port and TLS HTTPS on `--tls-port` run simultaneously.

### With auth + TLS

```bash
dbslayer -s localhost -c /etc/mysql/dbslayer.cnf \
  -k my-secret-token \
  --tls-cert /etc/ssl/dbslayer.crt \
  --tls-key /etc/ssl/dbslayer.key
```

### Credentials

MySQL credentials are **never** passed on the command line. Use one of:

1. **MySQL config file** (recommended) — the `-c` file's `[client]` or named group section:
   ```ini
   [mysqld1]
   user = readonly
   password = secret
   host = 127.0.0.1
   port = 3306
   ```

2. **Environment variables**:
   ```bash
   DBSLAYER_DB_USER=readonly DBSLAYER_DB_PASS=secret \
     dbslayer -s localhost -c /etc/mysql/dbslayer.cnf
   ```

No passwords appear in `ps`, `/proc/pid/cmdline`, or `/stats/args`.

### Options

| Flag | Description | Default |
|------|-------------|---------|
| `-s` | MySQL server name(s), colon-separated for round-robin | *required* |
| `-c` | Path to MySQL config file (e.g. `my.cnf`) | *required* |
| `-m` | Named-server mode (multiple backends, client selects per-request) | off |
| `-p` | Listen port (plaintext HTTP) | 9090 |
| `-h` | Bind address | all interfaces |
| `-t` | Worker thread count | 1 |
| `-w` | Socket timeout (seconds) | 10 |
| `-k` | Bearer auth token (also: `--auth-token`) | disabled |
| `--auth-token-file` | Path to file containing bearer token | disabled |
| `--tls-cert` | Path to TLS certificate (PEM) — enables TLS listener | disabled |
| `--tls-key` | Path to TLS private key (PEM) | disabled |
| `--tls-port` | TLS listener port | 9443 |
| `-b` | Base directory for static file serving | disabled |
| `-l` | Access log file path | none |
| `-e` | Error log file path | none |
| `-n` | Stats bucket count | 1440 (24h at 1-min) |
| `-i` | Stats bucket interval (seconds) | 60 |
| `-d` | Debug mode (foreground, cores enabled) | daemonized |
| `-v` | Print version and exit | |

### Environment variables

| Variable | Purpose |
|----------|---------|
| `DBSLAYER_DB_USER` | MySQL username (if not in config file) |
| `DBSLAYER_DB_PASS` | MySQL password (if not in config file) |
| `DBSLAYER_AUTH_TOKEN` | Bearer auth token (alternative to `-k`) |

### HTTP endpoints

| Path | Auth required | Description |
|------|---------------|-------------|
| `/db` | yes (if token set) | Execute SQL (JSON in query string) |
| `/dbform` | yes (if token set) | Execute SQL (form params, easier for browser) |
| `/stats` | yes (if token set) | Server statistics as JSON |
| `/stats/log` | yes (if token set) | Recent request log (100 entries) |
| `/stats/error` | yes (if token set) | Recent error log (100 entries) |
| `/stats/args` | yes (if token set) | Startup arguments (passwords redacted) |
| `/shutdown` | localhost only | Graceful shutdown (bearer auth exempt) |
| `/*` | yes (if token set) | Static file serving (if `-b` is set) |

When no auth token is configured, all endpoints are open (backwards compatible).

### Authentication

When `-k` or `--auth-token` is set, clients must include an `Authorization` header:

```
GET /db?%7B%22SQL%22:%22SELECT%201%22%7D HTTP/1.1
Host: localhost:9090
Authorization: Bearer my-secret-token
```

Requests without the header or with an incorrect token receive `401 Unauthorized`.

### TLS

When `--tls-cert` and `--tls-key` are provided, a TLS listener starts on `--tls-port` (default 9443). The plaintext listener on `-p` continues to run, allowing gradual migration.

```
https://localhost:9443/db?...
http://localhost:9090/db?...   (still works)
```

TLS requires building with `--with-openssl`. If OpenSSL is not detected at build time, TLS flags are accepted but print a warning at startup.

---

## Security

### Threat model

DBSlayer is designed as an **internal** service. The 2026 security modernization adds optional TLS and bearer authentication, but the core design — raw SQL over HTTP — is unchanged. When deployed without auth or TLS, all security controls must come from the surrounding deployment: firewalls, reverse proxies, network ACLs, and least-privilege MySQL grants.

### 2026 security hardening

**Phase 1 — Bug fixes (17 defects from Codex Security scan):**

| Fix | What it does |
|-----|-------------|
| F1 | Initialize dispatch status (prevent spurious shutdown) |
| F2 | Fail startup if log files can't be opened |
| F3 | Safe ctype calls (unsigned char casts) |
| F4 | Escape C0 control chars in JSON output |
| F5 | Strip XML-illegal control chars |
| F6 | Sanitize CR/LF in log fields (prevent log injection) |
| F7 | Native 64-bit BIGINT serialization (no precision loss) |
| F8 | Reject non-object JSON roots (prevent type confusion crash) |
| F9 | Check `apr_socket_accept` return (prevent null deref) |
| F10 | Keep parser pointers within buffer bounds |
| F11 | Bound JSON recursion at depth 64 (prevent stack overflow) |
| F12 | Report multi-statement errors to clients (was silent) |
| F13 | Retry queries only on connection loss (prevent double-write) |
| F14 | `realpath` containment for static file server |
| F15 | Non-blocking accept handoff (prevent accept-thread stall) |
| F16 | `apr_size_t` for response lengths (prevent integer overflow) |
| F17 | Redact password from `ps`/`/stats/args`, disable core dumps |

**Phase 2 — Security modernization:**

| Feature | What it does |
|---------|-------------|
| TLS | Optional OpenSSL TLS on separate port (default 9443) |
| Bearer auth | Optional token-based authentication on all endpoints |
| Credential cleanup | Removed `-u`/`-x` from CLI — credentials via config file or env vars |

### Known limitations

- No rate limiting or request size limits
- No query cost limits (a `SELECT * FROM huge_table` can exhaust memory)
- `/shutdown` gated by local IP comparison (spoofable by local processes, but bearer-auth exempt)
- Password stored in process memory for reconnection (not zeroized after connect)
- HTTP parser is hand-rolled and does not handle all RFC 7230 edge cases
- F15 (non-blocking accept handoff) changes threading behavior — load-test before production use

---

## Testing

### Standalone tests (no MySQL required)

```bash
python3 test/test_bugfixes.py
```

20 tests covering: server startup, type confusion rejection, recursion depth limit, accept resilience, password redaction, log injection, JSON validity, shutdown, 404 routing, bearer auth (reject/accept), and TLS HTTPS.

### Integration tests (require MySQL)

```bash
# Load test data
mysql -u root -p < test/test_dbslayer.sql

# Start dbslayer
dbslayer -c test/test-my.cnf -s localhost -p 9090

# Run Ruby test suite
ruby test/unit-tests.rb localhost 9090
```

### CI

GitHub Actions workflow (`.github/workflows/build-and-test.yml`) builds and runs standalone tests on every push and PR.

---

## Architecture

```
                   ┌──────────────────────────────────────────────┐
                   │           dbslayer (single process)            │
                   │                                              │
  Plaintext :9090 ─┤  accept thread (poll)                        │
  TLS :9443    ────┤    ↓ apr_queue_trypush (non-blocking)        │
  (optional)       │                                              │
                   │  worker threads (N, default 1)                │
                   │    ↓ decode JSON → mysql_query                │
                   │    ↓ mysql_store_result → JSON serialize      │
                   │    ↓ apr_queue_push (out)                     │
                   │    ↓ bearer token check (if configured)       │
                   │                                              │
                   │  output thread (poll)                         │
                   │    ↓ TLS send / apr_socket_send               │
                   └──────────────────────────────────────────────┘
```

- **APR pools** for memory management (per-connection, per-request)
- **APR pollset** for non-blocking I/O
- **APR thread queues** for work distribution
- **OpenSSL** for optional TLS (separate listener, non-blocking I/O via SSL_read/SSL_write)
- **Custom JSON parser** (recursive descent, depth-limited at 64)
- **Custom JSON/XML serializers** (bucket-brigade based)
- **Skip list** data structure for JSON objects (O(log n) lookup)
- **MySQL connection pooling** with round-robin failover (`-s`) or named selection (`-m`)

### Source layout

```
server/dbslayer_server.c     Entry point, arg parsing, service registration
common/slayer_http_server.c   HTTP server: accept, dispatch, thread pool, response, auth
common/slayer_http_parse.c    Hand-rolled HTTP request-line + header parser
common/slayer_tls.c           Optional OpenSSL TLS module (init, accept, recv, send, close)
common/simplejson.c           JSON decoder (recursive descent, depth-limited)
common/serializejson.c        JSON serializer (bucket brigade)
common/json2xml.c             XML serializer
common/json_skip.c            Skip list (JSON object storage)
common/urldecode.c            URL percent-decoding
common/slayer_http_fileserver.c  Static file serving with realpath containment
common/slayer_server_logging.c   Access/error logging with CRLF sanitization
common/slayer_server_stats.c     Statistics buckets
db/dbaccess.c                 MySQL connection management, query execution, result→JSON
include/                      Header files (slayer_tls.h added)
test/                         Python (20 tests) and Ruby test suites
```

---

## History

The DBSlayer was written in 2007 by Derek Gottfrid at the New York Times, with assistance from Jacob Harris. Other contributors include Roger Caplan (admin scripts, PHP examples), Tammy Hepps, and Andrzej Lawn (stats AJAX page). Ken Robertson contributed affected-rows and insert-id support.

Originally hosted at dbslayer.org, the source moved to GitHub in 2009. The 2026 maintenance branch added 17 security fixes, native 64-bit BIGINT support, optional TLS, bearer authentication, credential cleanup, CI, and standalone tests.

## License

Apache License, Version 2.0. See [COPYING](COPYING).
