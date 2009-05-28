// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_GTK_OPTIONS_OPTIONS_LAYOUT_GTK_H_
#define CHROME_BROWSER_GTK_OPTIONS_OPTIONS_LAYOUT_GTK_H_

#include <gtk/gtk.h>
#include <string>

#include "base/basictypes.h"

class OptionsLayoutBuilderGtk {
 public:
  explicit OptionsLayoutBuilderGtk(int num_rows);

  GtkWidget* get_page_widget() {
    return page_;
  }

  // Adds an option group to the table.  Handles layout and the placing of
  // separators between groups.
  void AddOptionGroup(const std::string& title, GtkWidget* content);

 private:
  // The parent widget
  GtkWidget* page_;

  DISALLOW_COPY_AND_ASSIGN(OptionsLayoutBuilderGtk);
};

#endif  // CHROME_BROWSER_GTK_OPTIONS_OPTIONS_LAYOUT_GTK_H_
