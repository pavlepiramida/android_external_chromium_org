#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import optparse
import os
import sys

import SimpleHTTPServer
import BaseHTTPServer

from build import calcdeps

DEFAULT_PORT = 8003

class Handler(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def do_GET(self):
    if self.path == '/src/base/deps.js':
      self.log_message('Regenerating deps')
      calcdeps.regenerate_deps()
    return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

class Server(BaseHTTPServer.HTTPServer):
  def handle_one_request(self):
    try:
      HTTPServer.handle_one_request(self)
    except KeyboardInterrupt:
      sys.exit(255)

def Main(args):
  parser = optparse.OptionParser()
  parser.add_option('--port',
                    action='store',
                    type='int',
                    default=DEFAULT_PORT,
                    help='Port to serve from')
  options, args = parser.parse_args()
  server = Server(('', options.port), Handler)
  sys.stderr.write("Now running on http://localhost:%i\n" % options.port)
  server.serve_forever()

if __name__ == '__main__':
  sys.exit(Main(sys.argv[1:]))
