// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_TEST_TEST_VIEWS_DELEGATE_H_
#define VIEWS_TEST_TEST_VIEWS_DELEGATE_H_

#include "base/memory/scoped_ptr.h"
#include "ui/base/accessibility/accessibility_types.h"
#include "ui/base/clipboard/clipboard.h"
#include "views/views_delegate.h"

// TODO(beng): this whole file should be in the views namespace.
namespace views {
class View;
class Widget;
}

class TestViewsDelegate : public views::ViewsDelegate {
 public:
  TestViewsDelegate();
  virtual ~TestViewsDelegate();

  void set_default_parent_view(views::View* view) {
    default_parent_view_ = view;
  }

  // Overridden from views::ViewsDelegate:
  virtual ui::Clipboard* GetClipboard() const OVERRIDE;
  virtual views::View* GetDefaultParentView() OVERRIDE;
  virtual void SaveWindowPlacement(const views::Widget* window,
                                   const std::wstring& window_name,
                                   const gfx::Rect& bounds,
                                   bool maximized) OVERRIDE {}
  virtual bool GetSavedWindowBounds(const std::wstring& window_name,
                                    gfx::Rect* bounds) const OVERRIDE;

  virtual bool GetSavedMaximizedState(const std::wstring& window_name,
                                      bool* maximized) const OVERRIDE;

  virtual void NotifyAccessibilityEvent(
      views::View* view, ui::AccessibilityTypes::Event event_type) OVERRIDE {}

  virtual void NotifyMenuItemFocused(
      const std::wstring& menu_name,
      const std::wstring& menu_item_name,
      int item_index,
      int item_count,
      bool has_submenu) OVERRIDE {}
#if defined(OS_WIN)
  virtual HICON GetDefaultWindowIcon() const OVERRIDE {
    return NULL;
  }
#endif

  virtual void AddRef() OVERRIDE {}
  virtual void ReleaseRef() OVERRIDE {}

  virtual int GetDispositionForEvent(int event_flags) OVERRIDE;

 private:
  views::View* default_parent_view_;
  mutable scoped_ptr<ui::Clipboard> clipboard_;

  DISALLOW_COPY_AND_ASSIGN(TestViewsDelegate);
};

#endif  // VIEWS_TEST_TEST_VIEWS_DELEGATE_H_
