# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'webkit_storage_browser',
      'type': '<(component)',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/sql/sql.gyp:sql',
        '<(DEPTH)/third_party/WebKit/public/blink.gyp:blink',
        '<(DEPTH)/third_party/leveldatabase/leveldatabase.gyp:leveldatabase',
        '<(DEPTH)/third_party/sqlite/sqlite.gyp:sqlite',
        '<(DEPTH)/webkit/base/webkit_base.gyp:webkit_base',
        '<(DEPTH)/webkit/storage_common.gyp:webkit_storage_common',
      ],
      'defines': ['WEBKIT_STORAGE_IMPLEMENTATION'],
      'sources': [
        # TODO(kinuko): Fix this export.
        'storage/webkit_storage_export.h',
        'browser/appcache/appcache.cc',
        'browser/appcache/appcache.h',
        'browser/appcache/appcache_backend_impl.cc',
        'browser/appcache/appcache_backend_impl.h',
        'browser/appcache/appcache_database.cc',
        'browser/appcache/appcache_database.h',
        'browser/appcache/appcache_disk_cache.cc',
        'browser/appcache/appcache_disk_cache.h',
        'browser/appcache/appcache_entry.h',
        'browser/appcache/appcache_group.cc',
        'browser/appcache/appcache_group.h',
        'browser/appcache/appcache_histograms.cc',
        'browser/appcache/appcache_histograms.h',
        'browser/appcache/appcache_host.cc',
        'browser/appcache/appcache_host.h',
        'browser/appcache/appcache_interceptor.cc',
        'browser/appcache/appcache_interceptor.h',
        'browser/appcache/appcache_policy.h',
        'browser/appcache/appcache_quota_client.cc',
        'browser/appcache/appcache_quota_client.h',
        'browser/appcache/appcache_request_handler.cc',
        'browser/appcache/appcache_request_handler.h',
        'browser/appcache/appcache_response.cc',
        'browser/appcache/appcache_response.h',
        'browser/appcache/appcache_service.cc',
        'browser/appcache/appcache_service.h',
        'browser/appcache/appcache_storage.cc',
        'browser/appcache/appcache_storage.h',
        'browser/appcache/appcache_storage_impl.cc',
        'browser/appcache/appcache_storage_impl.h',
        'browser/appcache/appcache_working_set.cc',
        'browser/appcache/appcache_working_set.h',
        'browser/appcache/appcache_update_job.cc',
        'browser/appcache/appcache_update_job.h',
        'browser/appcache/appcache_url_request_job.cc',
        'browser/appcache/appcache_url_request_job.h',
        'browser/appcache/manifest_parser.cc',
        'browser/appcache/manifest_parser.h',
        'browser/appcache/view_appcache_internals_job.h',
        'browser/appcache/view_appcache_internals_job.cc',
        'browser/blob/blob_data_handle.cc',
        'browser/blob/blob_data_handle.h',
        'browser/blob/blob_storage_context.cc',
        'browser/blob/blob_storage_context.h',
        'browser/blob/blob_storage_controller.cc',
        'browser/blob/blob_storage_controller.h',
        'browser/blob/blob_storage_host.cc',
        'browser/blob/blob_storage_host.h',
        'browser/blob/blob_url_request_job.cc',
        'browser/blob/blob_url_request_job.h',
        'browser/blob/blob_url_request_job_factory.cc',
        'browser/blob/blob_url_request_job_factory.h',
        'browser/blob/local_file_stream_reader.cc',
        'browser/blob/local_file_stream_reader.h',
        'browser/blob/view_blob_internals_job.cc',
        'browser/blob/view_blob_internals_job.h',
        'browser/database/database_quota_client.cc',
        'browser/database/database_quota_client.h',
        'browser/database/database_tracker.cc',
        'browser/database/database_tracker.h',
        'browser/database/database_util.cc',
        'browser/database/database_util.h',
        'browser/database/databases_table.cc',
        'browser/database/databases_table.h',
        'browser/database/vfs_backend.cc',
        'browser/database/vfs_backend.h',
        'browser/dom_storage/dom_storage_area.cc',
        'browser/dom_storage/dom_storage_area.h',
        'browser/dom_storage/dom_storage_context.cc',
        'browser/dom_storage/dom_storage_context.h',
        'browser/dom_storage/dom_storage_database.cc',
        'browser/dom_storage/dom_storage_database.h',
        'browser/dom_storage/dom_storage_database_adapter.h',
        'browser/dom_storage/dom_storage_host.cc',
        'browser/dom_storage/dom_storage_host.h',
        'browser/dom_storage/dom_storage_namespace.cc',
        'browser/dom_storage/dom_storage_namespace.h',
        'browser/dom_storage/dom_storage_session.cc',
        'browser/dom_storage/dom_storage_session.h',
        'browser/dom_storage/dom_storage_task_runner.cc',
        'browser/dom_storage/dom_storage_task_runner.h',
        'browser/dom_storage/local_storage_database_adapter.cc',
        'browser/dom_storage/local_storage_database_adapter.h',
        'browser/dom_storage/session_storage_database.cc',
        'browser/dom_storage/session_storage_database.h',
        'browser/dom_storage/session_storage_database_adapter.cc',
        'browser/dom_storage/session_storage_database_adapter.h',
        'browser/fileapi/async_file_util.h',
        'browser/fileapi/async_file_util_adapter.cc',
        'browser/fileapi/async_file_util_adapter.h',
        'browser/fileapi/copy_or_move_file_validator.h',
        'browser/fileapi/copy_or_move_operation_delegate.cc',
        'browser/fileapi/copy_or_move_operation_delegate.h',
        'browser/fileapi/external_mount_points.cc',
        'browser/fileapi/external_mount_points.h',
        'browser/fileapi/file_observers.h',
        'browser/fileapi/file_permission_policy.cc',
        'browser/fileapi/file_permission_policy.h',
        'browser/fileapi/file_stream_writer.h',
        'browser/fileapi/file_system_context.cc',
        'browser/fileapi/file_system_context.h',
        'browser/fileapi/file_system_dir_url_request_job.cc',
        'browser/fileapi/file_system_dir_url_request_job.h',
        'browser/fileapi/file_system_file_stream_reader.cc',
        'browser/fileapi/file_system_file_stream_reader.h',
        'browser/fileapi/file_system_file_util.cc',
        'browser/fileapi/file_system_file_util.h',
        'browser/fileapi/file_system_mount_point_provider.h',
        'browser/fileapi/file_system_operation.h',
        'browser/fileapi/file_system_operation_context.cc',
        'browser/fileapi/file_system_operation_context.h',
        'browser/fileapi/file_system_operation_runner.cc',
        'browser/fileapi/file_system_operation_runner.h',
        'browser/fileapi/file_system_options.cc',
        'browser/fileapi/file_system_options.h',
        'browser/fileapi/file_system_quota_client.cc',
        'browser/fileapi/file_system_quota_client.h',
        'browser/fileapi/file_system_quota_util.h',
        'browser/fileapi/file_system_task_runners.cc',
        'browser/fileapi/file_system_task_runners.h',
        'browser/fileapi/file_system_url.cc',
        'browser/fileapi/file_system_url.h',
        'browser/fileapi/file_system_url_request_job.cc',
        'browser/fileapi/file_system_url_request_job.h',
        'browser/fileapi/file_system_url_request_job_factory.cc',
        'browser/fileapi/file_system_url_request_job_factory.h',
        'browser/fileapi/file_system_usage_cache.cc',
        'browser/fileapi/file_system_usage_cache.h',
        'browser/fileapi/file_writer_delegate.cc',
        'browser/fileapi/file_writer_delegate.h',
        'browser/fileapi/isolated_context.cc',
        'browser/fileapi/isolated_context.h',
        'browser/fileapi/isolated_file_util.cc',
        'browser/fileapi/isolated_file_util.h',
        'browser/fileapi/isolated_mount_point_provider.cc',
        'browser/fileapi/isolated_mount_point_provider.h',
        'browser/fileapi/local_file_stream_writer.cc',
        'browser/fileapi/local_file_stream_writer.h',
        'browser/fileapi/local_file_system_operation.cc',
        'browser/fileapi/local_file_system_operation.h',
        'browser/fileapi/local_file_util.cc',
        'browser/fileapi/local_file_util.h',
        'browser/fileapi/mount_points.cc',
        'browser/fileapi/mount_points.h',
        'browser/fileapi/native_file_util.cc',
        'browser/fileapi/native_file_util.h',
        'browser/fileapi/obfuscated_file_util.cc',
        'browser/fileapi/obfuscated_file_util.h',
        'browser/fileapi/open_file_system_mode.h',
        'browser/fileapi/recursive_operation_delegate.cc',
        'browser/fileapi/recursive_operation_delegate.h',
        'browser/fileapi/remote_file_system_proxy.h',
        'browser/fileapi/remove_operation_delegate.cc',
        'browser/fileapi/remove_operation_delegate.h',
        'browser/fileapi/sandbox_directory_database.cc',
        'browser/fileapi/sandbox_directory_database.h',
        'browser/fileapi/sandbox_file_stream_writer.cc',
        'browser/fileapi/sandbox_file_stream_writer.h',
        'browser/fileapi/sandbox_isolated_origin_database.cc',
        'browser/fileapi/sandbox_isolated_origin_database.h',
        'browser/fileapi/sandbox_mount_point_provider.cc',
        'browser/fileapi/sandbox_mount_point_provider.h',
        'browser/fileapi/sandbox_origin_database.cc',
        'browser/fileapi/sandbox_origin_database.h',
        'browser/fileapi/sandbox_origin_database_interface.cc',
        'browser/fileapi/sandbox_origin_database_interface.h',
        'browser/fileapi/sandbox_quota_observer.cc',
        'browser/fileapi/sandbox_quota_observer.h',
        'browser/fileapi/syncable/file_change.cc',
        'browser/fileapi/syncable/file_change.h',
        'browser/fileapi/syncable/local_file_change_tracker.cc',
        'browser/fileapi/syncable/local_file_change_tracker.h',
        'browser/fileapi/syncable/local_file_sync_context.cc',
        'browser/fileapi/syncable/local_file_sync_context.h',
        'browser/fileapi/syncable/local_file_sync_status.cc',
        'browser/fileapi/syncable/local_file_sync_status.h',
        'browser/fileapi/syncable/local_origin_change_observer.h',
        'browser/fileapi/syncable/sync_action.h',
        'browser/fileapi/syncable/sync_callbacks.h',
        'browser/fileapi/syncable/sync_direction.h',
        'browser/fileapi/syncable/sync_file_metadata.cc',
        'browser/fileapi/syncable/sync_file_metadata.h',
        'browser/fileapi/syncable/sync_file_status.h',
        'browser/fileapi/syncable/sync_file_type.h',
        'browser/fileapi/syncable/sync_status_code.cc',
        'browser/fileapi/syncable/sync_status_code.h',
        'browser/fileapi/syncable/syncable_file_operation_runner.cc',
        'browser/fileapi/syncable/syncable_file_operation_runner.h',
        'browser/fileapi/syncable/syncable_file_system_operation.cc',
        'browser/fileapi/syncable/syncable_file_system_operation.h',
        'browser/fileapi/syncable/syncable_file_system_util.cc',
        'browser/fileapi/syncable/syncable_file_system_util.h',
        'browser/fileapi/task_runner_bound_observer_list.h',
        'browser/fileapi/test_mount_point_provider.cc',
        'browser/fileapi/test_mount_point_provider.h',
        'browser/fileapi/transient_file_util.cc',
        'browser/fileapi/transient_file_util.h',
        'browser/fileapi/upload_file_system_file_element_reader.cc',
        'browser/fileapi/upload_file_system_file_element_reader.h',
        'browser/quota/quota_callbacks.h',
        'browser/quota/quota_client.h',
        'browser/quota/quota_database.cc',
        'browser/quota/quota_database.h',
        'browser/quota/quota_manager.cc',
        'browser/quota/quota_manager.h',
        'browser/quota/quota_task.cc',
        'browser/quota/quota_task.h',
        'browser/quota/quota_temporary_storage_evictor.cc',
        'browser/quota/quota_temporary_storage_evictor.h',
        'browser/quota/special_storage_policy.cc',
        'browser/quota/special_storage_policy.h',
        'browser/quota/usage_tracker.cc',
        'browser/quota/usage_tracker.h',
      ],
      'conditions': [
        ['chromeos==1', {
          'sources': [
            'browser/chromeos/fileapi/async_file_stream.h',
            'browser/chromeos/fileapi/cros_mount_point_provider.cc',
            'browser/chromeos/fileapi/cros_mount_point_provider.h',
            'browser/chromeos/fileapi/file_access_permissions.cc',
            'browser/chromeos/fileapi/file_access_permissions.h',
            'browser/chromeos/fileapi/file_util_async.h',
            'browser/chromeos/fileapi/remote_file_system_operation.cc',
            'browser/chromeos/fileapi/remote_file_system_operation.h',
            'browser/chromeos/fileapi/remote_file_stream_writer.cc',
            'browser/chromeos/fileapi/remote_file_stream_writer.h',
          ],
        }],
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
  ],
}
