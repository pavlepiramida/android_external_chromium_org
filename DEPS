vars = {
  # Use this googlecode_url variable only if there is an internal mirror for it.
  # If you do not know, use the full path while defining your new deps entry.
  "googlecode_url": "http://%s.googlecode.com/svn",
  "sourceforge_url": "http://%(repo)s.svn.sourceforge.net/svnroot/%(repo)s",
  "webkit_trunk": "http://svn.webkit.org/repository/webkit/trunk",
  "nacl_trunk": "http://src.chromium.org/native_client/trunk",
  "webkit_revision": "117232",
  "chromium_git": "http://git.chromium.org/git",
  "swig_revision": "69281",
  "nacl_revision": "8592",
  # After changing nacl_revision, run 'glient sync' and check native_client/DEPS
  # to update other nacl_*_revision's.
  "nacl_tools_revision": "8228",  # native_client/DEPS: tools_rev

  # These hashes need to be updated when nacl_toolchain_revision is changed.
  # After changing nacl_toolchain_revision, run 'gclient runhooks' to get the
  # new values.
  "nacl_toolchain_linux_x86_hash":
      "bb547cdcb2f0ee05bcd89d69f6bb9f040dbb75b5",
  "nacl_toolchain_linux_x86_newlib_hash":
      "3e4b68370db6959e2748b037318a384b65bbbb43",
  "nacl_toolchain_mac_x86_hash":
      "ccee424c5ccc3e373f67d6af5fb28c6550d97660",
  "nacl_toolchain_mac_x86_newlib_hash":
      "37a896dc6d31e49f3612aead6cce878719586aa6",
  "nacl_toolchain_pnacl_linux_x86_32_hash":
      "18cb4755f56ad7832dea0602fc3c8518f9544146",
  "nacl_toolchain_pnacl_linux_x86_64_hash":
      "d68750488fb2456d6cafd33ca00522ecf106d542",
  "nacl_toolchain_pnacl_mac_x86_32_hash":
      "9e2dc26e1cc5a3a3a6c877f16280a2f003ad6ba2",
  "nacl_toolchain_pnacl_translator_hash":
      "7fe891cac2e9504fe5837cb8dae90629da12c3a1",
  "nacl_toolchain_pnacl_win_x86_32_hash":
      "22e44ecbbdcd4577f6a5dea8fb819b61b338dd21",
  "nacl_toolchain_win_x86_hash":
      "c62cc8258c9f618e9a4481b44fee9e1873469ad7",
  "nacl_toolchain_win_x86_newlib_hash":
      "7711dbc818e569834e6ebd19b1f8aa6e333062b5",
  "nacl_toolchain_revision": "8607",
  "pnacl_toolchain_revision": "8607",

  "libjingle_revision": "136",
  "libphonenumber_revision": "425",
  "libvpx_revision": "134182",
  "lss_revision": "9",

  # These two FFmpeg variables must be updated together.  One is used for SVN
  # checkouts and the other for Git checkouts.
  "ffmpeg_revision": "137006",
  "ffmpeg_hash": "4ece657f9dad6a37e7b7e3584eb8887aad4dc499",

  "sfntly_revision": "128",
  "skia_revision": "3982",
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling Skia
  # and V8 without interference from each other.
  "v8_revision": "11582",
  "webrtc_revision": "2180",
  "jsoncpp_revision": "248",
  "nss_revision": "136057",
  "rlz_revision": "128",
}

deps = {
  "src/breakpad/src":
    (Var("googlecode_url") % "google-breakpad") + "/trunk/src@939",

  "src/build/util/support":
    "/trunk/deps/support@20411",

  "src/googleurl":
    (Var("googlecode_url") % "google-url") + "/trunk@175",

  "src/seccompsandbox":
    (Var("googlecode_url") % "seccompsandbox") + "/trunk@178",

  "src/sdch/open-vcdiff":
    (Var("googlecode_url") % "open-vcdiff") + "/trunk@42",

  "src/testing/gtest":
    (Var("googlecode_url") % "googletest") + "/trunk@613",

  "src/testing/gmock":
    (Var("googlecode_url") % "googlemock") + "/trunk@405",

  "src/third_party/angle":
    (Var("googlecode_url") % "angleproject") + "/trunk@1046",

  # Note that this is *not* where we check out WebKit -- this just
  # puts some extra files into place for the real WebKit checkout to
  # happen.  See lines mentioning "webkit_revision" for the real
  # WebKit checkout.
  "src/third_party/WebKit":
    "/trunk/deps/third_party/WebKit@76115",

  "src/third_party/icu":
    "/trunk/deps/third_party/icu46@135385",

  "src/third_party/hunspell":
   "/trunk/deps/third_party/hunspell@132738",

  "src/third_party/hunspell_dictionaries":
    "/trunk/deps/third_party/hunspell_dictionaries@79099",

  "src/third_party/safe_browsing/testing":
    (Var("googlecode_url") % "google-safe-browsing") + "/trunk/testing@110",

  "src/third_party/cacheinvalidation/files/src/google":
    (Var("googlecode_url") % "google-cache-invalidation-api") +
    "/trunk/src/google@203",

  "src/third_party/leveldatabase/src":
    (Var("googlecode_url") % "leveldb") + "/trunk@64",

  "src/third_party/snappy/src":
    (Var("googlecode_url") % "snappy") + "/trunk@37",

  "src/tools/grit":
    (Var("googlecode_url") % "grit-i18n") + "/trunk@37",

  "src/tools/gyp":
    (Var("googlecode_url") % "gyp") + "/trunk@1381",

  "src/v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),

  "src/native_client":
    Var("nacl_trunk") + "/src/native_client@" + Var("nacl_revision"),

  "src/native_client_sdk/src/site_scons":
    Var("nacl_trunk") + "/src/native_client/site_scons@" + Var("nacl_revision"),

  "src/third_party/pymox/src":
    (Var("googlecode_url") % "pymox") + "/trunk@70",

  "src/chrome/test/data/extensions/api_test/permissions/nacl_enabled/bin":
    Var("nacl_trunk") + "/src/native_client/tests/prebuilt@" +
    Var("nacl_revision"),

  "src/third_party/sfntly/cpp/src":
    (Var("googlecode_url") % "sfntly") + "/trunk/cpp/src@" +
    Var("sfntly_revision"),

  "src/third_party/skia/src":
    (Var("googlecode_url") % "skia") + "/trunk/src@" + Var("skia_revision"),

  "src/third_party/skia/include":
    (Var("googlecode_url") % "skia") + "/trunk/include@" + Var("skia_revision"),

  "src/third_party/WebKit/LayoutTests":
    Var("webkit_trunk") + "/LayoutTests@" + Var("webkit_revision"),

  "src/third_party/WebKit/Source":
    Var("webkit_trunk") + "/Source@" + Var("webkit_revision"),

  "src/third_party/WebKit/Tools/DumpRenderTree":
    Var("webkit_trunk") + "/Tools/DumpRenderTree@" + Var("webkit_revision"),

  "src/third_party/WebKit/Tools/Scripts":
    Var("webkit_trunk") + "/Tools/Scripts@" + Var("webkit_revision"),

  "src/third_party/WebKit/Tools/TestWebKitAPI":
    Var("webkit_trunk") + "/Tools/TestWebKitAPI@" + Var("webkit_revision"),

  "src/third_party/ots":
    (Var("googlecode_url") % "ots") + "/trunk@87",

  "src/tools/page_cycler/acid3":
    "/trunk/deps/page_cycler/acid3@102714",

  "src/chrome/test/data/perf/canvas_bench":
    "/trunk/deps/canvas_bench@122605",

  "src/chrome/test/data/perf/frame_rate/content":
    "/trunk/deps/frame_rate/content@93671",

  "src/third_party/bidichecker":
    (Var("googlecode_url") % "bidichecker") + "/trunk/lib@4",

  "src/third_party/v8-i18n":
    (Var("googlecode_url") % "v8-i18n") + "/trunk@67",

  # When roll to another webgl conformance tests revision, please goto
  # chrome/test/gpu and run generate_webgl_conformance_test_list.py.
  "src/third_party/webgl_conformance":
    "/trunk/deps/third_party/webgl/sdk/tests@134742",

  # We should use the same software_rendering_list.json for all branches.
  "src/chrome/browser/resources/software_rendering_list":
    "/trunk/deps/gpu/software_rendering_list@135645",

  # We run these layout tests as UI tests. Since many of the buildbots that
  # run layout tests do NOT have access to the LayoutTest directory, we need
  # to map them here. In practice, these do not take up much space.
  "src/content/test/data/layout_tests/LayoutTests/fast/events":
    Var("webkit_trunk") + "/LayoutTests/fast/events@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/fast/js/resources":
    Var("webkit_trunk") + "/LayoutTests/fast/js/resources@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/fast/workers":
    Var("webkit_trunk") + "/LayoutTests/fast/workers@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/http/tests/resources":
    Var("webkit_trunk") + "/LayoutTests/http/tests/resources@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/http/tests/workers":
    Var("webkit_trunk") + "/LayoutTests/http/tests/workers@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/http/tests/xmlhttprequest":
    Var("webkit_trunk") + "/LayoutTests/http/tests/xmlhttprequest@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/http/tests/websocket/tests":
    Var("webkit_trunk") + "/LayoutTests/http/tests/websocket/tests@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/platform/chromium/fast/workers":
    Var("webkit_trunk") + "/LayoutTests/platform/chromium/fast/workers@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/platform/chromium/fast/events":
    Var("webkit_trunk") + "/LayoutTests/platform/chromium/fast/events@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/fast/events":
    Var("webkit_trunk") + "/LayoutTests/platform/chromium-win/fast/events@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/fast/workers":
    Var("webkit_trunk") + "/LayoutTests/platform/chromium-win/fast/workers@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/http/tests/appcache":
    Var("webkit_trunk") + "/LayoutTests/http/tests/appcache@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/http/tests/workers":
    Var("webkit_trunk") + "/LayoutTests/platform/chromium-win/http/tests/workers@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/platform/chromium-win/storage/domstorage":
    Var("webkit_trunk") + "/LayoutTests/platform/chromium-win/storage/domstorage@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/storage/domstorage":
    Var("webkit_trunk") + "/LayoutTests/storage/domstorage@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/storage/indexeddb":
    Var("webkit_trunk") + "/LayoutTests/storage/indexeddb@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/fast/filesystem/resources":
    Var("webkit_trunk") + "/LayoutTests/fast/filesystem/resources@" +
    Var("webkit_revision"),
  "src/content/test/data/layout_tests/LayoutTests/media":
    Var("webkit_trunk") + "/LayoutTests/media@" +
    Var("webkit_revision"),

  "src/third_party/swig/Lib":
    "/trunk/deps/third_party/swig/Lib@" + Var("swig_revision"),

  # Make sure you update the two functional.DEPS and webdriver.DEPS too.
  "src/third_party/webdriver/pylib":
    "http://selenium.googlecode.com/svn/trunk/py@13487",

  "src/third_party/libvpx":
    "/trunk/deps/third_party/libvpx@" +
    Var("libvpx_revision"),

  "src/third_party/ffmpeg":
    "/trunk/deps/third_party/ffmpeg@" +
    Var("ffmpeg_revision"),

  "src/third_party/libjingle/source":
    (Var("googlecode_url") % "libjingle") + "/trunk@" +
    Var("libjingle_revision"),

  "src/third_party/libsrtp":
    "/trunk/deps/third_party/libsrtp@123853",

  "src/third_party/speex":
    "/trunk/deps/third_party/speex@111570",

  "src/third_party/yasm/source/patched-yasm":
    "/trunk/deps/third_party/yasm/patched-yasm@134927",

  "src/third_party/libjpeg_turbo":
    "/trunk/deps/third_party/libjpeg_turbo@136524",

  "src/third_party/flac":
    "/trunk/deps/third_party/flac@120197",

  "src/third_party/pyftpdlib/src":
    (Var("googlecode_url") % "pyftpdlib") + "/trunk@977",

  # Needed to support nacl browser test jig.
  "src/third_party/pylib":
    Var("nacl_trunk") + "/src/third_party/pylib@" + Var("nacl_tools_revision"),
  "src/third_party/scons-2.0.1":
    Var("nacl_trunk") + "/src/third_party/scons-2.0.1@" +
        Var("nacl_tools_revision"),

  "src/third_party/webrtc":
    (Var("googlecode_url") % "webrtc") + "/stable/src@" + Var("webrtc_revision"),

  "src/third_party/jsoncpp/source/include":
    (Var("sourceforge_url") % {"repo": "jsoncpp"}) +
        "/trunk/jsoncpp/include@" + Var("jsoncpp_revision"),

  "src/third_party/jsoncpp/source/src/lib_json":
    (Var("sourceforge_url") % {"repo": "jsoncpp"}) +
        "/trunk/jsoncpp/src/lib_json@" + Var("jsoncpp_revision"),

  "src/third_party/libyuv":
    (Var("googlecode_url") % "libyuv") + "/trunk@247",

  "src/third_party/mozc/session":
    (Var("googlecode_url") % "mozc") + "/trunk/src/session@83",

  "src/third_party/mozc/chrome/chromeos/renderer":
    (Var("googlecode_url") % "mozc") + "/trunk/src/chrome/chromeos/renderer@83",

  "src/third_party/smhasher/src":
    (Var("googlecode_url") % "smhasher") + "/trunk@136",

  "src/third_party/libphonenumber/src/phonenumbers":
     (Var("googlecode_url") % "libphonenumber") +
         "/trunk/cpp/src/phonenumbers@" + Var("libphonenumber_revision"),
  "src/third_party/libphonenumber/src/test":
     (Var("googlecode_url") % "libphonenumber") + "/trunk/cpp/test@" +
         Var("libphonenumber_revision"),
  "src/third_party/libphonenumber/src/resources":
     (Var("googlecode_url") % "libphonenumber") + "/trunk/resources@" +
         Var("libphonenumber_revision"),

  "src/third_party/undoview":
    "/trunk/deps/third_party/undoview@119694",

  "src/tools/deps2git":
    "/trunk/tools/deps2git@128331",

  "src/third_party/webpagereplay":
    (Var("googlecode_url") % "web-page-replay") + "/trunk@465",
}


deps_os = {
  "win": {
    "src/chrome/tools/test/reference_build/chrome_win":
      "/trunk/deps/reference_builds/chrome_win@89574",

    "src/third_party/cygwin":
      "/trunk/deps/third_party/cygwin@133786",

    "src/third_party/python_26":
      "/trunk/tools/third_party/python_26@89111",

    "src/third_party/psyco_win32":
      "/trunk/deps/third_party/psyco_win32@79861",

    "src/third_party/lighttpd":
      "/trunk/deps/third_party/lighttpd@33727",

    # Chrome Frame related deps
    "src/third_party/xulrunner-sdk":
      "/trunk/deps/third_party/xulrunner-sdk@119756",
    "src/chrome_frame/tools/test/reference_build/chrome_win":
      "/trunk/deps/reference_builds/chrome_win@89574",

    # Parses Windows PE/COFF executable format.
    "src/third_party/pefile":
      (Var("googlecode_url") % "pefile") + "/trunk@63",

    # NSS, for SSLClientSocketNSS.
    "src/third_party/nss":
      "/trunk/deps/third_party/nss@" + Var("nss_revision"),

    "src/third_party/swig/win":
      "/trunk/deps/third_party/swig/win@" + Var("swig_revision"),

    # GNU binutils assembler for x86-32.
    "src/third_party/gnu_binutils":
      (Var("nacl_trunk") + "/deps/third_party/gnu_binutils@" +
       Var("nacl_tools_revision")),
    # GNU binutils assembler for x86-64.
    "src/third_party/mingw-w64/mingw/bin":
      (Var("nacl_trunk") + "/deps/third_party/mingw-w64/mingw/bin@" +
       Var("nacl_tools_revision")),

    "src/rlz":
      (Var("googlecode_url") % "rlz") + "/trunk@" + Var("rlz_revision"),

    # Dependencies used by libjpeg-turbo
    "src/third_party/yasm/binaries":
      "/trunk/deps/third_party/yasm/binaries@74228",

    # Binary level profile guided optimizations. This points to the
    # latest release binaries for the toolchain.
    "src/third_party/syzygy/binaries":
      (Var("googlecode_url") % "sawbuck") + "/trunk/syzygy/binaries@782",

    # Binaries for nacl sdk.
    "src/third_party/nacl_sdk_binaries":
      "/trunk/deps/third_party/nacl_sdk_binaries@111576",
  },
  "mac": {
    "src/chrome/tools/test/reference_build/chrome_mac":
      "/trunk/deps/reference_builds/chrome_mac@89574",

    "src/third_party/GTM":
      (Var("googlecode_url") % "google-toolbox-for-mac") + "/trunk@516",
    "src/third_party/pdfsqueeze":
      (Var("googlecode_url") % "pdfsqueeze") + "/trunk@4",
    "src/third_party/lighttpd":
      "/trunk/deps/third_party/lighttpd@33737",

    "src/third_party/swig/mac":
      "/trunk/deps/third_party/swig/mac@" + Var("swig_revision"),

    "src/rlz":
      (Var("googlecode_url") % "rlz") + "/trunk@" + Var("rlz_revision"),

    # NSS, for SSLClientSocketNSS.
    "src/third_party/nss":
      "/trunk/deps/third_party/nss@" + Var("nss_revision"),

    "src/chrome/installer/mac/third_party/xz/xz":
      "/trunk/deps/third_party/xz@87706",
  },
  "unix": {
    # Linux, really.
    "src/chrome/tools/test/reference_build/chrome_linux":
      "/trunk/deps/reference_builds/chrome_linux@89574",

    "src/third_party/xdg-utils":
      "/trunk/deps/third_party/xdg-utils@93299",

    "src/third_party/swig/linux":
      "/trunk/deps/third_party/swig/linux@" + Var("swig_revision"),

    "src/third_party/lss":
      ((Var("googlecode_url") % "linux-syscall-support") + "/trunk/lss@" +
       Var("lss_revision")),

    "src/third_party/openssl":
      "/trunk/deps/third_party/openssl@130472",

    "src/third_party/WebKit/Tools/gdb":
      Var("webkit_trunk") + "/Tools/gdb@" + Var("webkit_revision"),

    "src/third_party/gold":
      "/trunk/deps/third_party/gold@124239",
  },
}


include_rules = [
  # Everybody can use some things.
  "+base",
  "+build",
  "+googleurl",
  "+ipc",

  # For now, we allow ICU to be included by specifying "unicode/...", although
  # this should probably change.
  "+unicode",
  "+testing",
]


# checkdeps.py shouldn't check include paths for files in these dirs:
skip_child_includes = [
  "breakpad",
  "chrome_frame",
  "delegate_execute",
  "metro_driver",
  "native_client",
  "native_client_sdk",
  "o3d",
  "pdf",
  "sdch",
  "skia",
  "testing",
  "third_party",
  "v8",
]


hooks = [
  {
    # This downloads binaries for Native Client's newlib toolchain.
    # Done in lieu of building the toolchain from scratch as it can take
    # anywhere from 30 minutes to 4 hours depending on platform to build.
    "pattern": ".",
    "action": [
        "python", "src/build/download_nacl_toolchains.py",
         "--no-arm-trusted",
         "--optional-pnacl",
         "--pnacl-version", Var("pnacl_toolchain_revision"),
         "--file-hash", "pnacl_linux_x86_32",
             Var("nacl_toolchain_pnacl_linux_x86_32_hash"),
         "--file-hash", "pnacl_linux_x86_64",
             Var("nacl_toolchain_pnacl_linux_x86_64_hash"),
         "--file-hash", "pnacl_translator",
             Var("nacl_toolchain_pnacl_translator_hash"),
         "--file-hash", "pnacl_mac_x86_32",
             Var("nacl_toolchain_pnacl_mac_x86_32_hash"),
         "--file-hash", "pnacl_win_x86_32",
             Var("nacl_toolchain_pnacl_win_x86_32_hash"),
         "--x86-version", Var("nacl_toolchain_revision"),
         "--file-hash", "mac_x86_newlib",
             Var("nacl_toolchain_mac_x86_newlib_hash"),
         "--file-hash", "win_x86_newlib",
             Var("nacl_toolchain_win_x86_newlib_hash"),
         "--file-hash", "linux_x86_newlib",
             Var("nacl_toolchain_linux_x86_newlib_hash"),
         "--file-hash", "mac_x86",
             Var("nacl_toolchain_mac_x86_hash"),
         "--file-hash", "win_x86",
             Var("nacl_toolchain_win_x86_hash"),
         "--file-hash", "linux_x86",
             Var("nacl_toolchain_linux_x86_hash"),
         "--save-downloads-dir",
             "src/native_client_sdk/src/build_tools/toolchain_archives",
         "--keep",
    ],
  },
  {
    # Pull clang on mac. If nothing changed, or on non-mac platforms, this takes
    # zero seconds to run. If something changed, it downloads a prebuilt clang,
    # which takes ~20s, but clang speeds up builds by more than 20s.
    "pattern": ".",
    "action": ["python", "src/tools/clang/scripts/update.py", "--mac-only"],
  },
  {
    # Update the cygwin mount on Windows.
    "pattern": ".",
    "action": ["python", "src/build/win/setup_cygwin_mount.py", "--win-only"],
  },
  {
    # Update LASTCHANGE.
    "pattern": ".",
    "action": ["python", "src/build/util/lastchange.py",
               "-o", "src/build/util/LASTCHANGE"],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", "src/build/gyp_chromium"],
  },
]
