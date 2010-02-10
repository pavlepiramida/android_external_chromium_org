// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/browser_list.h"

#include <gtk/gtk.h>

// static
void BrowserList::AllBrowsersClosed() {
  // Close non-browser windows.
  GList* window_list = gtk_window_list_toplevels();
  g_list_foreach(window_list, (GFunc)g_object_ref, NULL);
  for (GList* i = window_list; i != NULL; i = g_list_next(i)) {
    GtkWindow* window = GTK_WINDOW(i->data);
    // We filter by visible widgets because there are toplevel windows that if
    // we try to destroy, we crash.  For example, trying to destroy the tooltip
    // window or the toplevel associated with drop down windows crashes.
    if (GTK_WIDGET_VISIBLE(GTK_WIDGET(window)))
      gtk_widget_destroy(GTK_WIDGET(window));
  }
  g_list_foreach(window_list, (GFunc)g_object_unref, NULL);
  g_list_free(window_list);
}
