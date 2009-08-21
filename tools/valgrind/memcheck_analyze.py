#!/usr/bin/python
# Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# memcheck_analyze.py

''' Given a valgrind XML file, parses errors and uniques them.'''

import logging
import optparse
import os
import re
import subprocess
import sys
import tempfile
import time
from xml.dom.minidom import parse
from xml.parsers.expat import ExpatError

# Global symbol table (yuck)
TheAddressTable = None

GDB_LINE_RE = re.compile(r'Line ([0-9]*) of "([^"]*)".*')

def _GdbOutputToFileLine(output_line):
  ''' Parse the gdb output line, return a pair (file, line num) '''
  match =  GDB_LINE_RE.match(output_line)
  if match:
    return match.groups()[1], match.groups()[0]
  else:
    return None

def ResolveAddressesWithinABinary(binary_name, address_list):
  ''' For each address, return a pair (file, line num) '''
  commands = tempfile.NamedTemporaryFile()
  commands.write('file %s\n' % binary_name)
  for addr in address_list:
    commands.write('info line *%s\n' % addr)
  commands.write('quit\n')
  commands.flush()
  gdb_commandline = 'gdb -batch -x %s 2>/dev/null' % commands.name
  gdb_pipe = os.popen(gdb_commandline)
  result = gdb_pipe.readlines()

  address_count = 0
  ret = {}
  for line in result:
    if line.startswith('Line'):
      ret[address_list[address_count]] = _GdbOutputToFileLine(line)
      address_count += 1
    if line.startswith('No line'):
      ret[address_list[address_count]] = (None, None)
      address_count += 1
  gdb_pipe.close()
  commands.close()
  return ret

class _AddressTable(object):
  ''' Object to do batched line number lookup. '''
  def __init__(self):
    self._binaries = {}
    self._all_resolved = False

  def Add(self, binary, address):
    ''' Register a lookup request. '''
    if binary in self._binaries:
      self._binaries[binary].append(address)
    else:
      self._binaries[binary] = [address]
    self._all_resolved = False

  def ResolveAll(self):
    ''' Carry out all lookup requests. '''
    self._translation = {}
    for binary in self._binaries.keys():
      addr = ResolveAddressesWithinABinary(binary, self._binaries[binary])
      self._translation[binary] = addr
    self._all_resolved = True

  def GetFileLine(self, binary, addr):
    ''' Get the (filename, linenum) result of a previously-registered lookup request. '''
    if self._all_resolved:
      if binary in self._translation:
        if addr in self._translation[binary]:
          return self._translation[binary][addr]
    return (None, None)

# These are functions (using C++ mangled names) that we look for in stack
# traces. We don't show stack frames while pretty printing when they are below
# any of the following:
_TOP_OF_STACK_POINTS = [
  # Don't show our testing framework.
  "testing::Test::Run()",
  # Also don't show the internals of libc/pthread.
  "start_thread"
]

def getTextOf(top_node, name):
  ''' Returns all text in all DOM nodes with a certain |name| that are children
  of |top_node|.
  '''

  text = ""
  for nodes_named in top_node.getElementsByTagName(name):
    text += "".join([node.data for node in nodes_named.childNodes
                     if node.nodeType == node.TEXT_NODE])
  return text

def removeCommonRoot(source_dir, directory):
  '''Returns a string with the string prefix |source_dir| removed from
  |directory|.'''
  if source_dir:
    # Do this for safety, just in case directory is an absolute path outside of
    # source_dir.
    prefix = os.path.commonprefix([source_dir, directory])
    return directory[len(prefix) + 1:]

  return directory

# Constants that give real names to the abbreviations in valgrind XML output.
INSTRUCTION_POINTER = "ip"
OBJECT_FILE = "obj"
FUNCTION_NAME = "fn"
SRC_FILE_DIR = "dir"
SRC_FILE_NAME = "file"
SRC_LINE = "line"

def gatherFrames(node, source_dir):
  frames = []
  for frame in node.getElementsByTagName("frame"):
    frame_dict = {
      INSTRUCTION_POINTER : getTextOf(frame, INSTRUCTION_POINTER),
      OBJECT_FILE         : getTextOf(frame, OBJECT_FILE),
      FUNCTION_NAME       : getTextOf(frame, FUNCTION_NAME),
      SRC_FILE_DIR        : removeCommonRoot(
          source_dir, getTextOf(frame, SRC_FILE_DIR)),
      SRC_FILE_NAME       : getTextOf(frame, SRC_FILE_NAME),
      SRC_LINE            : getTextOf(frame, SRC_LINE)
    }
    frames += [frame_dict]
    if frame_dict[FUNCTION_NAME] in _TOP_OF_STACK_POINTS:
      break
    global TheAddressTable
    if TheAddressTable != None and frame_dict[SRC_LINE] == "":
      # Try using gdb
      TheAddressTable.Add(frame_dict[OBJECT_FILE], frame_dict[INSTRUCTION_POINTER])
  return frames

class ValgrindError:
  ''' Takes a <DOM Element: error> node and reads all the data from it. A
  ValgrindError is immutable and is hashed on its pretty printed output.
  '''

  def __init__(self, source_dir, error_node):
    ''' Copies all the relevant information out of the DOM and into object
    properties.

    Args:
      error_node: The <error></error> DOM node we're extracting from.
      source_dir: Prefix that should be stripped from the <dir> node.
    '''

    # Valgrind errors contain one <what><stack> pair, plus an optional
    # <auxwhat><stack> pair, plus an optional <origin><what><stack></origin>.
    # (Origin is nicely enclosed; too bad the other two aren't.)
    # The most common way to see all three in one report is
    # a syscall with a parameter that points to uninitialized memory, e.g.
    # Format:
    # <error>
    #   <unique>0x6d</unique>
    #   <tid>1</tid>
    #   <kind>SyscallParam</kind>
    #   <what>Syscall param write(buf) points to uninitialised byte(s)</what>
    #   <stack>
    #     <frame>
    #     ...
    #     </frame>
    #   </stack>
    #   <auxwhat>Address 0x5c9af4f is 7 bytes inside a block of ...</auxwhat>
    #   <stack>
    #     <frame>
    #     ...
    #     </frame>
    #   </stack>
    #   <origin>
    #   <what>Uninitialised value was created by a heap allocation</what>
    #   <stack>
    #     <frame>
    #     ...
    #     </frame>
    #   </stack>
    #   </origin>
    #
    # Each frame looks like this:
    #  <frame>
    #    <ip>0x83751BC</ip>
    #    <obj>/usr/local/google/bigdata/dkegel/chrome-build/src/out/Release/base_unittests</obj>
    #    <fn>_ZN7testing8internal12TestInfoImpl7RunTestEPNS_8TestInfoE</fn>
    #    <dir>/data/dkegel/chrome-build/src/testing/gtest/src</dir>
    #    <file>gtest-internal-inl.h</file>
    #    <line>655</line>
    #  </frame>
    # although the dir, file, and line elements are missing if there is no debug info.

    self._kind = getTextOf(error_node, "kind")
    self._backtraces = []

    # Iterate through the nodes, parsing <what|auxwhat><stack> pairs.
    description = None
    for node in error_node.childNodes:
      if node.localName == "what" or node.localName == "auxwhat":
        description = "".join([n.data for n in node.childNodes
                              if n.nodeType == n.TEXT_NODE])
      elif node.localName == "xwhat":
        description = getTextOf(node, "text")
      elif node.localName == "stack":
        self._backtraces.append([description, gatherFrames(node, source_dir)])
        description = None
      elif node.localName == "origin":
        description = getTextOf(node, "what")
        stack = node.getElementsByTagName("stack")[0]
        frames = gatherFrames(stack, source_dir)
        self._backtraces.append([description, frames])
        description = None
        stack = None
        frames = None

  def __str__(self):
    ''' Pretty print the type and backtrace(s) of this specific error,
        including suppression (which is just a mangled backtrace).'''
    output = self._kind + "\n"
    for backtrace in self._backtraces:
      output += backtrace[0] + "\n"
      filter = subprocess.Popen("c++filt -n", stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT,
                                shell=True,
                                close_fds=True)
      buf = ""
      for frame in backtrace[1]:
        buf +=  (frame[FUNCTION_NAME] or frame[INSTRUCTION_POINTER]) + "\n"
      (stdoutbuf, stderrbuf) = filter.communicate(buf.encode('latin-1'))
      demangled_names = stdoutbuf.split("\n")

      i = 0
      for frame in backtrace[1]:
        output += ("  " + demangled_names[i])
        i = i + 1

        global TheAddressTable
        if TheAddressTable != None and frame[SRC_FILE_DIR] == "":
           # Try using gdb
           foo = TheAddressTable.GetFileLine(frame[OBJECT_FILE], frame[INSTRUCTION_POINTER])
           if foo[0] != None:
             output += (" (" + foo[0] + ":" + foo[1] + ")")
        elif frame[SRC_FILE_DIR] != "":
          output += (" (" + frame[SRC_FILE_DIR] + "/" + frame[SRC_FILE_NAME] + ":" +
                     frame[SRC_LINE] + ")")
        else:
          output += " (" + frame[OBJECT_FILE] + ")"
        output += "\n"

      output += "Suppression:\n"
      for frame in backtrace[1]:
        output += "  fun:" + (frame[FUNCTION_NAME] or "*") + "\n"

    return output

  def UniqueString(self):
    ''' String to use for object identity. Don't print this, use str(obj)
    instead.'''
    rep = self._kind + " "
    for backtrace in self._backtraces:
      for frame in backtrace[1]:
        rep += frame[FUNCTION_NAME]

        if frame[SRC_FILE_DIR] != "":
          rep += frame[SRC_FILE_DIR] + "/" + frame[SRC_FILE_NAME]
        else:
          rep += frame[OBJECT_FILE]

    return rep

  def __hash__(self):
    return hash(self.UniqueString())
  def __eq__(self, rhs):
    return self.UniqueString() == rhs

def find_and_truncate(f):
  f.seek(0)
  while True:
    line = f.readline()
    if line == "":
      return False
    if '</valgrindoutput>' in line:
      # valgrind often has garbage after </valgrindoutput> upon crash
      f.truncate()
      return True

class MemcheckAnalyze:
  ''' Given a set of Valgrind XML files, parse all the errors out of them,
  unique them and output the results.'''

  def __init__(self, source_dir, files, show_all_leaks=False, use_gdb=False):
    '''Reads in a set of files.

    Args:
      source_dir: Path to top of source tree for this build
      files: A list of filenames.
      show_all_leaks: whether to show even less important leaks
    '''

    if use_gdb:
      global TheAddressTable
      TheAddressTable = _AddressTable()
    self._errors = set()
    badfiles = set()
    start = time.time()
    self._parse_failed = False
    for file in files:
      # Wait up to three minutes for valgrind to finish writing all files,
      # but after that, just skip incomplete files and warn.
      f = open(file, "r+")
      found = False
      firstrun = True
      origsize = os.path.getsize(file)
      while (not found and (firstrun or ((time.time() - start) < 180.0))):
        firstrun = False
        f.seek(0)
        found = find_and_truncate(f)
        if not found:
          time.sleep(1)
      f.close()
      if not found:
        badfiles.add(file)
      else:
        newsize = os.path.getsize(file)
        if origsize > newsize+1:
          logging.warn(str(origsize - newsize) + " bytes of junk were after </valgrindoutput> in %s!" % file)
        try:
          raw_errors = parse(file).getElementsByTagName("error")
          for raw_error in raw_errors:
            # Ignore "possible" leaks for now by default.
            if (show_all_leaks or
                getTextOf(raw_error, "kind") != "Leak_PossiblyLost"):
              error = ValgrindError(source_dir, raw_error)
              self._errors.add(error)
        except ExpatError, e:
          self._parse_failed = True
          logging.warn("could not parse %s: %s" % (file, e))
          lineno = e.lineno - 1
          context_lines = 5
          context_start = max(0, lineno - context_lines)
          context_end = lineno + context_lines + 1
          context_file = open(file, "r")
          for i in range(0, context_start):
            context_file.readline()
          for i in range(context_start, context_end):
            context_data = context_file.readline().rstrip()
            if i != lineno:
              logging.warn("  %s" % context_data)
            else:
              logging.warn("> %s" % context_data)
          context_file.close()
          continue
    if len(badfiles) > 0:
      logging.warn("valgrind didn't finish writing %d files?!" % len(badfiles))
      for file in badfiles:
        logging.warn("Last 20 lines of %s :" % file)
        os.system("tail -n 20 '%s' 1>&2" % file)

  def Report(self):
    if self._parse_failed:
      logging.error("FAIL! Couldn't parse Valgrind output file")
      return -2

    if self._errors:
      logging.error("FAIL! There were %s errors: " % len(self._errors))

      if TheAddressTable != None:
        TheAddressTable.ResolveAll()

      for error in self._errors:
        logging.error(error)

      return -1

    logging.info("PASS! No errors found!")
    return 0

def _main():
  '''For testing only. The MemcheckAnalyze class should be imported instead.'''
  retcode = 0
  parser = optparse.OptionParser("usage: %prog [options] <files to analyze>")
  parser.add_option("", "--source_dir",
                    help="path to top of source tree for this build"
                    "(used to normalize source paths in baseline)")

  (options, args) = parser.parse_args()
  if not len(args) >= 1:
    parser.error("no filename specified")
  filenames = args

  analyzer = MemcheckAnalyze(options.source_dir, filenames, use_gdb=True)
  retcode = analyzer.Report()

  sys.exit(retcode)

if __name__ == "__main__":
  _main()
