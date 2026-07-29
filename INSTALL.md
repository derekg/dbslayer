# Building DBSlayer

## Prerequisites

| Dependency | Version | Debian package |
|---|---|---|
| Apache Portable Runtime (APR) | 1.2+ | `libapr1-dev` |
| APR-util | 1.2+ | `libaprutil1-dev` |
| MySQL client libraries | 5.0+ | `libmysqlclient-dev` |
| GCC | C99+ | `gcc` |
| GNU Make | 3.81+ | `make` |

## Quick build

```bash
./configure
make
sudo make install
```

## Non-standard library locations

```bash
./configure \
  --with-apr-1-config=/path/to/apr-1-config \
  --with-apu-1-config=/path/to/apu-1-config \
  --with-mysql-config=/path/to/mysql_config
```

If the MySQL `mysql_config` from a binary tarball is broken (emits `-l` with no library name), create a wrapper:

```bash
cat > /usr/local/bin/mysql_config << 'EOF'
#!/bin/sh
prefix=/path/to/mysql-connector
echo "-I$prefix/include"
echo "-L$prefix/lib -lmysqlclient -lpthread -ldl -lm"
EOF
chmod +x /usr/local/bin/mysql_config
```

## RPATH for non-system libraries

If APR/MySQL are installed outside the standard library path, add rpath:

```bash
./configure LDFLAGS="-Wl,-rpath,/path/to/lib"
```

## Verify the build

```bash
./server/dbslayer -v          # print version
python3 test/test_bugfixes.py  # run standalone tests
```

## Debug build

```bash
CFLAGS="-g -O0 -Wall -Wextra -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" ./configure
make
```

## CI

GitHub Actions builds and tests on every push/PR. See `.github/workflows/build-and-test.yml`.
