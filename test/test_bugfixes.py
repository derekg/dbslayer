#!/usr/bin/env python3
"""
Standalone unit tests for dbslayer bug fixes.
Tests the C components directly via HTTP without requiring MySQL.

Requires:
  - server/dbslayer binary built and in the build tree
  - Python 3.10+
  - No MySQL server needed (tests exercise HTTP parsing, JSON, and routing only)

Run:
  python3 test/test_bugfixes.py
"""
import http.client
import json
import ssl
import subprocess
import socket
import tempfile
import time
import os
import signal
import sys
import urllib.parse

HOST = "127.0.0.1"
PORT = 19099
TLS_PORT = 19100
BINARY = os.path.join(os.path.dirname(__file__), "..", "server", "dbslayer")
CONFIG = os.path.join(os.path.dirname(__file__), "test-my.cnf")  # minimal config — no MySQL needed for non-DB tests

class DBSlayerTest:
    def __init__(self):
        self.proc = None
        self.passed = 0
        self.failed = 0
        self.failures = []
        self.skipped = 0

    def start(self, extra_args=None):
        """Start dbslayer on a test port."""
        args = [BINARY, "-s", "localhost", "-c", CONFIG, "-p", str(PORT),
                "-d", "1"]
        if extra_args:
            args.extend(extra_args)
        self.proc = subprocess.Popen(
            args,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        # Wait for it to bind
        for _ in range(50):
            try:
                s = socket.create_connection((HOST, PORT), timeout=0.2)
                s.close()
                return True
            except (ConnectionRefusedError, OSError):
                if self.proc.poll() is not None:
                    return False
                time.sleep(0.1)
        return False

    def stop(self):
        if self.proc:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.proc.kill()
            self.proc = None

    def http_get(self, path, headers=None):
        """Make an HTTP GET request, return (status_code, body_text)."""
        conn = http.client.HTTPConnection(HOST, PORT, timeout=5)
        try:
            conn.request("GET", path, headers=headers or {})
            resp = conn.getresponse()
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
        except Exception as e:
            return -1, str(e)
        finally:
            conn.close()

    def https_get(self, path):
        """Make an HTTPS GET request with test-certificate verification off."""
        context = ssl._create_unverified_context()
        conn = http.client.HTTPSConnection(HOST, TLS_PORT, timeout=5,
                                           context=context)
        try:
            conn.request("GET", path)
            resp = conn.getresponse()
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, body
        except Exception as e:
            return -1, str(e)
        finally:
            conn.close()

    def skip(self, name, detail):
        self.skipped += 1
        print(f"  - {name} — skipped: {detail}")

    def http_get_raw(self, path_bytes):
        """Send a raw HTTP request with a non-standard path to test parser edge cases."""
        s = socket.create_connection((HOST, PORT), timeout=5)
        try:
            s.sendall(b"GET " + path_bytes + b" HTTP/1.0\r\nHost: localhost\r\n\r\n")
            data = b""
            while True:
                chunk = s.recv(4096)
                if not chunk:
                    break
                data += chunk
            parts = data.split(b"\r\n\r\n", 1)
            status_line = parts[0].split(b"\r\n")[0] if parts else b""
            body = parts[1] if len(parts) > 1 else b""
            return status_line.decode("utf-8", "replace"), body.decode("utf-8", "replace")
        finally:
            s.close()

    def assert_true(self, name, condition, detail=""):
        if condition:
            self.passed += 1
            print(f"  ✓ {name}")
        else:
            self.failed += 1
            self.failures.append((name, detail))
            print(f"  ✗ {name} — {detail}")

    def assert_contains(self, name, haystack, needle):
        self.assert_true(name, needle in haystack,
                         f"expected '{needle}' in response, got: {haystack[:200]}")

    def assert_not_contains(self, name, haystack, needle):
        self.assert_true(name, needle not in haystack,
                         f"'{needle}' should NOT be in response: {haystack[:200]}")

    def assert_json_valid(self, name, body):
        try:
            json.loads(body)
            self.assert_true(name, True)
        except json.JSONDecodeError as e:
            self.assert_true(name, False, f"invalid JSON: {e}, body: {body[:200]}")

    # ── Tests ──────────────────────────────────────────────────────

    def test_server_starts(self):
        """Server binds and responds to /stats."""
        status, body = self.http_get("/stats")
        self.assert_true("server starts and /stats returns 200", status == 200,
                         f"got status {status}, body: {body[:200]}")

    def test_f1_dispatch_status_initialized(self):
        """F1: dispatch status is initialized — /stats should not crash server."""
        for i in range(5):
            status, body = self.http_get("/stats")
            if status != 200:
                self.assert_true("F1: repeated /stats stays stable", False,
                                 f"request {i} got status {status}")
                return
        self.assert_true("F1: repeated /stats stays stable (5x)", True)

    def test_f8_non_object_json_root(self):
        """F8: /db?true should not crash — should return error, not crash."""
        # Send a boolean as JSON root
        encoded = urllib.parse.quote("true")
        status, body = self.http_get(f"/db?{encoded}")
        # Should get 200 (dbslayer always returns 200) with an ERROR in the body
        self.assert_true("F8: /db?true does not crash server", status == 200,
                         f"got status {status}")
        if status == 200:
            # Verify server is still alive
            status2, _ = self.http_get("/stats")
            self.assert_true("F8: server alive after /db?true", status2 == 200)

        # Test with an array
        encoded = urllib.parse.quote("[]")
        status, body = self.http_get(f"/db?{encoded}")
        self.assert_true("F8: /db?[] does not crash server", status == 200)

        # Test with a string
        encoded = urllib.parse.quote('"hello"')
        status, body = self.http_get(f"/db?{encoded}")
        self.assert_true("F8: /db?\"hello\" does not crash server", status == 200)

    def test_f9_accept_failure_resilience(self):
        """F9: rapid connect/close should not crash server."""
        for i in range(50):
            try:
                s = socket.create_connection((HOST, PORT), timeout=0.5)
                s.close()
            except:
                pass
        status, body = self.http_get("/stats")
        self.assert_true("F9: server alive after 50 connect/close", status == 200)

    def test_f11_json_depth_limit(self):
        """F11: deeply nested JSON should be rejected, not crash server."""
        # Create deeply nested JSON: [[[... 100 levels ...]]]
        depth = 200  # well above the 64 limit
        nested = "[" * depth + "]" * depth
        encoded = urllib.parse.quote(nested)
        status, body = self.http_get(f"/db?{encoded}")
        self.assert_true("F11: 200-deep nesting does not crash server", status == 200)
        # Verify server is still alive
        status2, _ = self.http_get("/stats")
        self.assert_true("F11: server alive after deep nesting", status2 == 200)

    def test_f4_json_control_chars_in_stats(self):
        """F4/F5: /stats response should be valid JSON (control chars escaped)."""
        status, body = self.http_get("/stats")
        if status == 200:
            self.assert_json_valid("F4: /stats returns valid JSON", body)

    def test_f17_password_not_in_stats_args(self):
        """F17: password should not appear in /stats/args."""
        status, body = self.http_get("/stats/args")
        self.assert_true("F17: /stats/args returns 200", status == 200)
        if status == 200:
            self.assert_not_contains("F17: password redacted from /stats/args",
                                     body, "conduit-admin")
            self.assert_not_contains("F17: no -x password visible",
                                     body, "-x")

    def test_f6_log_injection_crlf(self):
        """F6: CR/LF in request target should not inject into log entries."""
        # Send a request with embedded CR/LF in the path
        status_line, body = self.http_get_raw(b"/stats%0d%0aFAKE:%20injected")
        # /stats%0d%0a... won't match /stats, so we get 404
        # The key is the server doesn't crash
        status2, _ = self.http_get("/stats")
        self.assert_true("F6: server alive after CRLF request", status2 == 200)

    def test_shutdown_local(self):
        """Verify /shutdown works from localhost (local IP match).
        dbslayer queues the response then immediately closes the socket and
        shuts down, so the HTTP body may not be flushed — that's pre-existing
        behavior. The key assertions: server stops after /shutdown."""
        try:
            s = socket.create_connection((HOST, PORT), timeout=5)
            s.sendall(b"GET /shutdown HTTP/1.0\r\nHost: localhost\r\n\r\n")
            s.recv(4096)  # may or may not get data before close
            s.close()
        except:
            pass
        # Server should now be shutting down
        time.sleep(0.5)
        status2, _ = self.http_get("/stats")
        self.assert_true("server shuts down after /shutdown", status2 == -1 or status2 == 0,
                         f"expected server down, got status {status2}")

    def test_404_for_unknown_path(self):
        """Unknown path returns 404."""
        status, body = self.http_get("/nonexistent")
        self.assert_true("unknown path returns 404", status == 404,
                         f"got status {status}")

    def test_bearer_auth(self):
        """S9: bearer auth rejects missing/wrong tokens and accepts the right one."""
        status, body = self.http_get("/db?true")
        self.assert_true("S9 auth: /db without bearer token returns 401",
                         status == 401,
                         f"got status {status}, body: {body[:200]}")

        status, body = self.http_get(
            "/db?true", headers={"Authorization": "Bearer wrong-token"})
        self.assert_true("S9 auth: wrong bearer token returns 401",
                         status == 401,
                         f"got status {status}, body: {body[:200]}")

        status, body = self.http_get(
            "/db?true",
            headers={"authorization": "Bearer dbslayer-test-token"})
        self.assert_true("S9 auth: correct bearer token reaches /db",
                         status == 200,
                         f"got status {status}, body: {body[:200]}")

    def test_tls(self):
        """S9: TLS listener serves a basic HTTPS request when OpenSSL is enabled."""
        makefile = os.path.join(os.path.dirname(__file__), "..", "common",
                                "Makefile")
        try:
            with open(makefile, encoding="utf-8") as f:
                openssl_enabled = any(
                    line.startswith("OPENSSL_LIBS =") and
                    line.partition("=")[2].strip()
                    for line in f
                )
        except OSError:
            openssl_enabled = False
        if not openssl_enabled:
            self.skip("S9 TLS: HTTPS /stats returns 200",
                      "build was configured without OpenSSL")
            return

        with tempfile.TemporaryDirectory(prefix="dbslayer-tls-") as tempdir:
            cert = os.path.join(tempdir, "cert.pem")
            key = os.path.join(tempdir, "key.pem")
            try:
                subprocess.run(
                    ["openssl", "req", "-x509", "-newkey", "rsa:2048",
                     "-keyout", key, "-out", cert, "-days", "1", "-nodes",
                     "-subj", "/CN=localhost"],
                    check=True, stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )
            except (FileNotFoundError, subprocess.CalledProcessError) as e:
                self.assert_true("S9 TLS: generate test certificate", False,
                                 str(e))
                return

            if not self.start(["--tls-cert", cert, "--tls-key", key,
                               "--tls-port", str(TLS_PORT)]):
                self.assert_true("S9 TLS: server starts TLS listener", False)
                return
            try:
                status, body = self.https_get("/stats")
                self.assert_true("S9 TLS: HTTPS /stats returns 200",
                                 status == 200,
                                 f"got status {status}, body: {body[:200]}")
            finally:
                self.stop()

    def run_all(self):
        baseline_tests = [
            self.test_server_starts,
            self.test_f1_dispatch_status_initialized,
            self.test_f8_non_object_json_root,
            self.test_f9_accept_failure_resilience,
            self.test_f11_json_depth_limit,
            self.test_f4_json_control_chars_in_stats,
            self.test_f17_password_not_in_stats_args,
            self.test_f6_log_injection_crlf,
            self.test_404_for_unknown_path,
            self.test_shutdown_local,  # must be last — kills server
        ]
        if not self.start():
            self.assert_true("backwards-compatible plaintext server starts",
                             False)
            return False
        print(f"Plaintext server started on {HOST}:{PORT}\n")
        for test in baseline_tests:
            test()
        self.stop()

        print("\nBearer authentication")
        if not self.start(["--auth-token", "dbslayer-test-token"]):
            self.assert_true("S9 auth: protected server starts", False)
        else:
            try:
                self.test_bearer_auth()
            finally:
                self.stop()

        print("\nTLS")
        self.test_tls()
        return self.failed == 0

def main():
    print("=" * 60)
    print("dbslayer bugfix unit tests")
    print("=" * 60)

    if not os.path.exists(BINARY):
        print(f"ERROR: Binary not found at {BINARY}")
        print("Run 'make' first.")
        sys.exit(1)

    runner = DBSlayerTest()
    try:
        success = runner.run_all()
    finally:
        runner.stop()

    print(f"\n{'=' * 60}")
    print(f"Results: {runner.passed} passed, {runner.failed} failed, "
          f"{runner.skipped} skipped")
    if runner.failures:
        print("\nFailures:")
        for name, detail in runner.failures:
            print(f"  - {name}: {detail}")
    print(f"{'=' * 60}")

    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
