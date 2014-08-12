# This file is generated by gyp; do not edit.

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE := base_base_gyp
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_TARGET_ARCH := $(TARGET_$(GYP_VAR_PREFIX)ARCH)
gyp_intermediate_dir := $(call local-intermediates-dir,,$(GYP_VAR_PREFIX))
gyp_shared_intermediate_dir := $(call intermediates-dir-for,GYP,shared,,,$(GYP_VAR_PREFIX))

# Make sure our deps are built first.
GYP_TARGET_DEPENDENCIES := \
	$(call intermediates-dir-for,GYP,testing_gtest_prod_gyp,,,$(GYP_VAR_PREFIX))/gtest_prod.stamp \
	$(call intermediates-dir-for,GYP,base_base_jni_headers_gyp,,,$(GYP_VAR_PREFIX))/base_jni_headers.stamp \
	$(call intermediates-dir-for,GYP,third_party_ashmem_ashmem_gyp,,,$(GYP_VAR_PREFIX))/ashmem.stamp

GYP_GENERATED_OUTPUTS :=

# Make sure our deps and generated files are built first.
LOCAL_ADDITIONAL_DEPENDENCIES := $(GYP_TARGET_DEPENDENCIES) $(GYP_GENERATED_OUTPUTS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_GENERATED_SOURCES :=

GYP_COPIED_SOURCE_ORIGIN_DIRS :=

LOCAL_SRC_FILES := \
	base/async_socket_io_handler_posix.cc \
	base/event_recorder_stubs.cc \
	base/linux_util.cc \
	base/message_loop/message_pump_android.cc \
	base/message_loop/message_pump_libevent.cc \
	base/metrics/field_trial.cc \
	base/posix/file_descriptor_shuffle.cc \
	base/sync_socket_posix.cc \
	base/third_party/dmg_fp/g_fmt.cc \
	base/third_party/dmg_fp/dtoa_wrapper.cc \
	base/third_party/icu/icu_utf.cc \
	base/third_party/nspr/prtime.cc \
	base/third_party/superfasthash/superfasthash.c \
	base/allocator/allocator_extension.cc \
	base/allocator/type_profiler_control.cc \
	base/android/application_status_listener.cc \
	base/android/base_jni_registrar.cc \
	base/android/build_info.cc \
	base/android/command_line_android.cc \
	base/android/content_uri_utils.cc \
	base/android/cpu_features.cc \
	base/android/event_log.cc \
	base/android/field_trial_list.cc \
	base/android/fifo_utils.cc \
	base/android/important_file_writer_android.cc \
	base/android/scoped_java_ref.cc \
	base/android/jni_android.cc \
	base/android/jni_array.cc \
	base/android/jni_registrar.cc \
	base/android/jni_string.cc \
	base/android/jni_weak_ref.cc \
	base/android/library_loader/library_loader_hooks.cc \
	base/android/memory_pressure_listener_android.cc \
	base/android/java_handler_thread.cc \
	base/android/path_service_android.cc \
	base/android/path_utils.cc \
	base/android/sys_utils.cc \
	base/android/trace_event_binding.cc \
	base/at_exit.cc \
	base/barrier_closure.cc \
	base/base64.cc \
	base/base_paths.cc \
	base/base_paths_android.cc \
	base/big_endian.cc \
	base/bind_helpers.cc \
	base/build_time.cc \
	base/callback_helpers.cc \
	base/callback_internal.cc \
	base/command_line.cc \
	base/cpu.cc \
	base/debug/alias.cc \
	base/debug/asan_invalid_access.cc \
	base/debug/crash_logging.cc \
	base/debug/debugger.cc \
	base/debug/debugger_posix.cc \
	base/debug/dump_without_crashing.cc \
	base/debug/proc_maps_linux.cc \
	base/debug/profiler.cc \
	base/debug/stack_trace.cc \
	base/debug/stack_trace_android.cc \
	base/debug/trace_event_android.cc \
	base/debug/trace_event_argument.cc \
	base/debug/trace_event_impl.cc \
	base/debug/trace_event_impl_constants.cc \
	base/debug/trace_event_synthetic_delay.cc \
	base/debug/trace_event_system_stats_monitor.cc \
	base/debug/trace_event_memory.cc \
	base/deferred_sequenced_task_runner.cc \
	base/environment.cc \
	base/file_util.cc \
	base/file_util_android.cc \
	base/file_util_posix.cc \
	base/files/file.cc \
	base/files/file_enumerator.cc \
	base/files/file_enumerator_posix.cc \
	base/files/file_path.cc \
	base/files/file_path_constants.cc \
	base/files/file_path_watcher.cc \
	base/files/file_path_watcher_linux.cc \
	base/files/file_posix.cc \
	base/files/file_proxy.cc \
	base/files/file_util_proxy.cc \
	base/files/important_file_writer.cc \
	base/files/memory_mapped_file.cc \
	base/files/memory_mapped_file_posix.cc \
	base/files/scoped_file.cc \
	base/files/scoped_temp_dir.cc \
	base/guid.cc \
	base/guid_posix.cc \
	base/hash.cc \
	base/ini_parser.cc \
	base/json/json_file_value_serializer.cc \
	base/json/json_parser.cc \
	base/json/json_reader.cc \
	base/json/json_string_value_serializer.cc \
	base/json/json_writer.cc \
	base/json/string_escape.cc \
	base/lazy_instance.cc \
	base/location.cc \
	base/logging.cc \
	base/md5.cc \
	base/memory/aligned_memory.cc \
	base/memory/discardable_memory.cc \
	base/memory/discardable_memory_android.cc \
	base/memory/discardable_memory_emulated.cc \
	base/memory/discardable_memory_malloc.cc \
	base/memory/discardable_memory_manager.cc \
	base/memory/memory_pressure_listener.cc \
	base/memory/ref_counted.cc \
	base/memory/ref_counted_memory.cc \
	base/memory/shared_memory_android.cc \
	base/memory/shared_memory_posix.cc \
	base/memory/singleton.cc \
	base/memory/weak_ptr.cc \
	base/message_loop/incoming_task_queue.cc \
	base/message_loop/message_loop.cc \
	base/message_loop/message_loop_proxy.cc \
	base/message_loop/message_loop_proxy_impl.cc \
	base/message_loop/message_pump.cc \
	base/message_loop/message_pump_default.cc \
	base/metrics/sample_map.cc \
	base/metrics/sample_vector.cc \
	base/metrics/bucket_ranges.cc \
	base/metrics/histogram.cc \
	base/metrics/histogram_base.cc \
	base/metrics/histogram_delta_serialization.cc \
	base/metrics/histogram_samples.cc \
	base/metrics/histogram_snapshot_manager.cc \
	base/metrics/sparse_histogram.cc \
	base/metrics/statistics_recorder.cc \
	base/metrics/stats_counters.cc \
	base/metrics/stats_table.cc \
	base/metrics/user_metrics.cc \
	base/native_library_posix.cc \
	base/os_compat_android.cc \
	base/path_service.cc \
	base/pending_task.cc \
	base/pickle.cc \
	base/posix/global_descriptors.cc \
	base/posix/unix_domain_socket_linux.cc \
	base/power_monitor/power_monitor.cc \
	base/power_monitor/power_monitor_device_source_android.cc \
	base/power_monitor/power_monitor_device_source.cc \
	base/power_monitor/power_monitor_source.cc \
	base/process/internal_linux.cc \
	base/process/kill.cc \
	base/process/kill_posix.cc \
	base/process/launch.cc \
	base/process/launch_posix.cc \
	base/process/memory.cc \
	base/process/memory_linux.cc \
	base/process/process_handle_linux.cc \
	base/process/process_handle_posix.cc \
	base/process/process_iterator.cc \
	base/process/process_iterator_linux.cc \
	base/process/process_metrics.cc \
	base/process/process_metrics_linux.cc \
	base/process/process_metrics_posix.cc \
	base/process/process_posix.cc \
	base/profiler/scoped_profile.cc \
	base/profiler/alternate_timer.cc \
	base/profiler/tracked_time.cc \
	base/rand_util.cc \
	base/rand_util_posix.cc \
	base/run_loop.cc \
	base/safe_strerror_posix.cc \
	base/scoped_native_library.cc \
	base/sequence_checker_impl.cc \
	base/sequenced_task_runner.cc \
	base/sha1_portable.cc \
	base/strings/latin1_string_conversions.cc \
	base/strings/nullable_string16.cc \
	base/strings/safe_sprintf.cc \
	base/strings/string16.cc \
	base/strings/string_number_conversions.cc \
	base/strings/string_split.cc \
	base/strings/string_piece.cc \
	base/strings/string_util.cc \
	base/strings/string_util_constants.cc \
	base/strings/stringprintf.cc \
	base/strings/sys_string_conversions_posix.cc \
	base/strings/utf_offset_string_conversions.cc \
	base/strings/utf_string_conversion_utils.cc \
	base/strings/utf_string_conversions.cc \
	base/supports_user_data.cc \
	base/synchronization/cancellation_flag.cc \
	base/synchronization/condition_variable_posix.cc \
	base/synchronization/lock.cc \
	base/synchronization/lock_impl_posix.cc \
	base/synchronization/waitable_event_posix.cc \
	base/synchronization/waitable_event_watcher_posix.cc \
	base/system_monitor/system_monitor.cc \
	base/sys_info.cc \
	base/sys_info_android.cc \
	base/sys_info_linux.cc \
	base/sys_info_posix.cc \
	base/task/cancelable_task_tracker.cc \
	base/task_runner.cc \
	base/thread_task_runner_handle.cc \
	base/threading/non_thread_safe_impl.cc \
	base/threading/platform_thread_android.cc \
	base/threading/platform_thread_posix.cc \
	base/threading/post_task_and_reply_impl.cc \
	base/threading/sequenced_worker_pool.cc \
	base/threading/simple_thread.cc \
	base/threading/thread.cc \
	base/threading/thread_checker_impl.cc \
	base/threading/thread_collision_warner.cc \
	base/threading/thread_id_name_manager.cc \
	base/threading/thread_local_android.cc \
	base/threading/thread_local_posix.cc \
	base/threading/thread_local_storage.cc \
	base/threading/thread_local_storage_posix.cc \
	base/threading/thread_restrictions.cc \
	base/threading/watchdog.cc \
	base/threading/worker_pool.cc \
	base/threading/worker_pool_posix.cc \
	base/time/clock.cc \
	base/time/default_clock.cc \
	base/time/default_tick_clock.cc \
	base/time/tick_clock.cc \
	base/time/time.cc \
	base/time/time_posix.cc \
	base/timer/elapsed_timer.cc \
	base/timer/hi_res_timer_manager_posix.cc \
	base/timer/mock_timer.cc \
	base/timer/timer.cc \
	base/tracked_objects.cc \
	base/tracking_info.cc \
	base/values.cc \
	base/value_conversions.cc \
	base/version.cc \
	base/vlog.cc \
	base/memory/discardable_memory_ashmem_allocator.cc \
	base/memory/discardable_memory_ashmem.cc


# Flags passed to both C and C++ files.
MY_CFLAGS_Debug := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	 \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wall \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-unused-local-typedefs \
	-EL \
	-mhard-float \
	-ffunction-sections \
	-funwind-tables \
	-g \
	-fstack-protector \
	-fno-short-enums \
	-finline-limit=64 \
	-Wa,--noexecstack \
	-U_FORTIFY_SOURCE \
	-Wno-extra \
	-Wno-ignored-qualifiers \
	-Wno-type-limits \
	-Wno-unused-but-set-variable \
	-Os \
	-g \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer \
	-funwind-tables

MY_DEFS_Debug := \
	'-DV8_DEPRECATION_WARNINGS' \
	'-DBLINK_SCALE_FILTERS_AT_RECORD_TIME' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_BROWSER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DDISCARDABLE_MEMORY_ALWAYS_SUPPORTED_NATIVELY' \
	'-DSYSTEM_NATIVELY_SIGNALS_MEMORY_PRESSURE' \
	'-DENABLE_EGLIMAGE=1' \
	'-DCLD_VERSION=1' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DDATA_REDUCTION_FALLBACK_HOST="http://compress.googlezip.net:80/"' \
	'-DDATA_REDUCTION_DEV_HOST="http://proxy-dev.googlezip.net:80/"' \
	'-DSPDY_PROXY_AUTH_ORIGIN="https://proxy.googlezip.net:443/"' \
	'-DDATA_REDUCTION_PROXY_PROBE_URL="http://check.googlezip.net/connect"' \
	'-DDATA_REDUCTION_PROXY_WARMUP_URL="http://www.gstatic.com/generate_204"' \
	'-DVIDEO_HOLE=1' \
	'-DUSE_OPENSSL=1' \
	'-DUSE_OPENSSL_CERTS=1' \
	'-D__STDC_CONSTANT_MACROS' \
	'-D__STDC_FORMAT_MACROS' \
	'-DBASE_IMPLEMENTATION' \
	'-DANDROID_SINCOS_PROVIDED' \
	'-DANDROID' \
	'-D__GNU_SOURCE=1' \
	'-DUSE_STLPORT=1' \
	'-D_STLP_USE_PTR_SPECIALIZATIONS=1' \
	'-DCHROME_BUILD_ID=""' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=1' \
	'-DWTF_USE_DYNAMIC_ANNOTATIONS=1' \
	'-D_DEBUG'


# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES_Debug := \
	$(gyp_shared_intermediate_dir)/shim_headers/ashmem/target \
	$(gyp_shared_intermediate_dir) \
	$(gyp_shared_intermediate_dir)/base \
	$(LOCAL_PATH) \
	$(PWD)/frameworks/wilhelm/include \
	$(PWD)/bionic \
	$(PWD)/external/stlport/stlport


# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS_Debug := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wsign-compare \
	-Wno-uninitialized \
	-std=gnu++11 \
	-Wno-narrowing \
	-Wno-literal-suffix \
	-Wno-non-virtual-dtor \
	-Wno-sign-promo


# Flags passed to both C and C++ files.
MY_CFLAGS_Release := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	 \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wall \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-unused-local-typedefs \
	-EL \
	-mhard-float \
	-ffunction-sections \
	-funwind-tables \
	-g \
	-fstack-protector \
	-fno-short-enums \
	-finline-limit=64 \
	-Wa,--noexecstack \
	-U_FORTIFY_SOURCE \
	-Wno-extra \
	-Wno-ignored-qualifiers \
	-Wno-type-limits \
	-Wno-unused-but-set-variable \
	-Os \
	-fno-ident \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer \
	-funwind-tables

MY_DEFS_Release := \
	'-DV8_DEPRECATION_WARNINGS' \
	'-DBLINK_SCALE_FILTERS_AT_RECORD_TIME' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_BROWSER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DDISCARDABLE_MEMORY_ALWAYS_SUPPORTED_NATIVELY' \
	'-DSYSTEM_NATIVELY_SIGNALS_MEMORY_PRESSURE' \
	'-DENABLE_EGLIMAGE=1' \
	'-DCLD_VERSION=1' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DDATA_REDUCTION_FALLBACK_HOST="http://compress.googlezip.net:80/"' \
	'-DDATA_REDUCTION_DEV_HOST="http://proxy-dev.googlezip.net:80/"' \
	'-DSPDY_PROXY_AUTH_ORIGIN="https://proxy.googlezip.net:443/"' \
	'-DDATA_REDUCTION_PROXY_PROBE_URL="http://check.googlezip.net/connect"' \
	'-DDATA_REDUCTION_PROXY_WARMUP_URL="http://www.gstatic.com/generate_204"' \
	'-DVIDEO_HOLE=1' \
	'-DUSE_OPENSSL=1' \
	'-DUSE_OPENSSL_CERTS=1' \
	'-D__STDC_CONSTANT_MACROS' \
	'-D__STDC_FORMAT_MACROS' \
	'-DBASE_IMPLEMENTATION' \
	'-DANDROID_SINCOS_PROVIDED' \
	'-DANDROID' \
	'-D__GNU_SOURCE=1' \
	'-DUSE_STLPORT=1' \
	'-D_STLP_USE_PTR_SPECIALIZATIONS=1' \
	'-DCHROME_BUILD_ID=""' \
	'-DNDEBUG' \
	'-DNVALGRIND' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=0' \
	'-D_FORTIFY_SOURCE=2'


# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES_Release := \
	$(gyp_shared_intermediate_dir)/shim_headers/ashmem/target \
	$(gyp_shared_intermediate_dir) \
	$(gyp_shared_intermediate_dir)/base \
	$(LOCAL_PATH) \
	$(PWD)/frameworks/wilhelm/include \
	$(PWD)/bionic \
	$(PWD)/external/stlport/stlport


# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS_Release := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wsign-compare \
	-Wno-uninitialized \
	-std=gnu++11 \
	-Wno-narrowing \
	-Wno-literal-suffix \
	-Wno-non-virtual-dtor \
	-Wno-sign-promo


LOCAL_CFLAGS := $(MY_CFLAGS_$(GYP_CONFIGURATION)) $(MY_DEFS_$(GYP_CONFIGURATION))
LOCAL_C_INCLUDES := $(GYP_COPIED_SOURCE_ORIGIN_DIRS) $(LOCAL_C_INCLUDES_$(GYP_CONFIGURATION))
LOCAL_CPPFLAGS := $(LOCAL_CPPFLAGS_$(GYP_CONFIGURATION))
LOCAL_ASFLAGS := $(LOCAL_CFLAGS)
### Rules for final target.

LOCAL_LDFLAGS_Debug := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-Wl,--fatal-warnings \
	-Wl,-z,noexecstack \
	-fPIC \
	-EL \
	-Wl,--no-keep-memory \
	-nostdlib \
	-Wl,--no-undefined \
	-Wl,--exclude-libs=ALL \
	-Wl,--warn-shared-textrel \
	-Wl,-O1 \
	-Wl,--as-needed


LOCAL_LDFLAGS_Release := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-Wl,--fatal-warnings \
	-Wl,-z,noexecstack \
	-fPIC \
	-EL \
	-Wl,--no-keep-memory \
	-nostdlib \
	-Wl,--no-undefined \
	-Wl,--exclude-libs=ALL \
	-Wl,-O1 \
	-Wl,--as-needed \
	-Wl,--gc-sections \
	-Wl,--warn-shared-textrel


LOCAL_LDFLAGS := $(LOCAL_LDFLAGS_$(GYP_CONFIGURATION))

LOCAL_STATIC_LIBRARIES := \
	cpufeatures

# Enable grouping to fix circular references
LOCAL_GROUP_STATIC_LIBRARIES := true

LOCAL_SHARED_LIBRARIES := \
	libstlport \
	libdl

# Add target alias to "gyp_all_modules" target.
.PHONY: gyp_all_modules
gyp_all_modules: base_base_gyp

# Alias gyp target name.
.PHONY: base
base: base_base_gyp

include $(BUILD_STATIC_LIBRARY)
