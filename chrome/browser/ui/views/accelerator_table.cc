// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/accelerator_table.h"

#include "base/basictypes.h"
#include "chrome/app/chrome_command_ids.h"
#include "ui/base/events.h"
#include "ui/base/keycodes/keyboard_codes.h"

namespace browser {

// NOTE: Keep this list in the same (mostly-alphabetical) order as
// the Windows accelerators in ../../app/chrome_dll.rc.
// Do not use Ctrl-Alt as a shortcut modifier, as it is used by i18n keyboards:
// http://blogs.msdn.com/b/oldnewthing/archive/2004/03/29/101121.aspx
const AcceleratorMapping kAcceleratorMap[] = {
  { ui::VKEY_LEFT, ui::EF_ALT_DOWN, IDC_BACK },
  { ui::VKEY_BACK, ui::EF_NONE, IDC_BACK },
#if defined(OS_CHROMEOS)
  { ui::VKEY_F1, ui::EF_NONE, IDC_BACK },
#endif
  { ui::VKEY_D, ui::EF_CONTROL_DOWN, IDC_BOOKMARK_PAGE },
  { ui::VKEY_D, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_BOOKMARK_ALL_TABS },
#if !defined(OS_CHROMEOS)
  { ui::VKEY_DELETE, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_CLEAR_BROWSING_DATA },
#else
  { ui::VKEY_BACK, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_CLEAR_BROWSING_DATA },
#endif
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F4, ui::EF_CONTROL_DOWN, IDC_CLOSE_TAB },
#endif
  { ui::VKEY_W, ui::EF_CONTROL_DOWN, IDC_CLOSE_TAB },
  { ui::VKEY_W, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_CLOSE_WINDOW },
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F4, ui::EF_ALT_DOWN, IDC_CLOSE_WINDOW },
#endif
#if !defined(USE_ASH)
  { ui::VKEY_Q, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_EXIT },
#endif
  { ui::VKEY_F, ui::EF_CONTROL_DOWN, IDC_FIND },
  { ui::VKEY_G, ui::EF_CONTROL_DOWN, IDC_FIND_NEXT },
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F3, ui::EF_NONE, IDC_FIND_NEXT },
#endif
  { ui::VKEY_G, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_FIND_PREVIOUS },
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F3, ui::EF_SHIFT_DOWN, IDC_FIND_PREVIOUS },
#endif
  { ui::VKEY_D, ui::EF_ALT_DOWN, IDC_FOCUS_LOCATION },
  { ui::VKEY_L, ui::EF_CONTROL_DOWN, IDC_FOCUS_LOCATION },
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F10, ui::EF_NONE, IDC_FOCUS_MENU_BAR },
  { ui::VKEY_MENU, ui::EF_NONE, IDC_FOCUS_MENU_BAR },
  { ui::VKEY_F6, ui::EF_NONE, IDC_FOCUS_NEXT_PANE },
#else
  { ui::VKEY_F2, ui::EF_CONTROL_DOWN, IDC_FOCUS_NEXT_PANE },
#endif
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F6, ui::EF_SHIFT_DOWN, IDC_FOCUS_PREVIOUS_PANE },
#else
  { ui::VKEY_F1, ui::EF_CONTROL_DOWN, IDC_FOCUS_PREVIOUS_PANE },
#endif
  { ui::VKEY_K, ui::EF_CONTROL_DOWN, IDC_FOCUS_SEARCH },
  { ui::VKEY_E, ui::EF_CONTROL_DOWN, IDC_FOCUS_SEARCH },
  { ui::VKEY_BROWSER_SEARCH, ui::EF_NONE, IDC_FOCUS_SEARCH },
  { ui::VKEY_T, ui::EF_SHIFT_DOWN | ui::EF_ALT_DOWN, IDC_FOCUS_TOOLBAR },
  { ui::VKEY_B, ui::EF_SHIFT_DOWN | ui::EF_ALT_DOWN, IDC_FOCUS_BOOKMARKS },
  { ui::VKEY_RIGHT, ui::EF_ALT_DOWN, IDC_FORWARD },
  { ui::VKEY_BACK, ui::EF_SHIFT_DOWN, IDC_FORWARD },
#if defined(OS_CHROMEOS)
  { ui::VKEY_F2, ui::EF_NONE, IDC_FORWARD },
#endif
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F11, ui::EF_NONE, IDC_FULLSCREEN },
#else
  { ui::VKEY_F4, ui::EF_NONE, IDC_FULLSCREEN },
#endif
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F1, ui::EF_NONE, IDC_HELP_PAGE },
#else
  { ui::VKEY_OEM_2, ui::EF_CONTROL_DOWN, IDC_HELP_PAGE },
  { ui::VKEY_OEM_2, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_HELP_PAGE },
#endif
  { ui::VKEY_I, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_DEV_TOOLS },
#if defined(OS_CHROMEOS)
  { ui::VKEY_I, ui::EF_SHIFT_DOWN | ui::EF_ALT_DOWN, IDC_FEEDBACK },
#endif
  { ui::VKEY_F12, ui::EF_NONE, IDC_DEV_TOOLS },
  { ui::VKEY_J, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_DEV_TOOLS_CONSOLE },
  { ui::VKEY_C, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_DEV_TOOLS_INSPECT },
#if !defined(USE_ASH)
  { ui::VKEY_N, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_NEW_INCOGNITO_WINDOW },
  { ui::VKEY_T, ui::EF_CONTROL_DOWN, IDC_NEW_TAB },
  { ui::VKEY_N, ui::EF_CONTROL_DOWN, IDC_NEW_WINDOW },
#endif
  { ui::VKEY_O, ui::EF_CONTROL_DOWN, IDC_OPEN_FILE },
  { ui::VKEY_P, ui::EF_CONTROL_DOWN, IDC_PRINT},
  { ui::VKEY_P, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_ADVANCED_PRINT},
  { ui::VKEY_R, ui::EF_CONTROL_DOWN, IDC_RELOAD },
  { ui::VKEY_R, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_RELOAD_IGNORING_CACHE },
#if !defined(OS_CHROMEOS)
  { ui::VKEY_F5, ui::EF_NONE, IDC_RELOAD },
  { ui::VKEY_F5, ui::EF_CONTROL_DOWN, IDC_RELOAD_IGNORING_CACHE },
  { ui::VKEY_F5, ui::EF_SHIFT_DOWN, IDC_RELOAD_IGNORING_CACHE },
#else
  { ui::VKEY_F3, ui::EF_NONE, IDC_RELOAD },
  { ui::VKEY_F3, ui::EF_CONTROL_DOWN, IDC_RELOAD_IGNORING_CACHE },
  { ui::VKEY_F3, ui::EF_SHIFT_DOWN, IDC_RELOAD_IGNORING_CACHE },
#endif
  { ui::VKEY_HOME, ui::EF_ALT_DOWN, IDC_HOME },
#if !defined(USE_ASH)
  { ui::VKEY_T, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_RESTORE_TAB },
#endif
  { ui::VKEY_S, ui::EF_CONTROL_DOWN, IDC_SAVE_PAGE },
  { ui::VKEY_9, ui::EF_CONTROL_DOWN, IDC_SELECT_LAST_TAB },
  { ui::VKEY_NUMPAD9, ui::EF_CONTROL_DOWN, IDC_SELECT_LAST_TAB },
  { ui::VKEY_TAB, ui::EF_CONTROL_DOWN, IDC_SELECT_NEXT_TAB },
  { ui::VKEY_NEXT, ui::EF_CONTROL_DOWN, IDC_SELECT_NEXT_TAB },
  { ui::VKEY_TAB, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_SELECT_PREVIOUS_TAB },
  { ui::VKEY_PRIOR, ui::EF_CONTROL_DOWN, IDC_SELECT_PREVIOUS_TAB },
  { ui::VKEY_1, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_0 },
  { ui::VKEY_NUMPAD1, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_0 },
  { ui::VKEY_2, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_1 },
  { ui::VKEY_NUMPAD2, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_1 },
  { ui::VKEY_3, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_2 },
  { ui::VKEY_NUMPAD3, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_2 },
  { ui::VKEY_4, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_3 },
  { ui::VKEY_NUMPAD4, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_3 },
  { ui::VKEY_5, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_4 },
  { ui::VKEY_NUMPAD5, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_4 },
  { ui::VKEY_6, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_5 },
  { ui::VKEY_NUMPAD6, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_5 },
  { ui::VKEY_7, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_6 },
  { ui::VKEY_NUMPAD7, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_6 },
  { ui::VKEY_8, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_7 },
  { ui::VKEY_NUMPAD8, ui::EF_CONTROL_DOWN, IDC_SELECT_TAB_7 },
  { ui::VKEY_B, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_SHOW_BOOKMARK_BAR },
  { ui::VKEY_O, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_SHOW_BOOKMARK_MANAGER },
  { ui::VKEY_J, ui::EF_CONTROL_DOWN, IDC_SHOW_DOWNLOADS },
  { ui::VKEY_H, ui::EF_CONTROL_DOWN, IDC_SHOW_HISTORY },
  { ui::VKEY_F, ui::EF_ALT_DOWN, IDC_SHOW_APP_MENU},
  { ui::VKEY_E, ui::EF_ALT_DOWN, IDC_SHOW_APP_MENU},
#if !defined(OS_CHROMEOS)
  { ui::VKEY_M, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_SHOW_AVATAR_MENU},
#endif
  { ui::VKEY_ESCAPE, ui::EF_NONE, IDC_STOP },
#if !defined(USE_ASH)
  { ui::VKEY_ESCAPE, ui::EF_SHIFT_DOWN, IDC_TASK_MANAGER },
#endif
  { ui::VKEY_OEM_PERIOD, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_TOGGLE_SPEECH_INPUT },
  { ui::VKEY_U, ui::EF_CONTROL_DOWN, IDC_VIEW_SOURCE },
  { ui::VKEY_OEM_MINUS, ui::EF_CONTROL_DOWN, IDC_ZOOM_MINUS },
  { ui::VKEY_OEM_MINUS, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN,
    IDC_ZOOM_MINUS },
  { ui::VKEY_SUBTRACT, ui::EF_CONTROL_DOWN, IDC_ZOOM_MINUS },
  { ui::VKEY_0, ui::EF_CONTROL_DOWN, IDC_ZOOM_NORMAL },
  { ui::VKEY_NUMPAD0, ui::EF_CONTROL_DOWN, IDC_ZOOM_NORMAL },
  { ui::VKEY_OEM_PLUS, ui::EF_CONTROL_DOWN, IDC_ZOOM_PLUS },
  { ui::VKEY_OEM_PLUS, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN, IDC_ZOOM_PLUS },
  { ui::VKEY_ADD, ui::EF_CONTROL_DOWN, IDC_ZOOM_PLUS },
};

const size_t kAcceleratorMapLength = arraysize(kAcceleratorMap);

}  // namespace browser
