// extension_apitest.js
// mini-framework for ExtensionApiTest browser tests

var chrome = chrome || {};
(function() {
  chrome.test = chrome.test || {};
  
  chrome.test.tests = chrome.test.tests || [];

  var completed = false;
  var currentTest;

  function complete() {
    completed = true;

    // Try to get the script to stop running immediately.
    // This isn't an error, just an attempt at saying "done".
    throw "completed";
  }

  chrome.test.fail = function(message) {
    if (completed) throw "completed";

    var stack;
    try {
      crash.me += 0;  // An intentional exception to get the stack trace.
    } catch (e) {
      stack = e.stack.split("\n");
      stack = stack.slice(2);  // Remove title and fail() lines.
      stack = stack.join("\n");
    }

    if (!message) {
      message = "FAIL (no message)";
    }
    message += "\n" + stack;
    console.log("[FAIL] " + currentTest.name + ": " + message);
    chrome.test.notifyFail(message);
    complete();
  }

  function allTestsSucceeded() {
    console.log("All tests succeeded");
    if (completed) throw "completed";

    chrome.test.notifyPass();
    complete();
  }

  chrome.test.runNextTest = function() {
    currentTest = chrome.test.tests.shift();
    if (!currentTest) {
      allTestsSucceeded();
      return;
    }
    try {
      currentTest.call();
    } catch (e) {
      message = e.stack;
      console.log("[FAIL] " + currentTest.name + ": " + message);
      chrome.test.notifyFail(message);
      complete();
    }
  }

  chrome.test.succeed = function() {
    console.log("[SUCCESS] " + currentTest.name);
    chrome.test.runNextTest();
  }

  chrome.test.assertTrue = function(test, message) {
    if (test !== true) {
      if (typeof(test) == "string") {
        if (message) {
          message = test + "\n" + message;
        } else {
          message = test;
        }
      }
      chrome.test.fail(message);
    }
  }

  chrome.test.assertNoLastError = function() {
    if (chrome.extension.lastError != undefined) {
      chrome.test.fail("lastError.message == " +
                       chrome.extension.lastError.message);
    }
  }

  // Wrapper for generating test functions, that takes care of calling
  // assertNoLastError() and succeed() for you.
  chrome.test.testFunction = function(func) {
    return function() {
      chrome.test.assertNoLastError();
      try {
        func.apply(null, arguments);
      } catch (e) {
        var stack = null;
        if (typeof(e.stack) != "undefined") {
          stack = e.stack.toString()
        }
        var msg = "Exception during execution of testFunction in " +
                  currentTest.name;
        if (stack) {
          msg += "\n" + stack;
        } else {
          msg += "\n(no stack available)";
        }
        chrome.test.fail(msg);
      }
      chrome.test.succeed();
    };
  }
  
  chrome.test.runTests = function(tests) {
    chrome.test.tests = tests;
    chrome.test.runNextTest();
  }
})();
