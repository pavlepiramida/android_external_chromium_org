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
      ['touchui==0', {
        'sources/': [
          ['exclude', '_(touch)\\.cc$'],
        ],
      }],
      ['use_aura==1', {
        'sources/': [ ['exclude', '_win\\.(h|cc)$'],
                      ['exclude', '_gtk\\.(h|cc)$'],
                      ['exclude', '_x\\.(h|cc)$'] ],
        'dependencies': [ '../ui/aura/aura.gyp:aura', ],
      }],
      ['use_wayland == 1', {
        'dependencies': [
          '../build/linux/system.gyp:wayland',
        ],
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
        'accessibility/native_view_accessibility_win.cc',
        'accessibility/native_view_accessibility_win.h',
        'accessible_pane_view.cc',
        'accessible_pane_view.h',
        'animation/bounds_animator.cc',
        'animation/bounds_animator.h',
        'background.cc',
        'background.h',
        'border.cc',
        'border.h',
        'bubble/bubble_border.cc',
        'bubble/bubble_border.h',
        'bubble/border_contents_view.cc',
        'bubble/border_contents_view.h',
        'bubble/bubble_delegate.cc',
        'bubble/bubble_delegate.h',
        'bubble/bubble_frame_view.cc',
        'bubble/bubble_frame_view.h',
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
        'controls/combobox/combobox.cc',
        'controls/combobox/combobox.h',
        'controls/combobox/native_combobox_gtk.cc',
        'controls/combobox/native_combobox_gtk.h',
        'controls/combobox/native_combobox_views.cc',
        'controls/combobox/native_combobox_views.h',
        'controls/combobox/native_combobox_wayland.cc',
        'controls/combobox/native_combobox_win.cc',
        'controls/combobox/native_combobox_win.h',
        'controls/combobox/native_combobox_wrapper.h',
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
        'controls/native/native_view_host.cc',
        'controls/native/native_view_host.h',
        'controls/native/native_view_host_aura.cc',
        'controls/native/native_view_host_aura.h',
        'controls/native/native_view_host_gtk.cc',
        'controls/native/native_view_host_gtk.h',
        'controls/native/native_view_host_win.cc',
        'controls/native/native_view_host_win.h',
        'controls/native/native_view_host_views.cc',
        'controls/native/native_view_host_views.h',
        'controls/native/native_view_host_wayland.cc',
        'controls/native/native_view_host_wrapper.h',
        'controls/progress_bar.h',
        'controls/progress_bar.cc',
        'controls/resize_area.cc',
        'controls/resize_area.h',
        'controls/resize_area_delegate.h',
        'controls/scroll_view.cc',
        'controls/scroll_view.h',
        'controls/scrollbar/base_scroll_bar.cc',
        'controls/scrollbar/base_scroll_bar.h',
        'controls/scrollbar/base_scroll_bar_button.cc',
        'controls/scrollbar/base_scroll_bar_button.h',
        'controls/scrollbar/base_scroll_bar_thumb.cc',
        'controls/scrollbar/base_scroll_bar_thumb.h',
        'controls/scrollbar/bitmap_scroll_bar.cc',
        'controls/scrollbar/bitmap_scroll_bar.h',
        'controls/scrollbar/native_scroll_bar_gtk.cc',
        'controls/scrollbar/native_scroll_bar_gtk.h',
        'controls/scrollbar/native_scroll_bar_views.cc',
        'controls/scrollbar/native_scroll_bar_views.h',
        'controls/scrollbar/native_scroll_bar_win.cc',
        'controls/scrollbar/native_scroll_bar_win.h',
        'controls/scrollbar/native_scroll_bar_wrapper.h',
        'controls/scrollbar/native_scroll_bar.cc',
        'controls/scrollbar/native_scroll_bar.h',
        'controls/scrollbar/scroll_bar.cc',
        'controls/scrollbar/scroll_bar.h',
        'controls/separator.cc',
        'controls/separator.h',
        'controls/single_split_view.cc',
        'controls/single_split_view.h',
        'controls/single_split_view_listener.h',
        'controls/tabbed_pane/native_tabbed_pane_gtk.cc',
        'controls/tabbed_pane/native_tabbed_pane_gtk.h',
        'controls/tabbed_pane/native_tabbed_pane_win.cc',
        'controls/tabbed_pane/native_tabbed_pane_win.h',
        'controls/tabbed_pane/native_tabbed_pane_wrapper.h',
        'controls/tabbed_pane/tabbed_pane.cc',
        'controls/tabbed_pane/tabbed_pane.h',
        'controls/tabbed_pane/tabbed_pane_listener.h',
        'controls/table/native_table_wrapper.h',
        'controls/table/native_table_gtk.cc',
        'controls/table/native_table_gtk.h',
        'controls/table/native_table_win.cc',
        'controls/table/native_table_win.h',
        'controls/table/group_table_view.cc',
        'controls/table/group_table_view.h',
        'controls/table/table_view.cc',
        'controls/table/table_view.h',
        'controls/table/table_view2.cc',
        'controls/table/table_view2.h',
        'controls/table/table_view_observer.h',
        'controls/textfield/gtk_views_entry.cc',
        'controls/textfield/gtk_views_entry.h',
        'controls/textfield/gtk_views_textview.cc',
        'controls/textfield/gtk_views_textview.h',
        'controls/textfield/textfield.cc',
        'controls/textfield/textfield.h',
        'controls/textfield/textfield_controller.h',
        'controls/textfield/textfield_views_model.cc',
        'controls/textfield/textfield_views_model.h',
        'controls/textfield/native_textfield_gtk.cc',
        'controls/textfield/native_textfield_gtk.h',
        'controls/textfield/native_textfield_wayland.cc',
        'controls/textfield/native_textfield_win.cc',
        'controls/textfield/native_textfield_win.h',
        'controls/textfield/native_textfield_wrapper.h',
        'controls/textfield/native_textfield_views.cc',
        'controls/textfield/native_textfield_views.h',
        'controls/throbber.cc',
        'controls/throbber.h',
        'controls/tree/tree_view.cc',
        'controls/tree/tree_view.h',
        #'debug_utils.cc',
        #'debug_utils.h',
        'drag_controller.h',
        'drag_utils.cc',
        'drag_utils.h',
        'drag_utils_gtk.cc',
        'drag_utils_linux.cc',
        'drag_utils_win.cc',
        'events/event.cc',
        'events/event.h',
        'events/event_aura.cc',
        'events/event_gtk.cc',
        'events/event_wayland.cc',
        'events/event_win.cc',
        'events/event_x.cc',
        'focus/accelerator_handler.h',
        'focus/accelerator_handler_aura.cc',
        'focus/accelerator_handler_gtk.cc',
        'focus/accelerator_handler_touch.cc',
        'focus/accelerator_handler_wayland.cc',
        'focus/accelerator_handler_win.cc',
        'focus/external_focus_tracker.cc',
        'focus/external_focus_tracker.h',
        'focus/focus_manager.cc',
        'focus/focus_manager.h',
        'focus/focus_manager_factory.cc',
        'focus/focus_manager_factory.h',
        'focus/focus_search.cc',
        'focus/focus_search.h',
        'focus/view_storage.cc',
        'focus/view_storage.h',
        'focus/widget_focus_manager.cc',
        'focus/widget_focus_manager.h',
        'ime/input_method.h',
        'ime/input_method_delegate.h',
        'ime/input_method_base.cc',
        'ime/input_method_base.h',
        'ime/input_method_gtk.cc',
        'ime/input_method_gtk.h',
        'ime/input_method_ibus.cc',
        'ime/input_method_ibus.h',
        'ime/input_method_wayland.cc',
        'ime/input_method_wayland.h',
        'ime/input_method_win.cc',
        'ime/input_method_win.h',
        'ime/mock_input_method.cc',
        'ime/mock_input_method.h',
        'ime/text_input_type_tracker.h',
        'ime/text_input_type_tracker.cc',
        'layout/box_layout.cc',
        'layout/box_layout.h',
        'layout/fill_layout.cc',
        'layout/fill_layout.h',
        'layout/grid_layout.cc',
        'layout/grid_layout.h',
        'layout/layout_constants.h',
        'layout/layout_manager.cc',
        'layout/layout_manager.h',
        'metrics.cc',
        'metrics.h',
        'metrics_aura.cc',
        'metrics_gtk.cc',
        'metrics_wayland.cc',
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
        'touchui/gesture_manager.cc',
        'touchui/gesture_manager.h',
        'touchui/touch_selection_controller.cc',
        'touchui/touch_selection_controller.h',
        'touchui/touch_selection_controller_impl.cc',
        'touchui/touch_selection_controller_impl.h',
        'view.cc',
        'view.h',
        'view_aura.cc',
        'view_constants.cc',
        'view_constants.h',
        'view_gtk.cc',
        'view_text_utils.cc',
        'view_text_utils.h',
        'view_wayland.cc',
        'view_win.cc',
        'views_delegate.h',
        'widget/aero_tooltip_manager.cc',
        'widget/aero_tooltip_manager.h',
        'widget/child_window_message_processor.cc',
        'widget/child_window_message_processor.h',
        'widget/default_theme_provider.cc',
        'widget/default_theme_provider.h',
        'widget/drop_helper.cc',
        'widget/drop_helper.h',
        'widget/drop_target_gtk.cc',
        'widget/drop_target_gtk.h',
        'widget/drop_target_win.cc',
        'widget/drop_target_win.h',
        'widget/gtk_views_fixed.cc',
        'widget/gtk_views_fixed.h',
        'widget/gtk_views_window.cc',
        'widget/gtk_views_window.h',
        'widget/root_view.cc',
        'widget/root_view.h',
        'widget/tooltip_manager_gtk.cc',
        'widget/tooltip_manager_gtk.h',
        'widget/tooltip_manager_views.cc',
        'widget/tooltip_manager_views.h',
        'widget/tooltip_manager_win.cc',
        'widget/tooltip_manager_win.h',
        'widget/tooltip_manager.cc',
        'widget/tooltip_manager.h',
        'widget/monitor_win.cc',
        'widget/monitor_win.h',
        'widget/native_widget.h',
        'widget/native_widget_aura.cc',
        'widget/native_widget_aura.h',
        'widget/native_widget_delegate.h',
        'widget/native_widget_private.h',
        'widget/native_widget_gtk.cc',
        'widget/native_widget_gtk.h',
        'widget/native_widget_wayland.cc',
        'widget/native_widget_wayland.h',
        'widget/native_widget_view.cc',
        'widget/native_widget_view.h',
        'widget/native_widget_views.cc',
        'widget/native_widget_views.h',
        'widget/native_widget_win.cc',
        'widget/native_widget_win.h',
        'widget/widget.cc',
        'widget/widget.h',
        'widget/widget_delegate.cc',
        'widget/widget_delegate.h',
        'widget/window_manager.cc',
        'widget/window_manager.h',
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
        ['use_wayland == 1', {
          'dependencies': [
            '../ui/wayland/wayland.gyp:wayland',
          ],
          'sources/': [
            ['exclude', '_(gtk|x)\\.cc$'],
            ['exclude', '/(gtk|x)_[^/]*\\.cc$'],
            ['exclude', 'focus/accelerator_handler_touch.cc'],
            ['include', 'controls/menu/native_menu_views.cc'],
            ['include', 'controls/menu/native_menu_views.h'],
            ['include', 'drag_utils_gtk.cc'],
            ['include', 'widget/tooltip_manager_views.cc'],
          ],
        }],
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
            'controls/scrollbar/bitmap_scroll_bar.cc',
            'controls/scrollbar/bitmap_scroll_bar.h',
            'controls/tabbed_pane/tabbed_pane.cc',
            'controls/tabbed_pane/tabbed_pane.h',
            'controls/table/group_table_view.cc',
            'controls/table/group_table_view.h',
            'controls/table/native_table_wrapper.h',
            'controls/table/table_view.cc',
            'controls/table/table_view.h',
            'controls/table/table_view2.cc',
            'controls/table/table_view2.h',
            'controls/table/table_view_observer.h',
            'controls/tree/tree_view.cc',
            'controls/tree/tree_view.h',
            'focus/accelerator_handler_touch.cc',
            'widget/aero_tooltip_manager.cc',
            'widget/aero_tooltip_manager.h',
            'widget/child_window_message_processor.cc',
            'widget/child_window_message_processor.h',
          ],
          'conditions': [
            ['OS=="win"', {
              'sources/': [
                ['include', 'drag_utils_win.cc'],
              ],
            }],
          ],
        }, { # else: use_aura==1
          'sources!': [
            'drag_utils_linux.cc',
          ]
        }],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
            '../build/linux/system.gyp:x11',
            '../build/linux/system.gyp:xext',
          ],
          'sources!': [
            'accessibility/native_view_accessibility_win.cc',
            'controls/scrollbar/bitmap_scroll_bar.cc',
            'controls/native_control.cc',
            'controls/table/group_table_view.cc',
            'controls/table/table_view.cc',
            'controls/tree/tree_view.cc',
            'events/event_win.cc',
            'widget/aero_tooltip_manager.cc',
            'widget/child_window_message_processor.cc',
            'widget/child_window_message_processor.h',
            'widget/native_widget_win.cc',
          ],
        }],
        ['touchui==1', {
          'defines': ['TOUCH_UI=1'],
          'sources/': [
            ['exclude', 'focus/accelerator_handler_gtk.cc'],
            ['exclude', 'controls/menu/native_menu_gtk.cc'],
            ['exclude', 'widget/tooltip_manager_gtk.cc'],
          ],
        }],
        ['touchui==0', {
          'sources!': [
            'touchui/touch_selection_controller_impl.cc',
            'touchui/touch_selection_controller_impl.h',
          ],
        }],
        ['touchui==0 and use_aura==0', {
          'sources!': [
            'controls/menu/native_menu_views.cc',
            'controls/menu/native_menu_views.h',
            'widget/tooltip_manager_views.cc',
          ],
        }],
        ['use_ibus==1', {
          'dependencies': [
            '../build/linux/system.gyp:ibus',
          ],
          'sources/': [
            ['exclude', 'ime/mock_input_method.cc'],
            ['exclude', 'ime/mock_input_method.h'],
          ],
        }, { # else: use_ibus != 1
          'sources/': [
            ['exclude', 'ime/input_method_ibus.cc'],
            ['exclude', 'ime/input_method_ibus.h'],
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
            'events/event_x.cc',
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
        'accessible_pane_view_unittest.cc',
        'animation/bounds_animator_unittest.cc',
        'bubble/bubble_delegate_unittest.cc',
        'bubble/bubble_frame_view_unittest.cc',
        'controls/label_unittest.cc',
        'controls/progress_bar_unittest.cc',
        'controls/single_split_view_unittest.cc',
        'controls/tabbed_pane/tabbed_pane_unittest.cc',
        'controls/table/table_view_unittest.cc',
        'controls/combobox/native_combobox_views_unittest.cc',
        'controls/menu/menu_model_adapter_unittest.cc',
        'controls/textfield/native_textfield_views_unittest.cc',
        'controls/textfield/textfield_views_model_unittest.cc',
        'controls/scrollbar/scrollbar_unittest.cc',
        'events/event_unittest.cc',
        'focus/accelerator_handler_gtk_unittest.cc',
        'focus/focus_manager_unittest.cc',
        'layout/grid_layout_unittest.cc',
        'layout/box_layout_unittest.cc',
        'test/views_test_base.cc',
        'test/views_test_base.h',
        'run_all_unittests.cc',
        'test/test_views_delegate.cc',
        'test/test_views_delegate.h',
        'touchui/touch_selection_controller_impl_unittest.cc',
        'view_unittest.cc',
        'widget/native_widget_test_utils.h',
        'widget/native_widget_test_utils_aura.cc',
        'widget/native_widget_test_utils_gtk.cc',
        'widget/native_widget_test_utils_win.cc',
        'widget/native_widget_unittest.cc',
        'widget/native_widget_win_unittest.cc',
        'widget/widget_unittest.cc',

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
            [ 'touchui==1', {
              'sources!': [
                'focus/accelerator_handler_gtk_unittest.cc',
                'controls/table/table_view_unittest.cc',
                'controls/tabbed_pane/tabbed_pane_unittest.cc'
              ],
            }],
          ],
        }],
        ['touchui==0', {
          'sources!': [
            'touchui/touch_selection_controller_impl_unittest.cc',
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
            ['exclude', 'focus/focus_manager_unittest.cc'], # TODO(beng):
            ['exclude', 'widget/native_widget_win_unittest.cc'],
            ['exclude', 'controls/combobox/native_combobox_views_unittest.cc'],
            ['exclude', 'controls/table/table_view_unittest.cc'],
            ['exclude', 'controls/tabbed_pane/tabbed_pane_unittest.cc'],
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
        'examples/bubble_example.cc',
        'examples/bubble_example.h',
        'examples/button_example.cc',
        'examples/button_example.h',
        'examples/combobox_example.cc',
        'examples/combobox_example.h',
        'examples/double_split_view_example.cc',
        'examples/double_split_view_example.h',
        'examples/example_base.cc',
        'examples/example_base.h',
        'examples/example_combobox_model.cc',
        'examples/example_combobox_model.h',
        'examples/examples_main.cc',
        'examples/examples_main.h',
        'examples/link_example.cc',
        'examples/link_example.h',
        'examples/message_box_example.cc',
        'examples/message_box_example.h',
        'examples/menu_example.cc',
        'examples/menu_example.h',
        'examples/native_theme_button_example.cc',
        'examples/native_theme_button_example.h',
        'examples/native_theme_checkbox_example.cc',
        'examples/native_theme_checkbox_example.h',
        'examples/native_widget_views_example.cc',
        'examples/native_widget_views_example.h',
        'examples/progress_bar_example.cc',
        'examples/progress_bar_example.h',
        'examples/radio_button_example.cc',
        'examples/radio_button_example.h',
        'examples/scroll_view_example.cc',
        'examples/scroll_view_example.h',
        'examples/single_split_view_example.cc',
        'examples/single_split_view_example.h',
        'examples/tabbed_pane_example.cc',
        'examples/tabbed_pane_example.h',
        'examples/table2_example.cc',
        'examples/table2_example.h',
        'examples/text_example.cc',
        'examples/text_example.h',
        'examples/textfield_example.cc',
        'examples/textfield_example.h',
        'examples/throbber_example.cc',
        'examples/throbber_example.h',
        'examples/widget_example.cc',
        'examples/widget_example.h',
        'test/test_views_delegate.cc',
        'test/test_views_delegate.h',
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
              'AdditionalManifestFiles': '$(ProjectDir)\\examples\\views_examples.exe.manifest',
            },
          },
          'sources': [
            'examples/table_example.cc',
            'examples/table_example.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'views_desktop_lib',
      'type': 'static_library',
      'dependencies': [
        '../base/base.gyp:base',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/gfx/compositor/compositor.gyp:compositor',
        '../ui/ui.gyp:gfx_resources',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        'views',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'desktop/desktop_background.cc',
        'desktop/desktop_background.h',
        'desktop/desktop_window_manager.cc',
        'desktop/desktop_window_manager.h',
        'desktop/desktop_window_view.cc',
        'desktop/desktop_window_view.h',
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
        }],
      ],
    },
    {
      'target_name': 'views_desktop',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/ui.gyp:gfx_resources',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
        '../ui/gfx/compositor/compositor.gyp:compositor',
        'views',
        'views_desktop_lib',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'desktop/desktop_main.cc',
        'desktop/desktop_views_delegate.cc',
        'desktop/desktop_views_delegate.h',
        '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
      ],
      'conditions': [
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
        },
        ],
        ['use_glib == 1', {
          'dependencies': [
            '../build/linux/system.gyp:glib',
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
      ],
    },
  ],
  'conditions': [
    ['use_aura==1', {
      'targets': [
        {
          'target_name': 'views_aura_desktop',
          'type': 'executable',
          'dependencies': [
            '../base/base.gyp:base',
            '../base/base.gyp:base_i18n',
            '../chrome/chrome_resources.gyp:packed_resources',
            '../skia/skia.gyp:skia',
            '../third_party/icu/icu.gyp:icui18n',
            '../third_party/icu/icu.gyp:icuuc',
            '../ui/aura/aura.gyp:aura',
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
            'aura_desktop/aura_desktop_main.cc',
            '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
          ],
          'conditions': [
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
          ],
        },
      ],
    }],
  ],
}
