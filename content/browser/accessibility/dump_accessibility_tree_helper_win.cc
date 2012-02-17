// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/accessibility/dump_accessibility_tree_helper.h"

#include <oleacc.h>

#include <map>
#include <string>

#include "base/file_path.h"
#include "base/string_util.h"
#include "content/browser/accessibility/browser_accessibility_win.h"
#include "third_party/iaccessible2/ia2_api_all.h"

namespace {
std::map<int32, string16> role_string_map;
std::map<int32, string16> state_string_map;
} // namespace

// Convenience macros for generating readable strings.
#define ROLE_MAP(x) role_string_map[x] = L#x;
#define STATE_MAP(x) state_string_map[x] = L#x;

void DumpAccessibilityTreeHelper::Initialize() {
  ROLE_MAP(IA2_ROLE_UNKNOWN)
  ROLE_MAP(IA2_ROLE_CANVAS)
  ROLE_MAP(IA2_ROLE_CAPTION)
  ROLE_MAP(IA2_ROLE_CHECK_MENU_ITEM)
  ROLE_MAP(IA2_ROLE_COLOR_CHOOSER)
  ROLE_MAP(IA2_ROLE_DATE_EDITOR)
  ROLE_MAP(IA2_ROLE_DESKTOP_ICON)
  ROLE_MAP(IA2_ROLE_DESKTOP_PANE)
  ROLE_MAP(IA2_ROLE_DIRECTORY_PANE)
  ROLE_MAP(IA2_ROLE_EDITBAR)
  ROLE_MAP(IA2_ROLE_EMBEDDED_OBJECT)
  ROLE_MAP(IA2_ROLE_ENDNOTE)
  ROLE_MAP(IA2_ROLE_FILE_CHOOSER)
  ROLE_MAP(IA2_ROLE_FONT_CHOOSER)
  ROLE_MAP(IA2_ROLE_FOOTER)
  ROLE_MAP(IA2_ROLE_FOOTNOTE)
  ROLE_MAP(IA2_ROLE_FORM)
  ROLE_MAP(IA2_ROLE_FRAME)
  ROLE_MAP(IA2_ROLE_GLASS_PANE)
  ROLE_MAP(IA2_ROLE_HEADER)
  ROLE_MAP(IA2_ROLE_HEADING)
  ROLE_MAP(IA2_ROLE_ICON)
  ROLE_MAP(IA2_ROLE_IMAGE_MAP)
  ROLE_MAP(IA2_ROLE_INPUT_METHOD_WINDOW)
  ROLE_MAP(IA2_ROLE_INTERNAL_FRAME)
  ROLE_MAP(IA2_ROLE_LABEL)
  ROLE_MAP(IA2_ROLE_LAYERED_PANE)
  ROLE_MAP(IA2_ROLE_NOTE)
  ROLE_MAP(IA2_ROLE_OPTION_PANE)
  ROLE_MAP(IA2_ROLE_PAGE)
  ROLE_MAP(IA2_ROLE_PARAGRAPH)
  ROLE_MAP(IA2_ROLE_RADIO_MENU_ITEM)
  ROLE_MAP(IA2_ROLE_REDUNDANT_OBJECT)
  ROLE_MAP(IA2_ROLE_ROOT_PANE)
  ROLE_MAP(IA2_ROLE_RULER)
  ROLE_MAP(IA2_ROLE_SCROLL_PANE)
  ROLE_MAP(IA2_ROLE_SECTION)
  ROLE_MAP(IA2_ROLE_SHAPE)
  ROLE_MAP(IA2_ROLE_SPLIT_PANE)
  ROLE_MAP(IA2_ROLE_TEAR_OFF_MENU)
  ROLE_MAP(IA2_ROLE_TERMINAL)
  ROLE_MAP(IA2_ROLE_TEXT_FRAME)
  ROLE_MAP(IA2_ROLE_TOGGLE_BUTTON)
  ROLE_MAP(IA2_ROLE_VIEW_PORT)

  // MSAA roles.
  ROLE_MAP(ROLE_SYSTEM_TITLEBAR)
  ROLE_MAP(ROLE_SYSTEM_MENUBAR)
  ROLE_MAP(ROLE_SYSTEM_SCROLLBAR)
  ROLE_MAP(ROLE_SYSTEM_GRIP)
  ROLE_MAP(ROLE_SYSTEM_SOUND)
  ROLE_MAP(ROLE_SYSTEM_CURSOR)
  ROLE_MAP(ROLE_SYSTEM_CARET)
  ROLE_MAP(ROLE_SYSTEM_ALERT)
  ROLE_MAP(ROLE_SYSTEM_WINDOW)
  ROLE_MAP(ROLE_SYSTEM_CLIENT)
  ROLE_MAP(ROLE_SYSTEM_MENUPOPUP)
  ROLE_MAP(ROLE_SYSTEM_MENUITEM)
  ROLE_MAP(ROLE_SYSTEM_TOOLTIP)
  ROLE_MAP(ROLE_SYSTEM_APPLICATION)
  ROLE_MAP(ROLE_SYSTEM_DOCUMENT)
  ROLE_MAP(ROLE_SYSTEM_PANE)
  ROLE_MAP(ROLE_SYSTEM_CHART)
  ROLE_MAP(ROLE_SYSTEM_DIALOG)
  ROLE_MAP(ROLE_SYSTEM_BORDER)
  ROLE_MAP(ROLE_SYSTEM_GROUPING)
  ROLE_MAP(ROLE_SYSTEM_SEPARATOR)
  ROLE_MAP(ROLE_SYSTEM_TOOLBAR)
  ROLE_MAP(ROLE_SYSTEM_STATUSBAR)
  ROLE_MAP(ROLE_SYSTEM_TABLE)
  ROLE_MAP(ROLE_SYSTEM_COLUMNHEADER)
  ROLE_MAP(ROLE_SYSTEM_ROWHEADER)
  ROLE_MAP(ROLE_SYSTEM_COLUMN)
  ROLE_MAP(ROLE_SYSTEM_ROW)
  ROLE_MAP(ROLE_SYSTEM_CELL)
  ROLE_MAP(ROLE_SYSTEM_LINK)
  ROLE_MAP(ROLE_SYSTEM_HELPBALLOON)
  ROLE_MAP(ROLE_SYSTEM_CHARACTER)
  ROLE_MAP(ROLE_SYSTEM_LIST)
  ROLE_MAP(ROLE_SYSTEM_LISTITEM)
  ROLE_MAP(ROLE_SYSTEM_OUTLINE)
  ROLE_MAP(ROLE_SYSTEM_OUTLINEITEM)
  ROLE_MAP(ROLE_SYSTEM_PAGETAB)
  ROLE_MAP(ROLE_SYSTEM_PROPERTYPAGE)
  ROLE_MAP(ROLE_SYSTEM_INDICATOR)
  ROLE_MAP(ROLE_SYSTEM_GRAPHIC)
  ROLE_MAP(ROLE_SYSTEM_STATICTEXT)
  ROLE_MAP(ROLE_SYSTEM_TEXT)
  ROLE_MAP(ROLE_SYSTEM_PUSHBUTTON)
  ROLE_MAP(ROLE_SYSTEM_CHECKBUTTON)
  ROLE_MAP(ROLE_SYSTEM_RADIOBUTTON)
  ROLE_MAP(ROLE_SYSTEM_COMBOBOX)
  ROLE_MAP(ROLE_SYSTEM_DROPLIST)
  ROLE_MAP(ROLE_SYSTEM_PROGRESSBAR)
  ROLE_MAP(ROLE_SYSTEM_DIAL)
  ROLE_MAP(ROLE_SYSTEM_HOTKEYFIELD)
  ROLE_MAP(ROLE_SYSTEM_SLIDER)
  ROLE_MAP(ROLE_SYSTEM_SPINBUTTON)
  ROLE_MAP(ROLE_SYSTEM_DIAGRAM)
  ROLE_MAP(ROLE_SYSTEM_ANIMATION)
  ROLE_MAP(ROLE_SYSTEM_EQUATION)
  ROLE_MAP(ROLE_SYSTEM_BUTTONDROPDOWN)
  ROLE_MAP(ROLE_SYSTEM_BUTTONMENU)
  ROLE_MAP(ROLE_SYSTEM_BUTTONDROPDOWNGRID)
  ROLE_MAP(ROLE_SYSTEM_WHITESPACE)
  ROLE_MAP(ROLE_SYSTEM_PAGETABLIST)
  ROLE_MAP(ROLE_SYSTEM_CLOCK)
  ROLE_MAP(ROLE_SYSTEM_SPLITBUTTON)
  ROLE_MAP(ROLE_SYSTEM_IPADDRESS)
  ROLE_MAP(ROLE_SYSTEM_OUTLINEBUTTON)

  STATE_MAP(IA2_STATE_ACTIVE)
  STATE_MAP(IA2_STATE_ARMED)
  STATE_MAP(IA2_STATE_DEFUNCT)
  STATE_MAP(IA2_STATE_EDITABLE)
  STATE_MAP(IA2_STATE_HORIZONTAL)
  STATE_MAP(IA2_STATE_ICONIFIED)
  STATE_MAP(IA2_STATE_INVALID_ENTRY)
  STATE_MAP(IA2_STATE_MANAGES_DESCENDANTS)
  STATE_MAP(IA2_STATE_MODAL)
  STATE_MAP(IA2_STATE_MULTI_LINE)
  STATE_MAP(IA2_STATE_OPAQUE)
  STATE_MAP(IA2_STATE_REQUIRED)
  STATE_MAP(IA2_STATE_SELECTABLE_TEXT)
  STATE_MAP(IA2_STATE_SINGLE_LINE)
  STATE_MAP(IA2_STATE_STALE)
  STATE_MAP(IA2_STATE_SUPPORTS_AUTOCOMPLETION)
  STATE_MAP(IA2_STATE_TRANSIENT)
  STATE_MAP(IA2_STATE_VERTICAL)
}

string16 DumpAccessibilityTreeHelper::ToString(BrowserAccessibility* node) {
  if (role_string_map.empty())
    Initialize();

  BrowserAccessibilityWin* acc_obj = node->toBrowserAccessibilityWin();
  string16 state;
  std::map<int32, string16>::iterator it;

  for (it = state_string_map.begin(); it != state_string_map.end(); ++it) {
    if (it->first & acc_obj->ia2_state())
      state += L"|" + state_string_map[it->first];
  }
  return acc_obj->name() + L"|" + role_string_map[acc_obj->ia2_role()] + L"|" +
      state;
}

const FilePath::StringType DumpAccessibilityTreeHelper::GetActualFileSuffix()
    const {
  return FILE_PATH_LITERAL("-actual-win.txt");
}

const FilePath::StringType DumpAccessibilityTreeHelper::GetExpectedFileSuffix()
    const {
  return FILE_PATH_LITERAL("-expected-win.txt");
}
