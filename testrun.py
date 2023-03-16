# Run this file, or use 'py -m http.server 8000' for the same effect

import http.server
import socketserver

PORT = 8000

Handler = http.server.SimpleHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print("serving on http://127.0.0.1:" + str(PORT))
    httpd.serve_forever()