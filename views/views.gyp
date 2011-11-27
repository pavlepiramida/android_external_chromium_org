# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
  },

  'target_defaults': {
    'conditions': [
      ['OS=="win"', {'sources/': [
        ['include', '_(win)\\.cc$'],
        ['include', '/win/'],
        ['include', '/win_[^/]*\\.cc$'],
      ]}],
      ['use_aura==1', {
        'sources/': [ ['exclude', '_win\\.(h|cc)$'],
                      ['exclude', '_gtk\\.(h|cc)$'],
                      ['exclude', '_x\\.(h|cc)$'] ],
        'dependencies': [ '../ui/aura/aura.gyp:aura', ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'views',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/base/strings/ui_strings.gyp:ui_strings',
        '../ui/gfx/compositor/compositor.gyp:compositor',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
      ],
      'defines': [
        'VIEWS_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under views, except unittests
        'accessible_pane_view.cc',
        'accessible_pane_view.h',
        'background.cc',
        'background.h',
        'border.cc',
        'border.h',
        'context_menu_controller.h',
        'controls/button/button.cc',
        'controls/button/button.h',
        'controls/button/button_dropdown.cc',
        'controls/button/button_dropdown.h',
        'controls/button/checkbox.cc',
        'controls/button/checkbox.h',
        'controls/button/custom_button.cc',
        'controls/button/custom_button.h',
        'controls/button/image_button.cc',
        'controls/button/image_button.h',
        'controls/button/menu_button.cc',
        'controls/button/menu_button.h',
        'controls/button/radio_button.cc',
        'controls/button/radio_button.h',
        'controls/button/text_button.cc',
        'controls/button/text_button.h',
        'controls/focusable_border.cc',
        'controls/focusable_border.h',
        'controls/image_view.cc',
        'controls/image_view.h',
        'controls/label.cc',
        'controls/label.h',
        'controls/link.cc',
        'controls/link.h',
        'controls/link_listener.h',
        'controls/menu/menu.cc',
        'controls/menu/menu.h',
        'controls/menu/menu_2.cc',
        'controls/menu/menu_2.h',
        'controls/menu/menu_config.cc',
        'controls/menu/menu_config.h',
        'controls/menu/menu_config_aura.cc',
        'controls/menu/menu_config_linux.cc',
        'controls/menu/menu_config_win.cc',
        'controls/menu/menu_controller.cc',
        'controls/menu/menu_controller.h',
        'controls/menu/menu_controller_delegate.h',
        'controls/menu/menu_delegate.cc',
        'controls/menu/menu_delegate.h',
        'controls/menu/menu_gtk.cc',
        'controls/menu/menu_gtk.h',
        'controls/menu/menu_host.cc',
        'controls/menu/menu_host.h',
        'controls/menu/menu_host_root_view.cc',
        'controls/menu/menu_host_root_view.h',
        'controls/menu/menu_item_view.cc',
        'controls/menu/menu_item_view.h',
        'controls/menu/menu_item_view_aura.cc',
        'controls/menu/menu_item_view_linux.cc',
        'controls/menu/menu_item_view_win.cc',
        'controls/menu/menu_listener.h',
        'controls/menu/menu_model_adapter.cc',
        'controls/menu/menu_model_adapter.h',
        'controls/menu/menu_runner.cc',
        'controls/menu/menu_runner.h',
        'controls/menu/menu_scroll_view_container.cc',
        'controls/menu/menu_scroll_view_container.h',
        'controls/menu/menu_separator.h',
        'controls/menu/menu_separator_aura.cc',
        'controls/menu/menu_separator_linux.cc',
        'controls/menu/menu_separator_win.cc',
        'controls/menu/menu_win.cc',
        'controls/menu/menu_win.h',
        'controls/menu/menu_wrapper.h',
        'controls/menu/native_menu_gtk.cc',
        'controls/menu/native_menu_gtk.h',
        'controls/menu/native_menu_host.h',
        'controls/menu/native_menu_host_delegate.h',
        'controls/menu/native_menu_views.cc',
        'controls/menu/native_menu_views.h',
        'controls/menu/native_menu_win.cc',
        'controls/menu/native_menu_win.h',
        'controls/menu/nested_dispatcher_gtk.cc',
        'controls/menu/nested_dispatcher_gtk.h',
        'controls/menu/menu_image_util.cc',
        'controls/menu/menu_image_util.h',
        'controls/menu/submenu_view.cc',
        'controls/menu/submenu_view.h',
        'controls/menu/view_menu_delegate.h',
        'controls/message_box_view.cc',
        'controls/message_box_view.h',
        'controls/native_control.cc',
        'controls/native_control.h',
        'controls/native_control_gtk.cc',
        'controls/native_control_gtk.h',
        'controls/native_control_win.cc',
        'controls/native_control_win.h',
        'controls/progress_bar.h',
        'controls/progress_bar.cc',
        'controls/resize_area.cc',
        'controls/resize_area.h',
        'controls/resize_area_delegate.h',
        'controls/scroll_view.cc',
        'controls/scroll_view.h',
        'controls/separator.cc',
        'controls/separator.h',
        'controls/single_split_view.cc',
        'controls/single_split_view.h',
        'controls/single_split_view_listener.h',
        'controls/throbber.cc',
        'controls/throbber.h',
        #'debug_utils.cc',
        #'debug_utils.h',
        'drag_controller.h',
        'drag_utils.cc',
        'drag_utils.h',
        'drag_utils_aura.cc',
        'drag_utils_gtk.cc',
        'drag_utils_win.cc',
        'metrics.cc',
        'metrics.h',
        'metrics_aura.cc',
        'metrics_gtk.cc',
        'metrics_win.cc',
        'mouse_watcher.cc',
        'mouse_watcher.h',
        'native_theme_delegate.h',
        'native_theme_painter.cc',
        'native_theme_painter.h',
        'paint_lock.cc',
        'paint_lock.h',
        'painter.cc',
        'painter.h',
        'repeat_controller.cc',
        'repeat_controller.h',
        'view.cc',
        'view.h',
        'view_aura.cc',
        'view_constants.cc',
        'view_constants.h',
        'view_gtk.cc',
        'view_text_utils.cc',
        'view_text_utils.h',
        'view_win.cc',
        'views_delegate.h',
        '../ui/views/accessibility/native_view_accessibility_win.cc',
        '../ui/views/accessibility/native_view_accessibility_win.h',
        '../ui/views/animation/bounds_animator.cc',
        '../ui/views/animation/bounds_animator.h',
        '../ui/views/bubble/border_contents_view.cc',
        '../ui/views/bubble/border_contents_view.h',
        '../ui/views/bubble/bubble_border.cc',
        '../ui/views/bubble/bubble_border.h',
        '../ui/views/bubble/bubble_delegate.cc',
        '../ui/views/bubble/bubble_delegate.h',
        '../ui/views/bubble/bubble_frame_view.cc',
        '../ui/views/bubble/bubble_frame_view.h',
        '../ui/views/controls/combobox/combobox.cc',
        '../ui/views/controls/combobox/combobox.h',
        '../ui/views/controls/combobox/combobox_listener.h',
        '../ui/views/controls/combobox/native_combobox_gtk.cc',
        '../ui/views/controls/combobox/native_combobox_gtk.h',
        '../ui/views/controls/combobox/native_combobox_views.cc',
        '../ui/views/controls/combobox/native_combobox_views.h',
        '../ui/views/controls/combobox/native_combobox_win.cc',
        '../ui/views/controls/combobox/native_combobox_win.h',
        '../ui/views/controls/combobox/native_combobox_wrapper.h',
        '../ui/views/controls/native/native_view_host.cc',
        '../ui/views/controls/native/native_view_host.h',
        '../ui/views/controls/native/native_view_host_aura.cc',
        '../ui/views/controls/native/native_view_host_aura.h',
        '../ui/views/controls/native/native_view_host_gtk.cc',
        '../ui/views/controls/native/native_view_host_gtk.h',
        '../ui/views/controls/native/native_view_host_win.cc',
        '../ui/views/controls/native/native_view_host_win.h',
        '../ui/views/controls/scrollbar/base_scroll_bar.cc',
        '../ui/views/controls/scrollbar/base_scroll_bar.h',
        '../ui/views/controls/scrollbar/base_scroll_bar_button.cc',
        '../ui/views/controls/scrollbar/base_scroll_bar_button.h',
        '../ui/views/controls/scrollbar/base_scroll_bar_thumb.cc',
        '../ui/views/controls/scrollbar/base_scroll_bar_thumb.h',
        '../ui/views/controls/scrollbar/bitmap_scroll_bar.cc',
        '../ui/views/controls/scrollbar/bitmap_scroll_bar.h',
        '../ui/views/controls/scrollbar/native_scroll_bar_gtk.cc',
        '../ui/views/controls/scrollbar/native_scroll_bar_gtk.h',
        '../ui/views/controls/scrollbar/native_scroll_bar_views.cc',
        '../ui/views/controls/scrollbar/native_scroll_bar_views.h',
        '../ui/views/controls/scrollbar/native_scroll_bar_win.cc',
        '../ui/views/controls/scrollbar/native_scroll_bar_win.h',
        '../ui/views/controls/scrollbar/native_scroll_bar_wrapper.h',
        '../ui/views/controls/scrollbar/native_scroll_bar.cc',
        '../ui/views/controls/scrollbar/native_scroll_bar.h',
        '../ui/views/controls/scrollbar/scroll_bar.cc',
        '../ui/views/controls/scrollbar/scroll_bar.h',
        '../ui/views/controls/tabbed_pane/native_tabbed_pane_gtk.cc',
        '../ui/views/controls/tabbed_pane/native_tabbed_pane_gtk.h',
        '../ui/views/controls/tabbed_pane/native_tabbed_pane_win.cc',
        '../ui/views/controls/tabbed_pane/native_tabbed_pane_win.h',
        '../ui/views/controls/tabbed_pane/native_tabbed_pane_wrapper.h',
        '../ui/views/controls/tabbed_pane/tabbed_pane.cc',
        '../ui/views/controls/tabbed_pane/tabbed_pane.h',
        '../ui/views/controls/tabbed_pane/tabbed_pane_listener.h',
        '../ui/views/controls/table/native_table_wrapper.h',
        '../ui/views/controls/table/native_table_gtk.cc',
        '../ui/views/controls/table/native_table_gtk.h',
        '../ui/views/controls/table/native_table_win.cc',
        '../ui/views/controls/table/native_table_win.h',
        '../ui/views/controls/table/group_table_view.cc',
        '../ui/views/controls/table/group_table_view.h',
        '../ui/views/controls/table/table_view.cc',
        '../ui/views/controls/table/table_view.h',
        '../ui/views/controls/table/table_view2.cc',
        '../ui/views/controls/table/table_view2.h',
        '../ui/views/controls/table/table_view_observer.h',
        '../ui/views/controls/textfield/gtk_views_entry.cc',
        '../ui/views/controls/textfield/gtk_views_entry.h',
        '../ui/views/controls/textfield/gtk_views_textview.cc',
        '../ui/views/controls/textfield/gtk_views_textview.h',
        '../ui/views/controls/textfield/native_textfield_gtk.cc',
        '../ui/views/controls/textfield/native_textfield_gtk.h',
        '../ui/views/controls/textfield/native_textfield_views.cc',
        '../ui/views/controls/textfield/native_textfield_views.h',
        '../ui/views/controls/textfield/native_textfield_win.cc',
        '../ui/views/controls/textfield/native_textfield_win.h',
        '../ui/views/controls/textfield/native_textfield_wrapper.h',
        '../ui/views/controls/textfield/textfield.cc',
        '../ui/views/controls/textfield/textfield.h',
        '../ui/views/controls/textfield/textfield_controller.h',
        '../ui/views/controls/textfield/textfield_views_model.cc',
        '../ui/views/controls/textfield/textfield_views_model.h',
        '../ui/views/controls/tree/tree_view.cc',
        '../ui/views/controls/tree/tree_view.h',
        '../ui/views/events/event.cc',
        '../ui/views/events/event.h',
        '../ui/views/events/event_aura.cc',
        '../ui/views/events/event_gtk.cc',
        '../ui/views/events/event_win.cc',
        '../ui/views/events/event_x.cc',
        '../ui/views/focus/accelerator_handler.h',
        '../ui/views/focus/accelerator_handler_aura.cc',
        '../ui/views/focus/accelerator_handler_gtk.cc',
        '../ui/views/focus/accelerator_handler_win.cc',
        '../ui/views/focus/external_focus_tracker.cc',
        '../ui/views/focus/external_focus_tracker.h',
        '../ui/views/focus/focus_manager.cc',
        '../ui/views/focus/focus_manager.h',
        '../ui/views/focus/focus_manager_factory.cc',
        '../ui/views/focus/focus_manager_factory.h',
        '../ui/views/focus/focus_search.cc',
        '../ui/views/focus/focus_search.h',
        '../ui/views/focus/view_storage.cc',
        '../ui/views/focus/view_storage.h',
        '../ui/views/focus/widget_focus_manager.cc',
        '../ui/views/focus/widget_focus_manager.h',
        '../ui/views/ime/input_method_base.cc',
        '../ui/views/ime/input_method_base.h',
        '../ui/views/ime/input_method_delegate.h',
        '../ui/views/ime/input_method_gtk.cc',
        '../ui/views/ime/input_method_gtk.h',
        '../ui/views/ime/input_method.h',
        '../ui/views/ime/input_method_ibus.cc',
        '../ui/views/ime/input_method_ibus.h',
        '../ui/views/ime/input_method_win.cc',
        '../ui/views/ime/input_method_win.h',
        '../ui/views/ime/mock_input_method.cc',
        '../ui/views/ime/mock_input_method.h',
        '../ui/views/ime/text_input_type_tracker.cc',
        '../ui/views/ime/text_input_type_tracker.h',
        '../ui/views/layout/box_layout.cc',
        '../ui/views/layout/box_layout.h',
        '../ui/views/layout/fill_layout.cc',
        '../ui/views/layout/fill_layout.h',
        '../ui/views/layout/grid_layout.cc',
        '../ui/views/layout/grid_layout.h',
        '../ui/views/layout/layout_constants.h',
        '../ui/views/layout/layout_manager.cc',
        '../ui/views/layout/layout_manager.h',
        '../ui/views/touchui/gesture_manager.cc',
        '../ui/views/touchui/gesture_manager.h',
        '../ui/views/touchui/touch_selection_controller.cc',
        '../ui/views/touchui/touch_selection_controller.h',
        '../ui/views/widget/aero_tooltip_manager.cc',
        '../ui/views/widget/aero_tooltip_manager.h',
        '../ui/views/widget/child_window_message_processor.cc',
        '../ui/views/widget/child_window_message_processor.h',
        '../ui/views/widget/default_theme_provider.cc',
        '../ui/views/widget/default_theme_provider.h',
        '../ui/views/widget/drop_helper.cc',
        '../ui/views/widget/drop_helper.h',
        '../ui/views/widget/drop_target_gtk.cc',
        '../ui/views/widget/drop_target_gtk.h',
        '../ui/views/widget/drop_target_win.cc',
        '../ui/views/widget/drop_target_win.h',
        '../ui/views/widget/gtk_views_fixed.cc',
        '../ui/views/widget/gtk_views_fixed.h',
        '../ui/views/widget/gtk_views_window.cc',
        '../ui/views/widget/gtk_views_window.h',
        '../ui/views/widget/root_view.cc',
        '../ui/views/widget/root_view.h',
        '../ui/views/widget/tooltip_manager_gtk.cc',
        '../ui/views/widget/tooltip_manager_gtk.h',
        '../ui/views/widget/tooltip_manager_views.cc',
        '../ui/views/widget/tooltip_manager_views.h',
        '../ui/views/widget/tooltip_manager_win.cc',
        '../ui/views/widget/tooltip_manager_win.h',
        '../ui/views/widget/tooltip_manager.cc',
        '../ui/views/widget/tooltip_manager.h',
        '../ui/views/widget/monitor_win.cc',
        '../ui/views/widget/monitor_win.h',
        '../ui/views/widget/native_widget.h',
        '../ui/views/widget/native_widget_aura.cc',
        '../ui/views/widget/native_widget_aura.h',
        '../ui/views/widget/native_widget_delegate.h',
        '../ui/views/widget/native_widget_private.h',
        '../ui/views/widget/native_widget_gtk.cc',
        '../ui/views/widget/native_widget_gtk.h',
        '../ui/views/widget/native_widget_win.cc',
        '../ui/views/widget/native_widget_win.h',
        '../ui/views/widget/widget.cc',
        '../ui/views/widget/widget.h',
        '../ui/views/widget/widget_delegate.cc',
        '../ui/views/widget/widget_delegate.h',
        '../ui/views/widget/window_manager.cc',
        '../ui/views/widget/window_manager.h',
        '../ui/views/window/client_view.cc',
        '../ui/views/window/client_view.h',
        '../ui/views/window/custom_frame_view.cc',
        '../ui/views/window/window/custom_frame_view.h',
        '../ui/views/window/dialog_client_view.cc',
        '../ui/views/window/window/dialog_client_view.h',
        '../ui/views/window/dialog_delegate.cc',
        '../ui/views/window/dialog_delegate.h',
        '../ui/views/window/native_frame_view.cc',
        '../ui/views/window/native_frame_view.h',
        '../ui/views/window/non_client_view.cc',
        '../ui/views/window/non_client_view.h',
        '../ui/views/window/window_resources.h',
        '../ui/views/window/window_shape.cc',
        '../ui/views/window/window_shape.h',
      ],
      'include_dirs': [
        '../third_party/wtl/include',
      ],
      'conditions': [
        ['use_aura==1', {
          'sources/': [
            ['exclude', '_(gtk|x)\\.cc$'],
            ['exclude', '/(gtk|x)_[^/]*\\.cc$'],
            ['exclude', 'controls/menu/menu_2.*'],
          ],
          'sources!': [
            'controls/menu/menu_config_linux.cc',
            'controls/menu/menu_item_view_linux.cc',
            'controls/menu/menu_separator_linux.cc',
            'controls/native_control.cc',
            'controls/native_control.h',
            'drag_utils_win.cc',
            '../ui/views/controls/scrollbar/bitmap_scroll_bar.cc',
            '../ui/views/controls/scrollbar/bitmap_scroll_bar.h',
            '../ui/views/controls/tabbed_pane/tabbed_pane.cc',
            '../ui/views/controls/tabbed_pane/tabbed_pane.h',
            '../ui/views/controls/table/group_table_view.cc',
            '../ui/views/controls/table/group_table_view.h',
            '../ui/views/controls/table/native_table_wrapper.h',
            '../ui/views/controls/table/table_view.cc',
            '../ui/views/controls/table/table_view.h',
            '../ui/views/controls/table/table_view2.cc',
            '../ui/views/controls/table/table_view2.h',
            '../ui/views/controls/table/table_view_observer.h',
            '../ui/views/controls/tree/tree_view.cc',
            '../ui/views/controls/tree/tree_view.h',
            '../ui/views/widget/aero_tooltip_manager.cc',
            '../ui/views/widget/aero_tooltip_manager.h',
            '../ui/views/widget/child_window_message_processor.cc',
            '../ui/views/widget/child_window_message_processor.h',
          ],
        },
        ],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
            '../build/linux/system.gyp:x11',
            '../build/linux/system.gyp:xext',
          ],
          'sources!': [
            'controls/native_control.cc',
            '../ui/views/accessibility/native_view_accessibility_win.cc',
            '../ui/views/controls/scrollbar/bitmap_scroll_bar.cc',
            '../ui/views/controls/table/group_table_view.cc',
            '../ui/views/controls/table/table_view.cc',
            '../ui/views/controls/tree/tree_view.cc',
            '../ui/views/events/event_win.cc',
            '../ui/views/widget/aero_tooltip_manager.cc',
            '../ui/views/widget/child_window_message_processor.cc',
            '../ui/views/widget/child_window_message_processor.h',
            '../ui/views/widget/native_widget_win.cc',
          ],
        }],
        ['use_aura==0', {
          'sources!': [
            'controls/menu/native_menu_views.cc',
            'controls/menu/native_menu_views.h',
            '../ui/views/widget/tooltip_manager_views.cc',
          ],
        }],
        ['use_ibus==1', {
          'dependencies': [
            '../build/linux/system.gyp:ibus',
          ],
          'sources/': [
            ['exclude', '../ui/views/ime/mock_input_method.cc'],
            ['exclude', '../ui/views/ime/mock_input_method.h'],
          ],
        }, { # else: use_ibus != 1
          'sources/': [
            ['exclude', '../ui/views/ime/input_method_ibus.cc'],
            ['exclude', '../ui/views/ime/input_method_ibus.h'],
          ],
        }],
        ['OS=="win"', {
          'dependencies': [
            # For accessibility
            '../third_party/iaccessible2/iaccessible2.gyp:iaccessible2',
          ],
          'include_dirs': [
            '../third_party/wtl/include',
          ],
        }],
        ['use_x11==0', {
          'sources!': [
            '../ui/views/events/event_x.cc',
          ],
        }],
      ],
    },
    {
      'target_name': 'views_unittests',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:test_support_base',
        # TODO(jcivelli): ideally the resource needed by views would be
        #                 factored out. (for some reason it pulls in a bunch
        #                 unrelated things like v8, sqlite nss...).
        '../chrome/chrome_resources.gyp:packed_resources',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/base/strings/ui_strings.gyp:ui_strings',
        '../ui/gfx/compositor/compositor.gyp:compositor_test_support',
        '../ui/gfx/compositor/compositor.gyp:test_compositor',
        '../ui/ui.gyp:gfx_resources',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
        'views',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '../ui/views/animation/bounds_animator_unittest.cc',
        '../ui/views/bubble/bubble_delegate_unittest.cc',
        '../ui/views/bubble/bubble_frame_view_unittest.cc',
        '../ui/views/controls/combobox/native_combobox_views_unittest.cc',
        '../ui/views/controls/scrollbar/scrollbar_unittest.cc',
        '../ui/views/controls/tabbed_pane/tabbed_pane_unittest.cc',
        '../ui/views/controls/table/table_view_unittest.cc',
        '../ui/views/controls/textfield/native_textfield_views_unittest.cc',
        '../ui/views/controls/textfield/textfield_views_model_unittest.cc',
        '../ui/views/events/event_unittest.cc',
        '../ui/views/focus/accelerator_handler_gtk_unittest.cc',
        '../ui/views/focus/focus_manager_test.h',
        '../ui/views/focus/focus_manager_test.cc',
        '../ui/views/focus/focus_manager_unittest.cc',
        '../ui/views/focus/focus_manager_unittest_win.cc',
        '../ui/views/focus/focus_traversal_unittest.cc',
        '../ui/views/layout/box_layout_unittest.cc',
        '../ui/views/layout/grid_layout_unittest.cc',
        '../ui/views/test/test_views_delegate.cc',
        '../ui/views/test/test_views_delegate.h',
        '../ui/views/test/views_test_base.cc',
        '../ui/views/test/views_test_base.h',
        '../ui/views/widget/native_widget_test_utils.h',
        '../ui/views/widget/native_widget_test_utils_aura.cc',
        '../ui/views/widget/native_widget_test_utils_gtk.cc',
        '../ui/views/widget/native_widget_test_utils_win.cc',
        '../ui/views/widget/native_widget_unittest.cc',
        '../ui/views/widget/native_widget_win_unittest.cc',
        '../ui/views/widget/widget_unittest.cc',
        'accessible_pane_view_unittest.cc',
        'controls/label_unittest.cc',
        'controls/progress_bar_unittest.cc',
        'controls/single_split_view_unittest.cc',
        'controls/menu/menu_model_adapter_unittest.cc',
        'run_all_unittests.cc',
        'view_unittest.cc',

        '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
      ],
      'conditions': [
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
          'conditions': [
            ['linux_use_tcmalloc==1', {
               'dependencies': [
                 '../base/allocator/allocator.gyp:allocator',
               ],
            }],
          ],
        }],
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
            ]
          },
          'include_dirs': [
            '../third_party/wtl/include',
          ],
        }],
        [ 'use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:test_support_aura',
          ],
          'sources/': [
            ['exclude', '../ui/views/controls/combobox/native_combobox_views_unittest.cc'],
            ['exclude', '../ui/views/controls/tabbed_pane/tabbed_pane_unittest.cc'],
            ['exclude', '../ui/views/controls/table/table_view_unittest.cc'],
            ['exclude', '../ui/views/widget/native_widget_win_unittest.cc'],
          ],
        }, {
          'sources/': [
            ['exclude', '../ui/aura/test/test_desktop_delegate.cc'],
            ['exclude', '../ui/aura/test/test_desktop_delegate.h'],
          ],
        }],
      ],
    },
    {
      'target_name': 'views_examples',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:gfx_resources',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
        'views',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '../ui/views/examples/bubble_example.cc',
        '../ui/views/examples/bubble_example.h',
        '../ui/views/examples/button_example.cc',
        '../ui/views/examples/button_example.h',
        '../ui/views/examples/combobox_example.cc',
        '../ui/views/examples/combobox_example.h',
        '../ui/views/examples/double_split_view_example.cc',
        '../ui/views/examples/double_split_view_example.h',
        '../ui/views/examples/example_base.cc',
        '../ui/views/examples/example_base.h',
        '../ui/views/examples/example_combobox_model.cc',
        '../ui/views/examples/example_combobox_model.h',
        '../ui/views/examples/examples_main.cc',
        '../ui/views/examples/examples_main.h',
        '../ui/views/examples/link_example.cc',
        '../ui/views/examples/link_example.h',
        '../ui/views/examples/message_box_example.cc',
        '../ui/views/examples/message_box_example.h',
        '../ui/views/examples/menu_example.cc',
        '../ui/views/examples/menu_example.h',
        '../ui/views/examples/native_theme_button_example.cc',
        '../ui/views/examples/native_theme_button_example.h',
        '../ui/views/examples/native_theme_checkbox_example.cc',
        '../ui/views/examples/native_theme_checkbox_example.h',
        '../ui/views/examples/progress_bar_example.cc',
        '../ui/views/examples/progress_bar_example.h',
        '../ui/views/examples/radio_button_example.cc',
        '../ui/views/examples/radio_button_example.h',
        '../ui/views/examples/scroll_view_example.cc',
        '../ui/views/examples/scroll_view_example.h',
        '../ui/views/examples/single_split_view_example.cc',
        '../ui/views/examples/single_split_view_example.h',
        '../ui/views/examples/tabbed_pane_example.cc',
        '../ui/views/examples/tabbed_pane_example.h',
        '../ui/views/examples/table2_example.cc',
        '../ui/views/examples/table2_example.h',
        '../ui/views/examples/text_example.cc',
        '../ui/views/examples/text_example.h',
        '../ui/views/examples/textfield_example.cc',
        '../ui/views/examples/textfield_example.h',
        '../ui/views/examples/throbber_example.cc',
        '../ui/views/examples/throbber_example.h',
        '../ui/views/examples/widget_example.cc',
        '../ui/views/examples/widget_example.h',
        '../ui/views/test/test_views_delegate.cc',
        '../ui/views/test/test_views_delegate.h',
        '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
      ],
      'conditions': [
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
          'conditions': [
            ['linux_use_tcmalloc==1', {
               'dependencies': [
                 '../base/allocator/allocator.gyp:allocator',
               ],
            }],
          ],
        },
        ],
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
            ]
          },
          'include_dirs': [
            '../third_party/wtl/include',
          ],
          'msvs_settings': {
            'VCManifestTool': {
              'AdditionalManifestFiles': '..\\ui\\views\\examples\\views_examples.exe.manifest',
            },
          },
          'sources': [
            '../ui/views/examples/table_example.cc',
            '../ui/views/examples/table_example.h',
          ],
        }],
      ],
    },
  ],
}
