# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import unittest

from chrome_remote_control import browser_options
from chrome_remote_control import desktop_browser_finder
from chrome_remote_control.system_stub import (
  OSModuleStub,
  SysModuleStub
  )

# This file verifies the logic for finding a browser instance on all platforms
# at once. It does so by providing stubs for the OS/sys/subprocess primitives
# that the underlying finding logic usually uses to locate a suitable browser.
# We prefer this approach to having to run the same test on every platform on
# which we want this code to work.

class StubSubprocess(object):
  def __init__(self):
    self.call_hook = None

  def call(self, *args, **kwargs):
    assert self.call_hook
    return self.call_hook(*args, **kwargs)

class FindTestBase(unittest.TestCase):
  def setUp(self):
    self._options = browser_options.BrowserOptions()
    self._options.chrome_root = '../../../'
    self._sys_stub = SysModuleStub()
    self._os_stub = OSModuleStub(self._sys_stub)
    self._subprocess_stub = StubSubprocess()

  @property
  def _files(self):
    return self._os_stub.files

  def DoFindAll(self):
    return desktop_browser_finder.FindAllAvailableBrowsers(self._options,
        self._os_stub, self._sys_stub, self._subprocess_stub)

  def DoFindAllTypes(self):
    browsers = self.DoFindAll()
    return [b.browser_type for b in browsers]

def has_type(array, browser_type):
  return len([x for x in array if x.browser_type == browser_type]) != 0

class OSXFindTest(FindTestBase):
  def setUp(self):
    super(OSXFindTest, self).setUp()
    self._sys_stub.platform = 'darwin'
    self._files.append('/Applications/Google Chrome Canary.app/'
                       'Contents/MacOS/Google Chrome Canary')
    self._files.append('/Applications/Google Chrome.app/' +
                       'Contents/MacOS/Google Chrome')
    self._files.append(
      '../../../out/Release/Chromium.app/Contents/MacOS/Chromium')
    self._files.append(
      '../../../out/Debug/Chromium.app/Contents/MacOS/Chromium')
    self._files.append(
      '../../../out/Release/Content Shell.app/Contents/MacOS/Content Shell')
    self._files.append(
      '../../../out/Debug/Content Shell.app/Contents/MacOS/Content Shell')

  def testFindAll(self):
    types = self.DoFindAllTypes()
    self.assertEquals(
      set(types),
      set(['debug', 'release',
           'content-shell-debug', 'content-shell-release',
           'canary', 'system']))


class LinuxFindTest(FindTestBase):
  def setUp(self):
    super(LinuxFindTest, self).setUp()

    self._sys_stub.platform = 'linux2'
    self._files.append('/foo/chrome')
    self._files.append('../../../out/Release/chrome')
    self._files.append('../../../out/Debug/chrome')
    self._files.append('../../../out/Release/content_shell')
    self._files.append('../../../out/Debug/content_shell')

    self.has_google_chrome_on_path = False
    this = self
    def call_hook(*args, **kwargs): # pylint: disable=W0613
      if this.has_google_chrome_on_path:
        return 0
      raise OSError('Not found')
    self._subprocess_stub.call_hook = call_hook

  def testFindAllWithExact(self):
    types = self.DoFindAllTypes()
    self.assertEquals(
        set(types),
        set(['debug', 'release',
             'content-shell-debug', 'content-shell-release']))

  def testFindWithProvidedExecutable(self):
    self._options.browser_executable = '/foo/chrome'
    self.assertTrue('exact' in self.DoFindAllTypes())

  def testFindUsingDefaults(self):
    self.has_google_chrome_on_path = True
    self.assertTrue('release' in self.DoFindAllTypes())

    del self._files[1]
    self.has_google_chrome_on_path = True
    self.assertTrue('system' in self.DoFindAllTypes())

    self.has_google_chrome_on_path = False
    del self._files[1]
    self.assertEquals(['content-shell-debug', 'content-shell-release'],
                      self.DoFindAllTypes())

  def testFindUsingRelease(self):
    self.assertTrue('release' in self.DoFindAllTypes())


class WinFindTest(FindTestBase):
  def setUp(self):
    super(WinFindTest, self).setUp()

    self._sys_stub.platform = 'win32'
    self._os_stub.local_app_data = 'c:\\Users\\Someone\\AppData\\Local'
    self._files.append('c:\\tmp\\chrome.exe')
    self._files.append('..\\..\\..\\build\\Release\\chrome.exe')
    self._files.append('..\\..\\..\\build\\Debug\\chrome.exe')
    self._files.append('..\\..\\..\\build\\Release\\content_shell.exe')
    self._files.append('..\\..\\..\\build\\Debug\\content_shell.exe')
    self._files.append(self._os_stub.local_app_data + '\\' +
                       'Google\\Chrome\\Application\\chrome.exe')
    self._files.append(self._os_stub.local_app_data + '\\' +
                       'Google\\Chrome SxS\\Application\\chrome.exe')

  def testFindAllGivenDefaults(self):
    types = self.DoFindAllTypes()
    self.assertEquals(set(types),
                      set(['debug', 'release',
                           'content-shell-debug', 'content-shell-release',
                           'system', 'canary']))

  def testFindAllWithExact(self):
    self._options.browser_executable = 'c:\\tmp\\chrome.exe'
    types = self.DoFindAllTypes()
    self.assertEquals(
        set(types),
        set(['exact',
             'debug', 'release',
             'content-shell-debug', 'content-shell-release',
             'system', 'canary']))
