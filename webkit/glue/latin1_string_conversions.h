// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_GLUE_LATIN1_STRING_CONVERSIONS_H_
#define WEBKIT_GLUE_LATIN1_STRING_CONVERSIONS_H_

#include <string>

#include "base/base_export.h"
#include "base/string16.h"
#include "webkit/glue/webkit_glue_export.h"

namespace webkit_glue {

// This definition of Latin1Char matches the definition of LChar in Blink. We
// use unsigned char rather than char to make less tempting to mix and match
// Latin-1 and UTF-8 characters..
typedef unsigned char Latin1Char;

// This somewhat odd function is designed to help us convert from Blink Strings
// to string16. A Blink string is either backed by an array of Latin-1
// characters or an array of UTF-16 characters. This function is called by
// WebString::operator string16() to convert one or the other character array
// to string16. This function is defined here rather than in WebString.h to
// avoid binary bloat in all the callers of the conversion operator.
WEBKIT_GLUE_EXPORT string16 Latin1OrUTF16ToUTF16(size_t length,
                                                 const Latin1Char* latin1,
                                                 const char16* utf16);

}  // namespace webkit_glue

#endif  // WEBKIT_GLUE_LATIN1_STRING_CONVERSIONS_H_
