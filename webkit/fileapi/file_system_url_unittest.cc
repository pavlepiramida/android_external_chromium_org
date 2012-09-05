// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/fileapi/file_system_url.h"

#include "base/file_path.h"
#include "googleurl/src/gurl.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "webkit/fileapi/file_system_types.h"
#include "webkit/fileapi/file_system_util.h"

namespace fileapi {
namespace {
FileSystemURL CreateFileSystemURL(const char* url) {
 return FileSystemURL(GURL(url));
}
}  // namespace

TEST(FileSystemURLTest, ParsePersistent) {
  FileSystemURL url = CreateFileSystemURL(
     "filesystem:http://chromium.org/persistent/directory/file");
  ASSERT_TRUE(url.is_valid());
  EXPECT_EQ("http://chromium.org/", url.origin().spec());
  EXPECT_EQ(kFileSystemTypePersistent, url.type());
  EXPECT_EQ(FILE_PATH_LITERAL("file"),
      VirtualPath::BaseName(url.path()).value());
  EXPECT_EQ(FILE_PATH_LITERAL("directory"), url.path().DirName().value());
}

TEST(FileSystemURLTest, ParseTemporary) {
  FileSystemURL url = CreateFileSystemURL(
      "filesystem:http://chromium.org/temporary/directory/file");
  ASSERT_TRUE(url.is_valid());
  EXPECT_EQ("http://chromium.org/", url.origin().spec());
  EXPECT_EQ(kFileSystemTypeTemporary, url.type());
  EXPECT_EQ(FILE_PATH_LITERAL("file"),
      VirtualPath::BaseName(url.path()).value());
  EXPECT_EQ(FILE_PATH_LITERAL("directory"), url.path().DirName().value());
}

TEST(FileSystemURLTest, EnsureFilePathIsRelative) {
  FileSystemURL url = CreateFileSystemURL(
      "filesystem:http://chromium.org/temporary/////directory/file");
  ASSERT_TRUE(url.is_valid());
  EXPECT_EQ("http://chromium.org/", url.origin().spec());
  EXPECT_EQ(kFileSystemTypeTemporary, url.type());
  EXPECT_EQ(FILE_PATH_LITERAL("file"),
      VirtualPath::BaseName(url.path()).value());
  EXPECT_EQ(FILE_PATH_LITERAL("directory"), url.path().DirName().value());
  EXPECT_FALSE(url.path().IsAbsolute());
}

TEST(FileSystemURLTest, RejectBadSchemes) {
  EXPECT_FALSE(CreateFileSystemURL("http://chromium.org/").is_valid());
  EXPECT_FALSE(CreateFileSystemURL("https://chromium.org/").is_valid());
  EXPECT_FALSE(CreateFileSystemURL("file:///foo/bar").is_valid());
  EXPECT_FALSE(CreateFileSystemURL("foobar:///foo/bar").is_valid());
}

TEST(FileSystemURLTest, UnescapePath) {
  FileSystemURL url = CreateFileSystemURL(
      "filesystem:http://chromium.org/persistent/%7Echromium/space%20bar");
  ASSERT_TRUE(url.is_valid());
  EXPECT_EQ(FILE_PATH_LITERAL("space bar"),
      VirtualPath::BaseName(url.path()).value());
  EXPECT_EQ(FILE_PATH_LITERAL("~chromium"), url.path().DirName().value());
}

TEST(FileSystemURLTest, RejectBadType) {
  EXPECT_FALSE(CreateFileSystemURL(
      "filesystem:http://c.org/foobar/file").is_valid());
}

TEST(FileSystemURLTest, RejectMalformedURL) {
  EXPECT_FALSE(CreateFileSystemURL("filesystem:///foobar/file").is_valid());
  EXPECT_FALSE(CreateFileSystemURL("filesystem:foobar/file").is_valid());
}

}  // namespace fileapi
