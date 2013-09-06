# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'dependencies': [
    '../jingle/jingle.gyp:jingle_glue',
    '../media/media.gyp:media',
    '../net/net.gyp:net',
    '../skia/skia.gyp:skia',
    '../third_party/WebKit/public/blink.gyp:blink',
    '../third_party/icu/icu.gyp:icui18n',
    '../third_party/icu/icu.gyp:icuuc',
    '../third_party/libjingle/libjingle.gyp:libjingle',
    '../third_party/npapi/npapi.gyp:npapi',
    '../third_party/widevine/cdm/widevine_cdm.gyp:widevine_cdm_version_h',
    '../ui/native_theme/native_theme.gyp:native_theme',
    '../ui/surface/surface.gyp:surface',
    '../v8/tools/gyp/v8.gyp:v8',
    '../webkit/common/gpu/webkit_gpu.gyp:webkit_gpu',
    '../webkit/common/webkit_common.gyp:webkit_common',
    '../webkit/renderer/compositor_bindings/compositor_bindings.gyp:webkit_compositor_bindings',
    '../webkit/renderer/compositor_bindings/compositor_bindings.gyp:webkit_compositor_support',
    '../webkit/renderer/webkit_renderer.gyp:webkit_renderer',
    '../webkit/storage_common.gyp:webkit_storage_common',
    '../webkit/support/webkit_support.gyp:glue',
    '../webkit/support/webkit_support.gyp:glue_child',
  ],
  'include_dirs': [
    '..',
    '<(SHARED_INTERMEDIATE_DIR)',  # Needed by key_systems_info.cc.
  ],
  'sources': [
    'public/renderer/android_content_detection_prefixes.cc',
    'public/renderer/android_content_detection_prefixes.h',
    'public/renderer/content_renderer_client.cc',
    'public/renderer/content_renderer_client.h',
    'public/renderer/context_menu_client.h',
    'public/renderer/document_state.cc',
    'public/renderer/document_state.h',
    'public/renderer/history_item_serialization.cc',
    'public/renderer/history_item_serialization.h',
    'public/renderer/navigation_state.cc',
    'public/renderer/navigation_state.h',
    'public/renderer/pepper_plugin_instance.h',
    'public/renderer/renderer_ppapi_host.h',
    'public/renderer/render_frame.h',
    'public/renderer/render_process_observer.cc',
    'public/renderer/render_process_observer.h',
    'public/renderer/render_thread.cc',
    'public/renderer/render_thread.h',
    'public/renderer/render_view.h',
    'public/renderer/render_view_observer.cc',
    'public/renderer/render_view_observer.h',
    'public/renderer/render_view_observer_tracker.h',
    'public/renderer/render_view_visitor.h',
    'public/renderer/v8_value_converter.h',
    'public/renderer/web_preferences.h',
    'renderer/accessibility/accessibility_node_serializer.cc',
    'renderer/accessibility/accessibility_node_serializer.h',
    'renderer/accessibility/renderer_accessibility.cc',
    'renderer/accessibility/renderer_accessibility.h',
    'renderer/accessibility/renderer_accessibility_complete.cc',
    'renderer/accessibility/renderer_accessibility_complete.h',
    'renderer/accessibility/renderer_accessibility_focus_only.cc',
    'renderer/accessibility/renderer_accessibility_focus_only.h',
    'renderer/active_notification_tracker.cc',
    'renderer/active_notification_tracker.h',
    'renderer/android/address_detector.cc',
    'renderer/android/address_detector.h',
    'renderer/android/content_detector.cc',
    'renderer/android/content_detector.h',
    'renderer/android/email_detector.cc',
    'renderer/android/email_detector.h',
    'renderer/android/phone_number_detector.cc',
    'renderer/android/phone_number_detector.h',
    'renderer/android/synchronous_compositor_factory.cc',
    'renderer/android/synchronous_compositor_factory.h',
    'renderer/cursor_utils.cc',
    'renderer/cursor_utils.h',
    'renderer/device_orientation_dispatcher.cc',
    'renderer/device_orientation_dispatcher.h',
    'renderer/device_orientation/device_motion_event_pump.cc',
    'renderer/device_orientation/device_motion_event_pump.h',
    'renderer/device_orientation/device_orientation_event_pump.cc',
    'renderer/device_orientation/device_orientation_event_pump.h',
    'renderer/device_orientation/device_sensor_event_pump.cc',
    'renderer/device_orientation/device_sensor_event_pump.h',
    'renderer/devtools/devtools_agent.cc',
    'renderer/devtools/devtools_agent.h',
    'renderer/devtools/devtools_agent_filter.cc',
    'renderer/devtools/devtools_agent_filter.h',
    'renderer/devtools/devtools_client.cc',
    'renderer/devtools/devtools_client.h',
    'renderer/disambiguation_popup_helper.cc',
    'renderer/disambiguation_popup_helper.h',
    'renderer/dom_automation_controller.cc',
    'renderer/dom_automation_controller.h',
    'renderer/dom_storage/dom_storage_cached_area.cc',
    'renderer/dom_storage/dom_storage_cached_area.h',
    'renderer/dom_storage/dom_storage_dispatcher.cc',
    'renderer/dom_storage/dom_storage_dispatcher.h',
    'renderer/dom_storage/dom_storage_proxy.h',
    'renderer/dom_storage/webstoragearea_impl.cc',
    'renderer/dom_storage/webstoragearea_impl.h',
    'renderer/dom_storage/webstoragenamespace_impl.cc',
    'renderer/dom_storage/webstoragenamespace_impl.h',
    'renderer/drop_data_builder.cc',
    'renderer/drop_data_builder.h',
    'renderer/external_popup_menu.cc',
    'renderer/external_popup_menu.h',
    'renderer/gamepad_shared_memory_reader.cc',
    'renderer/gamepad_shared_memory_reader.h',
    'renderer/geolocation_dispatcher.cc',
    'renderer/geolocation_dispatcher.h',
    'renderer/gpu/compositor_output_surface.cc',
    'renderer/gpu/compositor_output_surface.h',
    'renderer/gpu/compositor_software_output_device.cc',
    'renderer/gpu/compositor_software_output_device.h',
    'renderer/gpu/delegated_compositor_output_surface.cc',
    'renderer/gpu/delegated_compositor_output_surface.h',
    'renderer/gpu/input_event_filter.cc',
    'renderer/gpu/input_event_filter.h',
    'renderer/gpu/input_handler_proxy.cc',
    'renderer/gpu/input_handler_proxy.h',
    'renderer/gpu/input_handler_manager.cc',
    'renderer/gpu/input_handler_manager.h',
    'renderer/gpu/input_handler_manager_client.h',
    'renderer/gpu/input_handler_wrapper.cc',
    'renderer/gpu/input_handler_wrapper.h',
    'renderer/gpu/gpu_benchmarking_extension.cc',
    'renderer/gpu/gpu_benchmarking_extension.h',
    'renderer/gpu/mailbox_output_surface.cc',
    'renderer/gpu/mailbox_output_surface.h',
    'renderer/gpu/stream_texture_host_android.cc',
    'renderer/gpu/stream_texture_host_android.h',
    'renderer/gpu/render_widget_compositor.cc',
    'renderer/gpu/render_widget_compositor.h',
    'renderer/idle_user_detector.cc',
    'renderer/idle_user_detector.h',
    'renderer/image_loading_helper.cc',
    'renderer/image_loading_helper.h',
    'renderer/in_process_renderer_thread.cc',
    'renderer/in_process_renderer_thread.h',
    'renderer/input_tag_speech_dispatcher.cc',
    'renderer/input_tag_speech_dispatcher.h',
    'renderer/internal_document_state_data.cc',
    'renderer/internal_document_state_data.h',
    'renderer/java/java_bridge_channel.cc',
    'renderer/java/java_bridge_channel.h',
    'renderer/java/java_bridge_dispatcher.cc',
    'renderer/java/java_bridge_dispatcher.h',
    'renderer/load_progress_tracker.cc',
    'renderer/load_progress_tracker.h',
    'renderer/media/active_loader.cc',
    'renderer/media/active_loader.h',
    'renderer/media/android/audio_decoder_android.cc',
    'renderer/media/android/audio_decoder_android.h',
    'renderer/media/android/media_info_loader.cc',
    'renderer/media/android/media_info_loader.h',
    'renderer/media/android/media_source_delegate.cc',
    'renderer/media/android/media_source_delegate.h',
    'renderer/media/android/proxy_media_keys.cc',
    'renderer/media/android/proxy_media_keys.h',
    'renderer/media/android/renderer_media_player_manager.cc',
    'renderer/media/android/renderer_media_player_manager.h',
    'renderer/media/android/stream_texture_factory_android.h',
    'renderer/media/android/stream_texture_factory_android_impl.cc',
    'renderer/media/android/stream_texture_factory_android_impl.h',
    'renderer/media/android/stream_texture_factory_android_synchronous_impl.cc',
    'renderer/media/android/stream_texture_factory_android_synchronous_impl.h',
    'renderer/media/android/webmediaplayer_android.cc',
    'renderer/media/android/webmediaplayer_android.h',
    'renderer/media/android/webmediaplayer_proxy_android.cc',
    'renderer/media/android/webmediaplayer_proxy_android.h',
    'renderer/media/audio_decoder.cc',
    'renderer/media/audio_decoder.h',
    'renderer/media/audio_device_factory.cc',
    'renderer/media/audio_device_factory.h',
    'renderer/media/audio_input_message_filter.cc',
    'renderer/media/audio_input_message_filter.h',
    'renderer/media/audio_message_filter.cc',
    'renderer/media/audio_message_filter.h',
    'renderer/media/audio_renderer_mixer_manager.cc',
    'renderer/media/audio_renderer_mixer_manager.h',
    'renderer/media/buffered_data_source.cc',
    'renderer/media/buffered_data_source.h',
    'renderer/media/buffered_resource_loader.cc',
    'renderer/media/buffered_resource_loader.h',
    'renderer/media/cache_util.cc',
    'renderer/media/cache_util.h',
    'renderer/media/crypto/content_decryption_module_factory.cc',
    'renderer/media/crypto/content_decryption_module_factory.h',
    'renderer/media/crypto/key_systems.cc',
    'renderer/media/crypto/key_systems.h',
    'renderer/media/crypto/key_systems_info.cc',
    'renderer/media/crypto/key_systems_info.h',
    'renderer/media/crypto/ppapi_decryptor.cc',
    'renderer/media/crypto/ppapi_decryptor.h',
    'renderer/media/crypto/proxy_decryptor.cc',
    'renderer/media/crypto/proxy_decryptor.h',
    'renderer/media/media_stream_audio_renderer.cc',
    'renderer/media/media_stream_audio_renderer.h',
    'renderer/media/media_stream_center.h',
    'renderer/media/media_stream_client.h',
    'renderer/media/media_stream_dependency_factory.h',
    'renderer/media/media_stream_dispatcher.h',
    'renderer/media/media_stream_dispatcher_eventhandler.h',
    'renderer/media/media_stream_impl.h',
    'renderer/media/midi_dispatcher.cc',
    'renderer/media/midi_dispatcher.h',
    'renderer/media/midi_message_filter.cc',
    'renderer/media/midi_message_filter.h',
    'renderer/media/pepper_platform_video_decoder.cc',
    'renderer/media/pepper_platform_video_decoder.h',
    'renderer/media/preload.h',
    'renderer/media/render_media_log.cc',
    'renderer/media/render_media_log.h',
    'renderer/media/renderer_gpu_video_accelerator_factories.cc',
    'renderer/media/renderer_gpu_video_accelerator_factories.h',
    'renderer/media/renderer_webaudiodevice_impl.cc',
    'renderer/media/renderer_webaudiodevice_impl.h',
    'renderer/media/renderer_webmidiaccessor_impl.cc',
    'renderer/media/renderer_webmidiaccessor_impl.h',
    'renderer/media/texttrack_impl.cc',
    'renderer/media/texttrack_impl.h',
    'renderer/media/video_capture_impl.cc',
    'renderer/media/video_capture_impl.h',
    'renderer/media/video_capture_impl_manager.cc',
    'renderer/media/video_capture_impl_manager.h',
    'renderer/media/video_capture_message_filter.cc',
    'renderer/media/video_capture_message_filter.h',
    'renderer/media/video_frame_provider.cc',
    'renderer/media/video_frame_provider.h',
    'renderer/media/webaudiosourceprovider_impl.cc',
    'renderer/media/webaudiosourceprovider_impl.h',
    'renderer/media/webcontentdecryptionmodule_impl.cc',
    'renderer/media/webcontentdecryptionmodule_impl.h',
    'renderer/media/webcontentdecryptionmodulesession_impl.cc',
    'renderer/media/webcontentdecryptionmodulesession_impl.h',
    'renderer/media/webinbandtexttrack_impl.cc',
    'renderer/media/webinbandtexttrack_impl.h',
    'renderer/media/webmediaplayer_delegate.h',
    'renderer/media/webmediaplayer_impl.cc',
    'renderer/media/webmediaplayer_impl.h',
    'renderer/media/webmediaplayer_ms.cc',
    'renderer/media/webmediaplayer_ms.h',
    'renderer/media/webmediaplayer_params.cc',
    'renderer/media/webmediaplayer_params.h',
    'renderer/media/webmediaplayer_util.cc',
    'renderer/media/webmediaplayer_util.h',
    'renderer/media/webmediasource_impl.cc',
    'renderer/media/webmediasource_impl.h',
    'renderer/media/websourcebuffer_impl.cc',
    'renderer/media/websourcebuffer_impl.h',
    'renderer/memory_benchmarking_extension.cc',
    'renderer/memory_benchmarking_extension.h',
    'renderer/menu_item_builder.cc',
    'renderer/menu_item_builder.h',
    'renderer/mhtml_generator.cc',
    'renderer/mhtml_generator.h',
    'renderer/mouse_lock_dispatcher.cc',
    'renderer/mouse_lock_dispatcher.h',
    'renderer/notification_provider.cc',
    'renderer/notification_provider.h',
    'renderer/paint_aggregator.cc',
    'renderer/paint_aggregator.h',
    'renderer/pepper/audio_helper.cc',
    'renderer/pepper/audio_helper.h',
    'renderer/pepper/common.h',
    'renderer/pepper/content_decryptor_delegate.cc',
    'renderer/pepper/content_decryptor_delegate.h',
    'renderer/pepper/content_renderer_pepper_host_factory.cc',
    'renderer/pepper/content_renderer_pepper_host_factory.h',
    'renderer/pepper/event_conversion.cc',
    'renderer/pepper/event_conversion.h',
    'renderer/pepper/fullscreen_container.h',
    'renderer/pepper/gfx_conversion.h',
    'renderer/pepper/host_array_buffer_var.cc',
    'renderer/pepper/host_array_buffer_var.h',
    'renderer/pepper/host_dispatcher_wrapper.cc',
    'renderer/pepper/host_dispatcher_wrapper.h',
    'renderer/pepper/host_globals.cc',
    'renderer/pepper/host_globals.h',
    'renderer/pepper/host_var_tracker.cc',
    'renderer/pepper/host_var_tracker.h',
    'renderer/pepper/message_channel.cc',
    'renderer/pepper/message_channel.h',
    'renderer/pepper/npapi_glue.cc',
    'renderer/pepper/npapi_glue.h',
    'renderer/pepper/npobject_var.cc',
    'renderer/pepper/npobject_var.h',
    'renderer/pepper/pepper_audio_input_host.cc',
    'renderer/pepper/pepper_audio_input_host.h',
    'renderer/pepper/pepper_broker.cc',
    'renderer/pepper/pepper_broker.h',
    'renderer/pepper/pepper_browser_connection.cc',
    'renderer/pepper/pepper_browser_connection.h',
    'renderer/pepper/pepper_device_enumeration_host_helper.cc',
    'renderer/pepper/pepper_device_enumeration_host_helper.h',
    'renderer/pepper/pepper_file_chooser_host.cc',
    'renderer/pepper/pepper_file_chooser_host.h',
    'renderer/pepper/pepper_file_io_host.cc',
    'renderer/pepper/pepper_file_io_host.h',
    'renderer/pepper/pepper_file_system_host.cc',
    'renderer/pepper/pepper_file_system_host.h',
    'renderer/pepper/pepper_graphics_2d_host.cc',
    'renderer/pepper/pepper_graphics_2d_host.h',
    'renderer/pepper/pepper_hung_plugin_filter.cc',
    'renderer/pepper/pepper_hung_plugin_filter.h',
    'renderer/pepper/pepper_in_process_resource_creation.cc',
    'renderer/pepper/pepper_in_process_resource_creation.h',
    'renderer/pepper/pepper_in_process_router.cc',
    'renderer/pepper/pepper_in_process_router.h',
    'renderer/pepper/pepper_media_device_manager.cc',
    'renderer/pepper/pepper_media_device_manager.h',
    'renderer/pepper/pepper_platform_audio_input.cc',
    'renderer/pepper/pepper_platform_audio_input.h',
    'renderer/pepper/pepper_platform_audio_output.cc',
    'renderer/pepper/pepper_platform_audio_output.h',
    'renderer/pepper/pepper_platform_context_3d.cc',
    'renderer/pepper/pepper_platform_context_3d.h',
    'renderer/pepper/pepper_platform_video_capture.cc',
    'renderer/pepper/pepper_platform_video_capture.h',
    'renderer/pepper/pepper_plugin_instance_impl.cc',
    'renderer/pepper/pepper_plugin_instance_impl.h',
    'renderer/pepper/pepper_plugin_registry.cc',
    'renderer/pepper/pepper_plugin_registry.h',
    'renderer/pepper/pepper_proxy_channel_delegate_impl.cc',
    'renderer/pepper/pepper_proxy_channel_delegate_impl.h',
    'renderer/pepper/pepper_truetype_font.h',
    'renderer/pepper/pepper_truetype_font_android.cc',
    'renderer/pepper/pepper_truetype_font_host.cc',
    'renderer/pepper/pepper_truetype_font_host.h',
    'renderer/pepper/pepper_truetype_font_linux.cc',
    'renderer/pepper/pepper_truetype_font_mac.mm',
    'renderer/pepper/pepper_truetype_font_win.cc',
    'renderer/pepper/pepper_url_loader_host.cc',
    'renderer/pepper/pepper_url_loader_host.h',
    'renderer/pepper/pepper_video_capture_host.cc',
    'renderer/pepper/pepper_video_capture_host.h',
    'renderer/pepper/pepper_webplugin_impl.cc',
    'renderer/pepper/pepper_webplugin_impl.h',
    'renderer/pepper/pepper_websocket_host.cc',
    'renderer/pepper/pepper_websocket_host.h',
    'renderer/pepper/plugin_module.cc',
    'renderer/pepper/plugin_module.h',
    'renderer/pepper/plugin_object.cc',
    'renderer/pepper/plugin_object.h',
    'renderer/pepper/ppb_audio_impl.cc',
    'renderer/pepper/ppb_audio_impl.h',
    'renderer/pepper/ppb_broker_impl.cc',
    'renderer/pepper/ppb_broker_impl.h',
    'renderer/pepper/ppb_buffer_impl.cc',
    'renderer/pepper/ppb_buffer_impl.h',
    'renderer/pepper/ppb_file_ref_impl.cc',
    'renderer/pepper/ppb_file_ref_impl.h',
    'renderer/pepper/ppb_flash_message_loop_impl.cc',
    'renderer/pepper/ppb_flash_message_loop_impl.h',
    'renderer/pepper/ppb_graphics_3d_impl.cc',
    'renderer/pepper/ppb_graphics_3d_impl.h',
    'renderer/pepper/ppb_image_data_impl.cc',
    'renderer/pepper/ppb_image_data_impl.h',
    'renderer/pepper/ppb_proxy_impl.cc',
    'renderer/pepper/ppb_proxy_impl.h',
    'renderer/pepper/ppb_scrollbar_impl.cc',
    'renderer/pepper/ppb_scrollbar_impl.h',
    'renderer/pepper/ppb_uma_private_impl.cc',
    'renderer/pepper/ppb_uma_private_impl.h',
    'renderer/pepper/ppb_var_deprecated_impl.cc',
    'renderer/pepper/ppb_var_deprecated_impl.h',
    'renderer/pepper/ppb_video_decoder_impl.cc',
    'renderer/pepper/ppb_video_decoder_impl.h',
    'renderer/pepper/ppb_widget_impl.cc',
    'renderer/pepper/ppb_widget_impl.h',
    'renderer/pepper/quota_file_io.cc',
    'renderer/pepper/quota_file_io.h',
    'renderer/pepper/renderer_ppapi_host_impl.cc',
    'renderer/pepper/renderer_ppapi_host_impl.h',
    'renderer/pepper/renderer_restrict_dispatch_group.h',
    'renderer/pepper/resource_converter.cc',
    'renderer/pepper/resource_converter.h',
    'renderer/pepper/resource_creation_impl.cc',
    'renderer/pepper/resource_creation_impl.h',
    'renderer/pepper/url_request_info_util.cc',
    'renderer/pepper/url_request_info_util.h',
    'renderer/pepper/url_response_info_util.cc',
    'renderer/pepper/url_response_info_util.h',
    'renderer/pepper/usb_key_code_conversion.h',
    'renderer/pepper/usb_key_code_conversion.cc',
    'renderer/pepper/usb_key_code_conversion_linux.cc',
    'renderer/pepper/usb_key_code_conversion_mac.cc',
    'renderer/pepper/usb_key_code_conversion_win.cc',
    'renderer/pepper/v8_var_converter.cc',
    'renderer/pepper/v8_var_converter.h',
    'renderer/browser_plugin/browser_plugin.cc',
    'renderer/browser_plugin/browser_plugin.h',
    'renderer/browser_plugin/browser_plugin_backing_store.h',
    'renderer/browser_plugin/browser_plugin_backing_store.cc',
    'renderer/browser_plugin/browser_plugin_bindings.h',
    'renderer/browser_plugin/browser_plugin_bindings.cc',
    'renderer/browser_plugin/browser_plugin_manager.h',
    'renderer/browser_plugin/browser_plugin_manager.cc',
    'renderer/browser_plugin/browser_plugin_manager_factory.h',
    'renderer/browser_plugin/browser_plugin_manager_impl.h',
    'renderer/browser_plugin/browser_plugin_manager_impl.cc',
    'renderer/browser_plugin/browser_plugin_compositing_helper.h',
    'renderer/browser_plugin/browser_plugin_compositing_helper.cc',
    'renderer/context_menu_params_builder.cc',
    'renderer/context_menu_params_builder.h',
    'renderer/date_time_formatter.cc',
    'renderer/date_time_formatter.h',
    'renderer/fetchers/alt_error_page_resource_fetcher.cc',
    'renderer/fetchers/alt_error_page_resource_fetcher.h',
    'renderer/fetchers/image_resource_fetcher.cc',
    'renderer/fetchers/image_resource_fetcher.h',
    'renderer/fetchers/multi_resolution_image_resource_fetcher.cc',
    'renderer/fetchers/multi_resolution_image_resource_fetcher.h',
    'renderer/fetchers/resource_fetcher.cc',
    'renderer/fetchers/resource_fetcher.h',
    'renderer/ime_event_guard.cc',
    'renderer/ime_event_guard.h',
    'renderer/npapi/plugin_channel_host.cc',
    'renderer/npapi/plugin_channel_host.h',
    'renderer/npapi/webplugin_delegate_proxy.cc',
    'renderer/npapi/webplugin_delegate_proxy.h',
    'renderer/npapi/webplugin_impl.cc',
    'renderer/npapi/webplugin_impl.h',
    'renderer/render_frame_impl.cc',
    'renderer/render_frame_impl.h',
    'renderer/render_process.h',
    'renderer/render_process_impl.cc',
    'renderer/render_process_impl.h',
    'renderer/render_thread_impl.cc',
    'renderer/render_thread_impl.h',
    'renderer/render_view_impl.cc',
    'renderer/render_view_impl.h',
    'renderer/render_view_impl_android.cc',
    'renderer/render_view_impl_params.cc',
    'renderer/render_view_impl_params.h',
    'renderer/render_view_linux.cc',
    'renderer/render_view_mouse_lock_dispatcher.cc',
    'renderer/render_view_mouse_lock_dispatcher.h',
    'renderer/render_widget.cc',
    'renderer/render_widget.h',
    'renderer/render_widget_fullscreen.cc',
    'renderer/render_widget_fullscreen.h',
    'renderer/render_widget_fullscreen_pepper.cc',
    'renderer/render_widget_fullscreen_pepper.h',
    'renderer/renderer_clipboard_client.cc',
    'renderer/renderer_clipboard_client.h',
    'renderer/renderer_date_time_picker.cc',
    'renderer/renderer_date_time_picker.h',
    'renderer/renderer_main.cc',
    'renderer/renderer_main_platform_delegate.h',
    'renderer/renderer_main_platform_delegate_android.cc',
    'renderer/renderer_main_platform_delegate_linux.cc',
    'renderer/renderer_main_platform_delegate_mac.mm',
    'renderer/renderer_main_platform_delegate_win.cc',
    'renderer/renderer_webapplicationcachehost_impl.cc',
    'renderer/renderer_webapplicationcachehost_impl.h',
    'renderer/renderer_webcookiejar_impl.cc',
    'renderer/renderer_webcookiejar_impl.h',
    'renderer/renderer_webcolorchooser_impl.cc',
    'renderer/renderer_webcolorchooser_impl.h',
    'renderer/renderer_webkitplatformsupport_impl.cc',
    'renderer/renderer_webkitplatformsupport_impl.h',
    'renderer/sad_plugin.cc',
    'renderer/sad_plugin.h',
    'renderer/savable_resources.cc',
    'renderer/savable_resources.h',
    'renderer/scoped_clipboard_writer_glue.cc',
    'renderer/scoped_clipboard_writer_glue.h',
    'renderer/shared_memory_seqlock_reader.cc',
    'renderer/shared_memory_seqlock_reader.h',
    'renderer/skia_benchmarking_extension.cc',
    'renderer/skia_benchmarking_extension.h',
    'renderer/speech_recognition_dispatcher.cc',
    'renderer/speech_recognition_dispatcher.h',
    'renderer/stats_collection_controller.cc',
    'renderer/stats_collection_controller.h',
    'renderer/stats_collection_observer.cc',
    'renderer/stats_collection_observer.h',
    'renderer/text_input_client_observer.cc',
    'renderer/text_input_client_observer.h',
    'renderer/v8_value_converter_impl.cc',
    'renderer/v8_value_converter_impl.h',
    'renderer/webclipboard_impl.cc',
    'renderer/webclipboard_impl.h',
    'renderer/web_preferences.cc',
    'renderer/web_ui_extension.cc',
    'renderer/web_ui_extension.h',
    'renderer/web_ui_extension_data.cc',
    'renderer/web_ui_extension_data.h',
    'renderer/webcrypto_impl.cc',
    'renderer/webcrypto_impl.h',
    'renderer/webcrypto_impl_nss.cc',
    'renderer/webcrypto_impl_openssl.cc',
    'renderer/websharedworker_proxy.cc',
    'renderer/websharedworker_proxy.h',
    'renderer/websharedworkerrepository_impl.cc',
    'renderer/websharedworkerrepository_impl.h',
  ],
  'conditions': [
    ['notifications==0', {
      'sources!': [
        'renderer/notification_provider.cc',
        'renderer/active_notification_tracker.cc',
      ],
    }],
    ['input_speech==0', {
      'sources!': [
        'renderer/input_tag_speech_dispatcher.cc',
        'renderer/input_tag_speech_dispatcher.h',
      ]
    }],
    ['toolkit_uses_gtk == 1', {
      'dependencies': [
        '../build/linux/system.gyp:gtk',
      ],
    }],
    ['OS=="mac"', {
      'sources!': [
        'common/process_watcher_posix.cc',
      ],
    }],
    ['OS=="win" and win_use_allocator_shim==1', {
      'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
      ],
    }],
    ['OS=="android"', {
      'sources!': [
        'renderer/accessibility/renderer_accessibility_focus_only.cc'
        'renderer/media/audio_decoder.cc',
        'renderer/media/filter_helpers.cc',
        'renderer/media/pepper_platform_video_decoder.cc',
        'renderer/media/webmediaplayer_impl.cc',
      ],
      'dependencies': [
        '../third_party/libphonenumber/libphonenumber.gyp:libphonenumber',
      ],
      'includes': [
        '../build/android/cpufeatures.gypi',
      ],
    }, {
      'sources!': [
        'renderer/java/java_bridge_channel.cc',
        'renderer/java/java_bridge_channel.h',
        'renderer/java/java_bridge_dispatcher.cc',
        'renderer/java/java_bridge_dispatcher.h',
      ],
    }],
    ['google_tv == 1', {
      'sources!': [
        'renderer/media/crypto/key_systems_info.cc',
      ],
    }],
    # TODO(jrg): remove the OS=="android" section?
    # http://crbug.com/113172
    # Understand better how media_stream_ is tied into Chromium.
    ['enable_webrtc==0 and OS=="android"', {
      'sources/': [
        ['exclude', '^renderer/media/media_stream_'],
      ],
    }],
    ['enable_webrtc==1', {
      'dependencies': [
        '../third_party/libjingle/libjingle.gyp:libjingle_webrtc',
        '../third_party/libjingle/libjingle.gyp:libpeerconnection',
        '../third_party/webrtc/modules/modules.gyp:audio_device',
        '<(DEPTH)/crypto/crypto.gyp:crypto',
      ],
      'sources': [
        'public/renderer/webrtc_log_message_delegate.h',
        'renderer/media/media_stream_center.cc',
        'renderer/media/media_stream_dependency_factory.cc',
        'renderer/media/media_stream_dispatcher.cc',
        'renderer/media/media_stream_impl.cc',
        'renderer/media/media_stream_registry_interface.h',
        'renderer/media/media_stream_source_observer.cc',
        'renderer/media/media_stream_source_observer.h',
        'renderer/media/native_handle_impl.cc',
        'renderer/media/native_handle_impl.h',
        'renderer/media/peer_connection_handler_base.cc',
        'renderer/media/peer_connection_handler_base.h',
        'renderer/media/peer_connection_identity_service.cc',
        'renderer/media/peer_connection_identity_service.h',
        'renderer/media/peer_connection_tracker.cc',
        'renderer/media/peer_connection_tracker.h',
        'renderer/media/remote_media_stream_impl.cc',
        'renderer/media/remote_media_stream_impl.h',
        'renderer/media/rtc_data_channel_handler.cc',
        'renderer/media/rtc_data_channel_handler.h',
        'renderer/media/rtc_dtmf_sender_handler.cc',
        'renderer/media/rtc_dtmf_sender_handler.h',
        'renderer/media/rtc_media_constraints.cc',
        'renderer/media/rtc_media_constraints.h',
        'renderer/media/rtc_peer_connection_handler.cc',
        'renderer/media/rtc_peer_connection_handler.h',
        'renderer/media/rtc_video_capture_delegate.cc',
        'renderer/media/rtc_video_capture_delegate.h',
        'renderer/media/rtc_video_capturer.cc',
        'renderer/media/rtc_video_capturer.h',
        'renderer/media/rtc_video_decoder.cc',
        'renderer/media/rtc_video_decoder.h',
        'renderer/media/rtc_video_decoder_factory.cc',
        'renderer/media/rtc_video_decoder_factory.h',
        'renderer/media/rtc_video_encoder.cc',
        'renderer/media/rtc_video_encoder.h',
        'renderer/media/rtc_video_encoder_factory.cc',
        'renderer/media/rtc_video_encoder_factory.h',
        'renderer/media/rtc_video_renderer.cc',
        'renderer/media/rtc_video_renderer.h',
        'renderer/media/video_destination_handler.cc',
        'renderer/media/video_destination_handler.h',
        'renderer/media/video_source_handler.cc',
        'renderer/media/video_source_handler.h',
        'renderer/media/webaudio_capturer_source.cc',
        'renderer/media/webaudio_capturer_source.h',
        'renderer/media/webrtc_audio_capturer.cc',
        'renderer/media/webrtc_audio_capturer.h',
        'renderer/media/webrtc_audio_capturer_sink_owner.cc',
        'renderer/media/webrtc_audio_capturer_sink_owner.h',
        'renderer/media/webrtc_audio_device_impl.cc',
        'renderer/media/webrtc_audio_device_impl.h',
        'renderer/media/webrtc_audio_device_not_impl.cc',
        'renderer/media/webrtc_audio_device_not_impl.h',
        'renderer/media/webrtc_audio_renderer.cc',
        'renderer/media/webrtc_audio_renderer.h',
        'renderer/media/webrtc_identity_service.cc',
        'renderer/media/webrtc_identity_service.h',
        'renderer/media/webrtc_local_audio_renderer.cc',
        'renderer/media/webrtc_local_audio_renderer.h',
        'renderer/media/webrtc_local_audio_track.cc',
        'renderer/media/webrtc_local_audio_track.h',
        'renderer/media/webrtc_logging_initializer.cc',
        'renderer/media/webrtc_logging_initializer.h',
        'renderer/p2p/host_address_request.cc',
        'renderer/p2p/host_address_request.h',
        'renderer/p2p/ipc_network_manager.cc',
        'renderer/p2p/ipc_network_manager.h',
        'renderer/p2p/ipc_socket_factory.cc',
        'renderer/p2p/ipc_socket_factory.h',
        'renderer/p2p/network_list_observer.h',
        'renderer/p2p/port_allocator.cc',
        'renderer/p2p/port_allocator.h',
        'renderer/p2p/socket_client.cc',
        'renderer/p2p/socket_client.h',
        'renderer/p2p/socket_dispatcher.cc',
        'renderer/p2p/socket_dispatcher.h',
        'renderer/pepper/pepper_video_destination_host.cc',
        'renderer/pepper/pepper_video_destination_host.h',
        'renderer/pepper/pepper_video_source_host.cc',
        'renderer/pepper/pepper_video_source_host.h',
      ],
    }],
    ['enable_webrtc==1 and google_tv==1', {
      'sources': [
        'renderer/media/rtc_video_decoder_bridge_tv.cc',
        'renderer/media/rtc_video_decoder_bridge_tv.h',
        'renderer/media/rtc_video_decoder_factory_tv.cc',
        'renderer/media/rtc_video_decoder_factory_tv.h',
      ],
    }],
    ['enable_plugins==1', {
      'dependencies': [
        '../ppapi/ppapi_internal.gyp:ppapi_host',
        '../ppapi/ppapi_internal.gyp:ppapi_proxy',
        '../ppapi/ppapi_internal.gyp:ppapi_shared',
      ],
    }, {  # enable_plugins==0
      'sources/': [
        ['exclude', '^renderer/npapi/'],
        ['exclude', '^renderer/pepper/'],
      ],
      'sources!': [
        'renderer/media/video_destination_handler.cc',
        'renderer/media/video_destination_handler.h',
        'renderer/render_widget_fullscreen_pepper.cc',
        'renderer/render_widget_fullscreen_pepper.h',
      ],
    }],
    ['enable_pepper_cdms != 1', {
      'sources!': [
        'renderer/media/crypto/ppapi_decryptor.cc',
        'renderer/media/crypto/ppapi_decryptor.h',
      ],
    }],
    ['enable_gpu!=1', {
      'sources!': [
        'renderer/pepper/ppb_graphics_3d_impl.cc',
        'renderer/pepper/ppb_graphics_3d_impl.h',
        'renderer/pepper/ppb_open_gl_es_impl.cc',
      ],
    }],
    ['use_openssl==1', {
      'sources!': [
        'renderer/webcrypto_impl_nss.cc',
      ],
      'dependencies': [
        '../third_party/openssl/openssl.gyp:openssl',
      ],
    }, {
      'sources!': [
        'renderer/webcrypto_impl_openssl.cc',
      ],
      'conditions': [
        ['os_posix == 1 and OS != "mac" and OS != "ios" and OS != "android"', {
          'dependencies': [
            '../build/linux/system.gyp:ssl',
          ],
        }, {
          'dependencies': [
            '../third_party/nss/nss.gyp:nspr',
            '../third_party/nss/nss.gyp:nss',
          ],
        }],
      ],
    }],
  ],
  'target_conditions': [
    ['OS=="android"', {
      'sources/': [
        ['include', '^renderer/render_view_linux\\.cc$'],
      ],
    }],
  ],
}
