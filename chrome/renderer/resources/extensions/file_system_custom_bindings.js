// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the fileSystem API.

var binding = require('binding').Binding.create('fileSystem');
var sendRequest = require('sendRequest');

var getFileBindingsForApi =
    require('fileEntryBindingUtil').getFileBindingsForApi;
var fileBindings = getFileBindingsForApi('fileSystem');
var bindFileEntryCallback = fileBindings.bindFileEntryCallback;
var entryIdManager = fileBindings.entryIdManager;

binding.registerCustomHook(function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;
  var fileSystem = bindingsAPI.compiledApi;

  function bindFileEntryFunction(functionName) {
    apiFunctions.setUpdateArgumentsPostValidate(
        functionName, function(fileEntry, callback) {
      var fileSystemName = fileEntry.filesystem.name;
      var relativePath = $String.slice(fileEntry.fullPath, 1);
      return [fileSystemName, relativePath, callback];
    });
  }
  $Array.forEach(['getDisplayPath', 'getWritableEntry', 'isWritableEntry'],
                  bindFileEntryFunction);

  $Array.forEach(['getWritableEntry', 'chooseEntry', 'restoreEntry'],
                  function(functionName) {
    bindFileEntryCallback(functionName, apiFunctions);
  });

  apiFunctions.setHandleRequest('retainEntry', function(fileEntry) {
    var id = entryIdManager.getEntryId(fileEntry);
    if (!id)
      return '';
    var fileSystemName = fileEntry.filesystem.name;
    var relativePath = $String.slice(fileEntry.fullPath, 1);

    sendRequest.sendRequest(this.name, [id, fileSystemName, relativePath],
      this.definition.parameters, {});
    return id;
  });

  apiFunctions.setHandleRequest('isRestorable',
      function(id, callback) {
    var savedEntry = entryIdManager.getEntryById(id);
    if (savedEntry) {
      sendRequest.safeCallbackApply(
          'fileSystem.isRestorable',
          {'stack': sendRequest.getExtensionStackTrace()},
          callback,
          [true]);
    } else {
      sendRequest.sendRequest(
          this.name, [id, callback], this.definition.parameters, {});
    }
  });

  apiFunctions.setUpdateArgumentsPostValidate('restoreEntry',
      function(id, callback) {
    var savedEntry = entryIdManager.getEntryById(id);
    if (savedEntry) {
      // We already have a file entry for this id so pass it to the callback and
      // send a request to the browser to move it to the back of the LRU.
      sendRequest.safeCallbackApply(
          'fileSystem.restoreEntry',
          {'stack': sendRequest.getExtensionStackTrace()},
          callback,
          [savedEntry]);
      return [id, false, null];
    } else {
      // Ask the browser process for a new file entry for this id, to be passed
      // to |callback|.
      return [id, true, callback];
    }
  });

  // TODO(benwells): Remove these deprecated versions of the functions.
  fileSystem.getWritableFileEntry = function() {
    console.log("chrome.fileSystem.getWritableFileEntry is deprecated");
    console.log("Please use chrome.fileSystem.getWritableEntry instead");
    $Function.apply(fileSystem.getWritableEntry, this, arguments);
  };

  fileSystem.isWritableFileEntry = function() {
    console.log("chrome.fileSystem.isWritableFileEntry is deprecated");
    console.log("Please use chrome.fileSystem.isWritableEntry instead");
    $Function.apply(fileSystem.isWritableEntry, this, arguments);
  };

  fileSystem.chooseFile = function() {
    console.log("chrome.fileSystem.chooseFile is deprecated");
    console.log("Please use chrome.fileSystem.chooseEntry instead");
    $Function.apply(fileSystem.chooseEntry, this, arguments);
  };
});

exports.bindFileEntryCallback = bindFileEntryCallback;
exports.binding = binding.generate();
