// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/tools/crash_service/crash_service.h"

#include <windows.h>

#include <sddl.h>
#include <fstream>
#include <map>

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/win/windows_version.h"
#include "breakpad/src/client/windows/crash_generation/client_info.h"
#include "breakpad/src/client/windows/crash_generation/crash_generation_server.h"
#include "breakpad/src/client/windows/sender/crash_report_sender.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_paths.h"

namespace {

const wchar_t kTestPipeName[] = L"\\\\.\\pipe\\ChromeCrashServices";

const wchar_t kCrashReportURL[] = L"https://clients2.google.com/cr/report";
const wchar_t kCheckPointFile[] = L"crash_checkpoint.txt";

typedef std::map<std::wstring, std::wstring> CrashMap;

bool CustomInfoToMap(const google_breakpad::ClientInfo* client_info,
                     const std::wstring& reporter_tag, CrashMap* map) {
  google_breakpad::CustomClientInfo info = client_info->GetCustomInfo();

  for (uintptr_t i = 0; i < info.count; ++i) {
    (*map)[info.entries[i].name] = info.entries[i].value;
  }

  (*map)[L"rept"] = reporter_tag;

  return !map->empty();
}

bool WriteCustomInfoToFile(const std::wstring& dump_path, const CrashMap& map) {
  std::wstring file_path(dump_path);
  size_t last_dot = file_path.rfind(L'.');
  if (last_dot == std::wstring::npos)
    return false;
  file_path.resize(last_dot);
  file_path += L".txt";

  std::wofstream file(file_path.c_str(),
      std::ios_base::out | std::ios_base::app | std::ios::binary);
  if (!file.is_open())
    return false;

  CrashMap::const_iterator pos;
  for (pos = map.begin(); pos != map.end(); ++pos) {
    std::wstring line = pos->first;
    line += L':';
    line += pos->second;
    line += L'\n';
    file.write(line.c_str(), static_cast<std::streamsize>(line.length()));
  }
  return true;
}

// The window procedure task is to handle when a) the user logs off.
// b) the system shuts down or c) when the user closes the window.
LRESULT __stdcall CrashSvcWndProc(HWND hwnd, UINT message,
                                  WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_CLOSE:
    case WM_ENDSESSION:
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hwnd, message, wparam, lparam);
  }
  return 0;
}

// This is the main and only application window.
HWND g_top_window = NULL;

bool CreateTopWindow(HINSTANCE instance, bool visible) {
  WNDCLASSEXW wcx = {0};
  wcx.cbSize = sizeof(wcx);
  wcx.style = CS_HREDRAW | CS_VREDRAW;
  wcx.lpfnWndProc = CrashSvcWndProc;
  wcx.hInstance = instance;
  wcx.lpszClassName = L"crash_svc_class";
  ATOM atom = ::RegisterClassExW(&wcx);
  DWORD style = visible ? WS_POPUPWINDOW | WS_VISIBLE : WS_OVERLAPPED;

  // The window size is zero but being a popup window still shows in the
  // task bar and can be closed using the system menu or using task manager.
  HWND window = CreateWindowExW(0, wcx.lpszClassName, L"crash service", style,
                                CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                                NULL, NULL, instance, NULL);
  if (!window)
    return false;

  ::UpdateWindow(window);
  VLOG(1) << "window handle is " << window;
  g_top_window = window;
  return true;
}

// Simple helper class to keep the process alive until the current request
// finishes.
class ProcessingLock {
 public:
  ProcessingLock() {
    ::InterlockedIncrement(&op_count_);
  }
  ~ProcessingLock() {
    ::InterlockedDecrement(&op_count_);
  }
  static bool IsWorking() {
    return (op_count_ != 0);
  }
 private:
  static volatile LONG op_count_;
};

volatile LONG ProcessingLock::op_count_ = 0;

// This structure contains the information that the worker thread needs to
// send a crash dump to the server.
struct DumpJobInfo {
  DWORD pid;
  CrashService* self;
  CrashMap map;
  std::wstring dump_path;

  DumpJobInfo(DWORD process_id, CrashService* service,
              const CrashMap& crash_map, const std::wstring& path)
      : pid(process_id), self(service), map(crash_map), dump_path(path) {
  }
};

}  // namespace

// Command line switches:
const char CrashService::kMaxReports[]        = "max-reports";
const char CrashService::kNoWindow[]          = "no-window";
const char CrashService::kReporterTag[]       = "reporter";
const char CrashService::kDumpsDir[]          = "dumps-dir";
const char CrashService::kPipeName[]          = "pipe-name";
const char CrashService::kArchiveDumpsByPid[] = "archive-dumps-by-pid";

bool CrashService::archive_dumps_by_pid_ = false;

CrashService::CrashService(const std::wstring& report_dir)
    : report_path_(report_dir),
      sender_(NULL),
      dumper_(NULL),
      requests_handled_(0),
      requests_sent_(0),
      clients_connected_(0),
      clients_terminated_(0) {
  chrome::RegisterPathProvider();
}

CrashService::~CrashService() {
  base::AutoLock lock(sending_);
  delete dumper_;
  delete sender_;
}


bool CrashService::Initialize(const std::wstring& command_line) {
  using google_breakpad::CrashReportSender;
  using google_breakpad::CrashGenerationServer;

  std::wstring pipe_name = kTestPipeName;
  int max_reports = -1;

  // The checkpoint file allows CrashReportSender to enforce the the maximum
  // reports per day quota. Does not seem to serve any other purpose.
  base::FilePath checkpoint_path = report_path_.Append(kCheckPointFile);

  // The dumps path is typically : '<user profile>\Local settings\
  // Application data\Goggle\Chrome\Crash Reports' and the report path is
  // Application data\Google\Chrome\Reported Crashes.txt
  base::FilePath user_data_dir;
  if (!PathService::Get(chrome::DIR_USER_DATA, &user_data_dir)) {
    LOG(ERROR) << "could not get DIR_USER_DATA";
    return false;
  }
  report_path_ = user_data_dir.Append(chrome::kCrashReportLog);

  CommandLine cmd_line = CommandLine::FromString(command_line);

  base::FilePath dumps_path;
  if (cmd_line.HasSwitch(kDumpsDir)) {
    dumps_path = base::FilePath(cmd_line.GetSwitchValueNative(kDumpsDir));
  } else {
    if (!PathService::Get(chrome::DIR_CRASH_DUMPS, &dumps_path)) {
      LOG(ERROR) << "could not get DIR_CRASH_DUMPS";
      return false;
    }
  }

  if (cmd_line.HasSwitch(kArchiveDumpsByPid))
    archive_dumps_by_pid_ = true;

  // We can override the send reports quota with a command line switch.
  if (cmd_line.HasSwitch(kMaxReports))
    max_reports = _wtoi(cmd_line.GetSwitchValueNative(kMaxReports).c_str());

  // Allow the global pipe name to be overridden for better testability.
  if (cmd_line.HasSwitch(kPipeName))
    pipe_name = cmd_line.GetSwitchValueNative(kPipeName);

#ifdef _WIN64
  pipe_name += L"-x64";
#endif

  if (max_reports > 0) {
    // Create the http sender object.
    sender_ = new CrashReportSender(checkpoint_path.value());
    if (!sender_) {
      LOG(ERROR) << "could not create sender";
      return false;
    }
    sender_->set_max_reports_per_day(max_reports);
  }

  SECURITY_ATTRIBUTES security_attributes = {0};
  SECURITY_ATTRIBUTES* security_attributes_actual = NULL;

  if (base::win::GetVersion() >= base::win::VERSION_VISTA) {
    SECURITY_DESCRIPTOR* security_descriptor =
        reinterpret_cast<SECURITY_DESCRIPTOR*>(
            GetSecurityDescriptorForLowIntegrity());
    DCHECK(security_descriptor != NULL);

    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.lpSecurityDescriptor = security_descriptor;
    security_attributes.bInheritHandle = FALSE;

    security_attributes_actual = &security_attributes;
  }

  // Create the OOP crash generator object.
  dumper_ = new CrashGenerationServer(pipe_name, security_attributes_actual,
                                      &CrashService::OnClientConnected, this,
                                      &CrashService::OnClientDumpRequest, this,
                                      &CrashService::OnClientExited, this,
                                      NULL, NULL,
                                      true, &dumps_path.value());

  if (!dumper_) {
    LOG(ERROR) << "could not create dumper";
    if (security_attributes.lpSecurityDescriptor)
      LocalFree(security_attributes.lpSecurityDescriptor);
    return false;
  }

  if (!CreateTopWindow(::GetModuleHandleW(NULL),
                       !cmd_line.HasSwitch(kNoWindow))) {
    LOG(ERROR) << "could not create window";
    if (security_attributes.lpSecurityDescriptor)
      LocalFree(security_attributes.lpSecurityDescriptor);
    return false;
  }

  reporter_tag_ = L"crash svc";
  if (cmd_line.HasSwitch(kReporterTag))
    reporter_tag_ = cmd_line.GetSwitchValueNative(kReporterTag);

  // Log basic information.
  VLOG(1) << "pipe name is " << pipe_name
          << "\ndumps at " << dumps_path.value()
          << "\nreports at " << report_path_.value();

  if (sender_) {
    VLOG(1) << "checkpoint is " << checkpoint_path.value()
            << "\nserver is " << kCrashReportURL
            << "\nmaximum " << sender_->max_reports_per_day() << " reports/day"
            << "\nreporter is " << reporter_tag_;
  }
  // Start servicing clients.
  if (!dumper_->Start()) {
    LOG(ERROR) << "could not start dumper";
    if (security_attributes.lpSecurityDescriptor)
      LocalFree(security_attributes.lpSecurityDescriptor);
    return false;
  }

  if (security_attributes.lpSecurityDescriptor)
    LocalFree(security_attributes.lpSecurityDescriptor);

  // This is throwaway code. We don't need to sync with the browser process
  // once Google Update is updated to a version supporting OOP crash handling.
  // Create or open an event to signal the browser process that the crash
  // service is initialized.
  HANDLE running_event =
      ::CreateEventW(NULL, TRUE, TRUE, L"g_chrome_crash_svc");
  // If the browser already had the event open, the CreateEvent call did not
  // signal it. We need to do it manually.
  ::SetEvent(running_event);

  return true;
}

void CrashService::OnClientConnected(void* context,
    const google_breakpad::ClientInfo* client_info) {
  ProcessingLock lock;
  VLOG(1) << "client start. pid = " << client_info->pid();
  CrashService* self = static_cast<CrashService*>(context);
  ::InterlockedIncrement(&self->clients_connected_);
}

void CrashService::OnClientExited(void* context,
    const google_breakpad::ClientInfo* client_info) {
  ProcessingLock lock;
  VLOG(1) << "client end. pid = " << client_info->pid();
  CrashService* self = static_cast<CrashService*>(context);
  ::InterlockedIncrement(&self->clients_terminated_);

  if (!self->sender_)
    return;

  // When we are instructed to send reports we need to exit if there are
  // no more clients to service. The next client that runs will start us.
  // Only chrome.exe starts crash_service with a non-zero max_reports.
  if (self->clients_connected_ > self->clients_terminated_)
    return;
  if (self->sender_->max_reports_per_day() > 0) {
    // Wait for the other thread to send crashes, if applicable. The sender
    // thread takes the sending_ lock, so the sleep is just to give it a
    // chance to start.
    ::Sleep(1000);
    base::AutoLock lock(self->sending_);
    // Some people can restart chrome very fast, check again if we have
    // a new client before exiting for real.
    if (self->clients_connected_ == self->clients_terminated_) {
      VLOG(1) << "zero clients. exiting";
      ::PostMessage(g_top_window, WM_CLOSE, 0, 0);
    }
  }
}

void CrashService::OnClientDumpRequest(void* context,
    const google_breakpad::ClientInfo* client_info,
    const std::wstring* file_path) {
  ProcessingLock lock;

  if (!file_path) {
    LOG(ERROR) << "dump with no file path";
    return;
  }
  if (!client_info) {
    LOG(ERROR) << "dump with no client info";
    return;
  }

  DWORD pid = client_info->pid();
  VLOG(1) << "dump for pid = " << pid << " is " << *file_path;

  CrashService* self = static_cast<CrashService*>(context);
  if (!self) {
    LOG(ERROR) << "dump with no context";
    return;
  }

  CrashMap map;
  CustomInfoToMap(client_info, self->reporter_tag_, &map);

  if (!WriteCustomInfoToFile(*file_path, map)) {
    LOG(ERROR) << "could not write custom info file";
  }

  // Move dump file to the directory under client pid.
  base::FilePath dump_path = base::FilePath(*file_path);
  if (archive_dumps_by_pid_) {
    base::FilePath dump_path_with_pid(dump_path.DirName());
    dump_path_with_pid = dump_path_with_pid.Append(base::Int64ToString16(pid));
    file_util::CreateDirectoryW(dump_path_with_pid);
    dump_path_with_pid = dump_path_with_pid.Append(dump_path.BaseName());
    file_util::Move(dump_path, dump_path_with_pid);
  }

  if (!self->sender_)
    return;

  // Send the crash dump using a worker thread. This operation has retry
  // logic in case there is no internet connection at the time.
  DumpJobInfo* dump_job = new DumpJobInfo(pid, self, map,
                                          dump_path.value());
  if (!::QueueUserWorkItem(&CrashService::AsyncSendDump,
                           dump_job, WT_EXECUTELONGFUNCTION)) {
    LOG(ERROR) << "could not queue job";
  }
}

// We are going to try sending the report several times. If we can't send,
// we sleep from one minute to several hours depending on the retry round.
unsigned long CrashService::AsyncSendDump(void* context) {
  if (!context)
    return 0;

  DumpJobInfo* info = static_cast<DumpJobInfo*>(context);

  std::wstring report_id = L"<unsent>";

  const DWORD kOneMinute = 60*1000;
  const DWORD kOneHour = 60*kOneMinute;

  const DWORD kSleepSchedule[] = {
      24*kOneHour,
      8*kOneHour,
      4*kOneHour,
      kOneHour,
      15*kOneMinute,
      0};

  int retry_round = arraysize(kSleepSchedule) - 1;

  do {
    ::Sleep(kSleepSchedule[retry_round]);
    {
      // Take the server lock while sending. This also prevent early
      // termination of the service object.
      base::AutoLock lock(info->self->sending_);
      VLOG(1) << "trying to send report for pid = " << info->pid;
      google_breakpad::ReportResult send_result
          = info->self->sender_->SendCrashReport(kCrashReportURL, info->map,
                                                 info->dump_path, &report_id);
      switch (send_result) {
        case google_breakpad::RESULT_FAILED:
          report_id = L"<network issue>";
          break;
        case google_breakpad::RESULT_REJECTED:
          report_id = L"<rejected>";
          ++info->self->requests_handled_;
          retry_round = 0;
          break;
        case google_breakpad::RESULT_SUCCEEDED:
          ++info->self->requests_sent_;
          ++info->self->requests_handled_;
          retry_round = 0;
          break;
        case google_breakpad::RESULT_THROTTLED:
          report_id = L"<throttled>";
          break;
        default:
          report_id = L"<unknown>";
          break;
      };
    }

    VLOG(1) << "dump for pid =" << info->pid << " crash2 id =" << report_id;
    --retry_round;
  } while (retry_round >= 0);

  if (!::DeleteFileW(info->dump_path.c_str()))
    LOG(WARNING) << "could not delete " << info->dump_path;

  delete info;
  return 0;
}

int CrashService::ProcessingLoop() {
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  VLOG(1) << "session ending..";
  while (ProcessingLock::IsWorking()) {
    ::Sleep(50);
  }

  VLOG(1) << "clients connected :" << clients_connected_
          << "\nclients terminated :" << clients_terminated_
          << "\ndumps serviced :" << requests_handled_
          << "\ndumps reported :" << requests_sent_;

  return static_cast<int>(msg.wParam);
}

PSECURITY_DESCRIPTOR CrashService::GetSecurityDescriptorForLowIntegrity() {
  // Build the SDDL string for the label.
  std::wstring sddl = L"S:(ML;;NW;;;S-1-16-4096)";

  DWORD error = ERROR_SUCCESS;
  PSECURITY_DESCRIPTOR sec_desc = NULL;

  PACL sacl = NULL;
  BOOL sacl_present = FALSE;
  BOOL sacl_defaulted = FALSE;

  if (::ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl.c_str(),
                                                             SDDL_REVISION,
                                                             &sec_desc, NULL)) {
    if (::GetSecurityDescriptorSacl(sec_desc, &sacl_present, &sacl,
                                    &sacl_defaulted)) {
      return sec_desc;
    }
  }

  return NULL;
}
