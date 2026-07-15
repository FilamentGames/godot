#!/usr/bin/env python3

import contextlib
import os
import socket
import ssl
import subprocess
import sys
from http.server import HTTPServer, SimpleHTTPRequestHandler
from pathlib import Path


# See cpython GH-17851 and GH-17864.
class DualStackServer(HTTPServer):
    def server_bind(self):
        # Create IPv6 socket if available
        try:
            self.socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
        except (OSError, AttributeError):
            # Fall back to IPv4 if IPv6 is not available
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        return super().server_bind()


class CORSRequestHandler(SimpleHTTPRequestHandler):
    def end_headers(self):
        # Add Content-Encoding header for brotli-compressed .wasm files
        if self.path.endswith('.wasm'):
            self.send_header("Content-Encoding", "br")
        
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Access-Control-Allow-Origin", "*")
        super().end_headers()


def shell_open(url):
    if sys.platform == "win32":
        os.startfile(url)
    else:
        opener = "open" if sys.platform == "darwin" else "xdg-open"
        subprocess.call([opener, url])


def ensure_certificates(cert_dir):
    """Ensure SSL certificates exist, generate them if needed."""
    cert_path = cert_dir / "localhost.pem"
    key_path = cert_dir / "localhost-key.pem"
    
    if not cert_path.exists() or not key_path.exists():
        print("Certificates not found. Attempting to generate with mkcert...")
        try:
            # Change to cert directory for mkcert
            original_dir = os.getcwd()
            os.chdir(cert_dir)
            result = subprocess.run(["mkcert", "localhost"], capture_output=True, text=True)
            os.chdir(original_dir)
            
            if result.returncode != 0:
                print(f"Error generating certificates: {result.stderr}")
                print("Please install mkcert (https://github.com/FiloSottile/mkcert) or create certificates manually.")
                return False
            print("Certificates generated successfully.")
        except FileNotFoundError:
            print("mkcert not found. Please install mkcert (https://github.com/FiloSottile/mkcert)")
            print("or create localhost.pem and localhost-key.pem manually.")
            return False
    
    return True


if __name__ == "__main__":
    # Hardcoded configuration
    script_dir = Path(__file__).resolve().parent
    root = script_dir / "bin" / ".web_zip"
    cert_dir = root
    port = 3000
    use_https = True
    
    if not root.exists():
        print(f"Error: Root directory does not exist: {root}")
        sys.exit(1)
    
    os.chdir(root)
    
    address = ("", port)
    try:
        httpd = DualStackServer(address, CORSRequestHandler)
    except OSError as e:
        if e.errno == 10048 or "Address already in use" in str(e):
            print(f"Error: Port {port} is already in use. Please stop the other server or use a different port.")
        else:
            print(f"Error binding to port {port}: {e}")
        sys.exit(1)
    
    if use_https:
        if not ensure_certificates(cert_dir):
            print("Falling back to HTTP...")
            use_https = False
        else:
            cert_path = cert_dir / "localhost.pem"
            key_path = cert_dir / "localhost-key.pem"
            
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            try:
                context.load_cert_chain(str(cert_path), str(key_path))
                httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
            except Exception as e:
                print(f"Error loading SSL certificates: {e}")
                print("Falling back to HTTP...")
                use_https = False
    
    protocol = "https" if use_https else "http"
    url = f"{protocol}://127.0.0.1:{port}"
    
    print(f"Serving at: {url}")
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nKeyboard interrupt received, stopping server.")
    finally:
        httpd.server_close()
