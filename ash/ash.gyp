# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/chrome',
  },

  'targets': [
    {
      'target_name': 'ash',
      'type': '<(component)',
      'dependencies': [
        'ash_strings.gyp:ash_strings',
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../content/content.gyp:content',
        '../content/content.gyp:content_browser',
        '../ipc/ipc.gyp:ipc',
        '../net/net.gyp:net',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura',
        '../ui/base/strings/ui_strings.gyp:ui_strings',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/oak/oak.gyp:oak',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
        '../ui/views/views.gyp:views',
      ],
      'defines': [
        'ASH_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under ash, except unittests
        'accelerators/accelerator_controller.cc',
        'accelerators/accelerator_controller.h',
        'accelerators/accelerator_dispatcher.cc',
        'accelerators/accelerator_dispatcher.h',
        'accelerators/accelerator_filter.cc',
        'accelerators/accelerator_filter.h',
        'accelerators/accelerator_table.cc',
        'accelerators/accelerator_table.h',
        'accelerators/focus_manager_factory.cc',
        'accelerators/focus_manager_factory.h',
        'accelerators/nested_dispatcher_controller.cc',
        'accelerators/nested_dispatcher_controller.h',
        'ash_switches.cc',
        'ash_switches.h',
        'caps_lock_delegate.h',
        'desktop_background/desktop_background_controller.cc',
        'desktop_background/desktop_background_controller.h',
        'desktop_background/desktop_background_resources.cc',
        'desktop_background/desktop_background_resources.h',
        'desktop_background/desktop_background_view.cc',
        'desktop_background/desktop_background_view.h',
        'drag_drop/drag_drop_controller.cc',
        'drag_drop/drag_drop_controller.h',
        'drag_drop/drag_image_view.cc',
        'drag_drop/drag_image_view.h',
        'focus_cycler.cc',
        'focus_cycler.h',
        'high_contrast/high_contrast_controller.cc',
        'high_contrast/high_contrast_controller.h',
        'key_rewriter_delegate.h',
        'launcher/background_animator.cc',
        'launcher/background_animator.h',
        'launcher/launcher.cc',
        'launcher/launcher.h',
        'launcher/launcher_alignment_menu.cc',
        'launcher/launcher_alignment_menu.h',
        'launcher/launcher_button.cc',
        'launcher/launcher_button.h',
        'launcher/launcher_context_menu.cc',
        'launcher/launcher_context_menu.h',
        'launcher/launcher_delegate.h',
        'launcher/launcher_icon_observer.h',
        'launcher/launcher_model.cc',
        'launcher/launcher_model.h',
        'launcher/launcher_model_observer.h',
        'launcher/launcher_navigator.cc',
        'launcher/launcher_navigator.h',
        'launcher/launcher_tooltip_manager.cc',
        'launcher/launcher_tooltip_manager.h',
        'launcher/launcher_types.cc',
        'launcher/launcher_types.h',
        'launcher/launcher_view.cc',
        'launcher/launcher_view.h',
        'launcher/tabbed_launcher_button.cc',
        'launcher/tabbed_launcher_button.h',
        'magnifier/magnification_controller.cc',
        'magnifier/magnification_controller.h',
        'monitor/monitor_controller.cc',
        'monitor/monitor_controller.h',
        'monitor/multi_monitor_manager.cc',
        'monitor/multi_monitor_manager.h',
        'monitor/secondary_monitor_view.cc',
        'monitor/secondary_monitor_view.h',
        'root_window_controller.cc',
        'root_window_controller.h',
        'screen_ash.cc',
        'screen_ash.h',
        'screensaver/screensaver_view.cc',
        'screensaver/screensaver_view.h',
        'screenshot_delegate.h',
        'shell.cc',
        'shell.h',
        'shell_context_menu.cc',
        'shell_context_menu.h',
        'shell_delegate.h',
        'shell_factory.h',
        'shell_window_ids.h',
        'system/audio/audio_observer.h',
        'system/audio/tray_volume.cc',
        'system/audio/tray_volume.h',
        'system/bluetooth/bluetooth_observer.h',
        'system/bluetooth/tray_bluetooth.cc',
        'system/bluetooth/tray_bluetooth.h',
        'system/brightness/brightness_observer.h',
        'system/brightness/brightness_control_delegate.h',
        'system/brightness/tray_brightness.cc',
        'system/brightness/tray_brightness.h',
        'system/date/clock_observer.h',
        'system/date/date_view.cc',
        'system/date/date_view.h',
        'system/date/tray_date.cc',
        'system/date/tray_date.h',
        'system/drive/drive_observer.h',
        'system/drive/tray_drive.cc',
        'system/drive/tray_drive.h',
        'system/ime/ime_observer.h',
        'system/ime/tray_ime.cc',
        'system/ime/tray_ime.h',
        'system/locale/tray_locale.cc',
        'system/locale/tray_locale.h',
        'system/network/network_observer.h',
        'system/network/tray_network.cc',
        'system/network/tray_network.h',
        'system/network/tray_sms.cc',
        'system/network/tray_sms.h',
        'system/power/power_status_observer.h',
        'system/power/power_status_view.cc',
        'system/power/power_status_view.h',
        'system/power/power_supply_status.cc',
        'system/power/power_supply_status.h',
        'system/power/tray_power.cc',
        'system/power/tray_power.h',
        'system/settings/tray_settings.cc',
        'system/settings/tray_settings.h',
        'system/status_area_widget.cc',
        'system/status_area_widget.h',
        'system/status_area_widget_delegate.cc',
        'system/status_area_widget_delegate.h',
        'system/tray/system_tray.cc',
        'system/tray/system_tray.h',
        'system/tray/system_tray_bubble.cc',
        'system/tray/system_tray_bubble.h',
        'system/tray/system_tray_delegate.cc',
        'system/tray/system_tray_delegate.h',
        'system/tray/system_tray_item.cc',
        'system/tray/system_tray_item.h',
        'system/tray/tray_background_view.cc',
        'system/tray/tray_background_view.h',
        'system/tray/tray_bubble_view.cc',
        'system/tray/tray_bubble_view.h',
        'system/tray/tray_constants.cc',
        'system/tray/tray_constants.h',
        'system/tray/tray_details_view.cc',
        'system/tray/tray_details_view.h',
        'system/tray/tray_empty.cc',
        'system/tray/tray_empty.h',
        'system/tray/tray_image_item.cc',
        'system/tray/tray_image_item.h',
        'system/tray/tray_item_more.cc',
        'system/tray/tray_item_more.h',
        'system/tray/tray_item_view.cc',
        'system/tray/tray_item_view.h',
        'system/tray/tray_views.cc',
        'system/tray/tray_views.h',
        'system/tray_accessibility.cc',
        'system/tray_accessibility.h',
        'system/tray_caps_lock.cc',
        'system/tray_caps_lock.h',
        'system/tray_update.cc',
        'system/tray_update.h',
        'system/user/login_status.h',
        'system/user/tray_user.cc',
        'system/user/tray_user.h',
        'system/user/update_observer.h',
        'system/user/user_observer.h',
        'system/web_notification/web_notification_tray.cc',
        'system/web_notification/web_notification_tray.h',
        'tooltips/tooltip_controller.cc',
        'tooltips/tooltip_controller.h',
        'touch/touch_observer_hud.cc',
        'touch/touch_observer_hud.h',
        'ui_controls_ash.cc',
        'volume_control_delegate.h',
        'wm/app_list_controller.cc',
        'wm/app_list_controller.h',
        'wm/activation_controller.cc',
        'wm/activation_controller.h',
        'wm/always_on_top_controller.cc',
        'wm/always_on_top_controller.h',
        'wm/base_layout_manager.cc',
        'wm/base_layout_manager.h',
        'wm/capture_controller.cc',
        'wm/capture_controller.h',
        'wm/custom_frame_view_ash.cc',
        'wm/custom_frame_view_ash.h',
        'wm/default_window_resizer.cc',
        'wm/default_window_resizer.h',
        'wm/dialog_frame_view.cc',
        'wm/dialog_frame_view.h',
        'wm/event_client_impl.cc',
        'wm/event_client_impl.h',
        'wm/frame_painter.cc',
        'wm/frame_painter.h',
        'wm/image_grid.cc',
        'wm/image_grid.h',
        'wm/key_rewriter_event_filter.cc',
        'wm/key_rewriter_event_filter.h',
        'wm/panel_frame_view.cc',
        'wm/panel_frame_view.h',
        'wm/panel_layout_manager.cc',
        'wm/panel_layout_manager.h',
        'wm/panel_window_event_filter.cc',
        'wm/panel_window_event_filter.h',
        'wm/partial_screenshot_event_filter.cc',
        'wm/partial_screenshot_event_filter.h',
        'wm/partial_screenshot_view.cc',
        'wm/partial_screenshot_view.h',
        'wm/power_button_controller.cc',
        'wm/power_button_controller.h',
        'wm/property_util.cc',
        'wm/property_util.h',
        'wm/resize_shadow.cc',
        'wm/resize_shadow.h',
        'wm/resize_shadow_controller.cc',
        'wm/resize_shadow_controller.h',
        'wm/root_window_layout_manager.cc',
        'wm/root_window_layout_manager.h',
        'wm/scoped_observer.h',
        'wm/screen_dimmer.cc',
        'wm/screen_dimmer.h',
        'wm/shadow.cc',
        'wm/shadow.h',
        'wm/shadow_controller.cc',
        'wm/shadow_controller.h',
        'wm/shadow_types.cc',
        'wm/shadow_types.h',
        'wm/shelf_auto_hide_behavior.h',
        'wm/shelf_layout_manager.cc',
        'wm/shelf_layout_manager.h',
        'wm/slow_animation_event_filter.cc',
        'wm/slow_animation_event_filter.h',
        'wm/stacking_controller.cc',
        'wm/stacking_controller.h',
        'wm/status_area_layout_manager.cc',
        'wm/status_area_layout_manager.h',
        'wm/system_gesture_event_filter.cc',
        'wm/system_gesture_event_filter.h',
        'wm/system_modal_container_event_filter.cc',
        'wm/system_modal_container_event_filter.h',
        'wm/system_modal_container_event_filter_delegate.h',
        'wm/system_modal_container_layout_manager.cc',
        'wm/system_modal_container_layout_manager.h',
        'wm/toplevel_window_event_filter.cc',
        'wm/toplevel_window_event_filter.h',
        'wm/user_activity_detector.cc',
        'wm/user_activity_detector.h',
        'wm/user_activity_observer.h',
        'wm/video_detector.cc',
        'wm/video_detector.h',
        'wm/visibility_controller.cc',
        'wm/visibility_controller.h',
        'wm/window_animations.cc',
        'wm/window_animations.h',
        'wm/window_cycle_controller.cc',
        'wm/window_cycle_controller.h',
        'wm/window_cycle_list.cc',
        'wm/window_cycle_list.h',
        'wm/window_frame.cc',
        'wm/window_frame.h',
        'wm/window_modality_controller.cc',
        'wm/window_modality_controller.h',
        'wm/window_properties.cc',
        'wm/window_properties.h',
        'wm/window_resizer.cc',
        'wm/window_resizer.h',
        'wm/window_util.cc',
        'wm/window_util.h',
        'wm/workspace_controller.cc',
        'wm/workspace_controller.h',
        'wm/workspace/frame_maximize_button.cc',
        'wm/workspace/frame_maximize_button.h',
        'wm/workspace/managed_workspace.cc',
        'wm/workspace/managed_workspace.h',
        'wm/workspace/maximized_workspace.cc',
        'wm/workspace/maximized_workspace.h',
        'wm/workspace/multi_window_resize_controller.cc',
        'wm/workspace/multi_window_resize_controller.h',
        'wm/workspace/phantom_window_controller.cc',
        'wm/workspace/phantom_window_controller.h',
        'wm/workspace/snap_sizer.cc',
        'wm/workspace/snap_sizer.h',
        'wm/workspace/workspace.cc',
        'wm/workspace/workspace.h',
        'wm/workspace/workspace_event_filter.cc',
        'wm/workspace/workspace_event_filter.h',
        'wm/workspace/workspace_layout_manager.cc',
        'wm/workspace/workspace_layout_manager.h',
        'wm/workspace/workspace_manager.cc',
        'wm/workspace/workspace_manager.h',
        'wm/workspace/workspace_window_resizer.cc',
        'wm/workspace/workspace_window_resizer.h',
      ],
      'conditions': [
        ['OS=="mac"', {
          'sources/': [
            ['exclude', 'accelerators/accelerator_controller.cc'],
            ['exclude', 'accelerators/accelerator_controller.h'],
            ['exclude', 'accelerators/accelerator_dispatcher.cc'],
            ['exclude', 'accelerators/accelerator_dispatcher.h'],
            ['exclude', 'accelerators/accelerator_filter.cc'],
            ['exclude', 'accelerators/accelerator_filter.h'],
            ['exclude', 'accelerators/nested_dispatcher_controller.cc'],
            ['exclude', 'accelerators/nested_dispatcher_controller.h'],
          ],
        }],
        ['chromeos==1', {
          'dependencies': [
            '../chromeos/chromeos.gyp:chromeos',
          ],
        }],
      ],
    },
    {
      'target_name': 'aura_shell_unittests',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:test_support_base',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../content/content.gyp:content_browser',
        '../content/content.gyp:test_support_content',
        '../build/temp_gyp/googleurl.gyp:googleurl',
        '../skia/skia.gyp:skia',
        '../testing/gtest.gyp:gtest',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/aura/aura.gyp:aura',
        '../ui/aura/aura.gyp:test_support_aura',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/compositor/compositor.gyp:compositor_test_support',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
        '../ui/ui.gyp:ui_test_support',
        '../ui/views/views.gyp:views',
        '../ui/views/views.gyp:test_support_views',
        'ash',
      ],
      'sources': [
        '../ui/views/test/test_views_delegate.cc',
        '../ui/views/test/test_views_delegate.h',
        'accelerators/accelerator_controller_unittest.cc',
        'accelerators/accelerator_filter_unittest.cc',
        'accelerators/accelerator_table_unittest.cc',
        'accelerators/nested_dispatcher_controller_unittest.cc',
        'dip_unittest.cc',
        'drag_drop/drag_drop_controller_unittest.cc',
        'extended_desktop_unittest.cc',
        'focus_cycler_unittest.cc',
        'launcher/launcher_context_menu_unittest.cc',
        'launcher/launcher_model_unittest.cc',
        'launcher/launcher_navigator_unittest.cc',
        'launcher/launcher_unittest.cc',
        'launcher/launcher_view_unittest.cc',
        'monitor/multi_monitor_manager_unittest.cc',
        'screensaver/screensaver_view_unittest.cc',
        'shell_unittest.cc',
        'system/tray/system_tray_unittest.cc',
        'system/web_notification/web_notification_tray_unittest.cc',
        'test/ash_test_base.cc',
        'test/ash_test_base.h',
        'test/ash_unittests.cc',
        'test/capture_tracking_view.cc',
        'test/capture_tracking_view.h',
        'test/launcher_view_test_api.cc',
        'test/launcher_view_test_api.h',
        'test/test_activation_delegate.cc',
        'test/test_activation_delegate.h',
        'test/test_launcher_delegate.cc',
        'test/test_launcher_delegate.h',
        'test/test_shell_delegate.cc',
        'test/test_shell_delegate.h',
        'test/test_suite.cc',
        'test/test_suite.h',
        'test/test_suite_init.h',
        'test/test_suite_init.mm',
        'tooltips/tooltip_controller_unittest.cc',
        'wm/activation_controller_unittest.cc',
        'wm/base_layout_manager_unittest.cc',
        'wm/custom_frame_view_ash_unittest.cc',
        'wm/frame_painter_unittest.cc',
        'wm/image_grid_unittest.cc',
        'wm/panel_layout_manager_unittest.cc',
        'wm/power_button_controller_unittest.cc',
        'wm/screen_dimmer_unittest.cc',
        'wm/shadow_controller_unittest.cc',
        'wm/shelf_layout_manager_unittest.cc',
        'wm/slow_animation_event_filter_unittest.cc',
        'wm/system_gesture_event_filter_unittest.cc',
        'wm/system_modal_container_layout_manager_unittest.cc',
        'wm/toplevel_window_event_filter_unittest.cc',
        'wm/video_detector_unittest.cc',
        'wm/visibility_controller_unittest.cc',
        'wm/window_animations_unittest.cc',
        'wm/window_cycle_controller_unittest.cc',
        'wm/window_manager_unittest.cc',
        'wm/window_modality_controller_unittest.cc',
        'wm/workspace_controller_test_helper.cc',
        'wm/workspace_controller_test_helper.h',
        'wm/workspace/multi_window_resize_controller_unittest.cc',
        'wm/workspace/workspace_event_filter_test_helper.cc',
        'wm/workspace/workspace_event_filter_test_helper.h',
        'wm/workspace/workspace_event_filter_unittest.cc',
        'wm/workspace/workspace_manager_unittest.cc',
        'wm/workspace/workspace_window_resizer_unittest.cc',

        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
      ],
      'conditions': [
        ['use_ibus==1', {
          'dependencies': [
            '../build/linux/system.gyp:ibus',
          ],
        }],
        ['OS=="win"', {
          # TODO(zork): fix this test to build on Windows. See: crosbug.com/26906
          'sources/': [
            ['exclude', 'focus_cycler_unittest.cc'],
          ],
        }],
        ['OS=="mac"', {
          'sources/': [
            ['exclude', 'accelerators/accelerator_controller_unittest.cc'],
            ['exclude', 'accelerators/accelerator_filter_unittest.cc'],
	    ['exclude', 'accelerators/nested_dispatcher_controller_unittest.cc'],
            ['exclude', 'drag_drop/drag_drop_controller_unittest.cc'],
            ['exclude', 'tooltips/tooltip_controller_unittest.cc'],
          ],
          'dependencies': [
            # Mac tests access resources via the 'AuraShell.app' directory.
            'ash_shell',
          ],
          # Special linker instructions that avoids stripping Obj-C classes that
          # are not referenced in code, but are referenced in nibs.
          'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-ObjC']},
        }],
      ],
    },
    {
      'target_name': 'ash_shell',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../chrome/chrome_resources.gyp:packed_resources',
        '../content/content.gyp:content_shell_lib',
        '../content/content.gyp:content',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/app_list/app_list.gyp:app_list',
        '../ui/aura/aura.gyp:aura',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/compositor/compositor.gyp:compositor_test_support',
        '../ui/ui.gyp:ui',
        '../ui/ui.gyp:ui_resources',
        '../ui/ui.gyp:ui_resources_standard',
        '../ui/views/views.gyp:views',
        '../ui/views/views.gyp:views_examples_lib',
        '../ui/views/views.gyp:test_support_views',
        'ash',
      ],
      'sources': [
        'shell/app_list.cc',
        'shell/bubble.cc',
        'shell/content_client/shell_browser_main_parts.cc',
        'shell/content_client/shell_browser_main_parts.h',
        'shell/content_client/shell_content_browser_client.cc',
        'shell/content_client/shell_content_browser_client.h',
        'shell/content_client/shell_main_delegate.cc',
        'shell/content_client/shell_main_delegate.h',
        'shell/example_factory.h',
        'shell/launcher_delegate_impl.cc',
        'shell/launcher_delegate_impl.h',
        'shell/lock_view.cc',
        'shell/panel_window.cc',
        'shell/panel_window.h',
        'shell/shell_delegate_impl.cc',
        'shell/shell_delegate_impl.h',
        'shell/shell_main.cc',
        'shell/shell_main_parts.cc',
        'shell/shell_main_parts.h',
        'shell/shell_main_parts_mac.mm',
        'shell/toplevel_window.cc',
        'shell/toplevel_window.h',
        'shell/widgets.cc',
        'shell/window_type_launcher.cc',
        'shell/window_type_launcher.h',
        'shell/window_watcher.cc',
        'shell/window_watcher.h',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.rc',
        '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.rc',
        '../content/app/startup_helper_win.cc',
        '../ui/views/test/test_views_delegate.cc',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCLinkerTool': {
              'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
            },
          },
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],          
        }],
        ['OS=="mac"', {
          'product_name': 'AuraShell',
          'mac_bundle': 1,
          'sources/': [
            ['exclude', 'shell/shell_main_parts.cc'],
          ],
          'mac_bundle_resources': [
            'shell/cocoa/app.icns',
            'shell/cocoa/app-Info.plist',
            'shell/cocoa/nibs/MainMenu.xib',
            'shell/cocoa/nibs/RootWindow.xib',
            '<(SHARED_INTERMEDIATE_DIR)/repack/chrome.pak',
            '<!@pymod_do_main(repack_locales -o -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(locales))',
          ],
          'mac_bundle_resources!': [
            'shell/cocoa/app-Info.plist',
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'shell/cocoa/app-Info.plist',
          },
        }],
      ],
    },
  ],
}
