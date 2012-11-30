// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/win/launch_process_with_token.h"

#include <windows.h>
#include <sddl.h>
#include <winternl.h>

#include <limits>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/process_util.h"
#include "base/rand_util.h"
#include "base/scoped_native_library.h"
#include "base/single_thread_task_runner.h"
#include "base/stringprintf.h"
#include "base/utf_string_conversions.h"
#include "base/win/scoped_handle.h"
#include "base/win/scoped_process_information.h"
#include "base/win/windows_version.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_channel.h"

using base::win::ScopedHandle;

namespace {

const char kCreateProcessDefaultPipeNameFormat[] =
    "\\\\.\\Pipe\\TerminalServer\\SystemExecSrvr\\%d";

// Undocumented WINSTATIONINFOCLASS value causing
// winsta!WinStationQueryInformationW() to return the name of the pipe for
// requesting cross-session process creation.
const WINSTATIONINFOCLASS kCreateProcessPipeNameClass =
    static_cast<WINSTATIONINFOCLASS>(0x21);

const int kPipeBusyWaitTimeoutMs = 2000;
const int kPipeConnectMaxAttempts = 3;

// Name of the default session desktop.
const char kDefaultDesktopName[] = "winsta0\\default";

// Terminates the process and closes process and thread handles in
// |process_information| structure.
void CloseHandlesAndTerminateProcess(PROCESS_INFORMATION* process_information) {
  if (process_information->hThread) {
    CloseHandle(process_information->hThread);
    process_information->hThread = NULL;
  }

  if (process_information->hProcess) {
    TerminateProcess(process_information->hProcess, CONTROL_C_EXIT);
    CloseHandle(process_information->hProcess);
    process_information->hProcess = NULL;
  }
}

// Connects to the executor server corresponding to |session_id|.
bool ConnectToExecutionServer(uint32 session_id,
                              base::win::ScopedHandle* pipe_out) {
  string16 pipe_name;

  // Use winsta!WinStationQueryInformationW() to determine the process creation
  // pipe name for the session.
  FilePath winsta_path(base::GetNativeLibraryName(UTF8ToUTF16("winsta")));
  base::ScopedNativeLibrary winsta(winsta_path);
  if (winsta.is_valid()) {
    PWINSTATIONQUERYINFORMATIONW win_station_query_information =
        static_cast<PWINSTATIONQUERYINFORMATIONW>(
            winsta.GetFunctionPointer("WinStationQueryInformationW"));
    if (win_station_query_information) {
      wchar_t name[MAX_PATH];
      ULONG name_length;
      if (win_station_query_information(0,
                                        session_id,
                                        kCreateProcessPipeNameClass,
                                        name,
                                        sizeof(name),
                                        &name_length)) {
        pipe_name.assign(name);
      }
    }
  }

  // Use the default pipe name if we couldn't query its name.
  if (pipe_name.empty()) {
    pipe_name = UTF8ToUTF16(
        StringPrintf(kCreateProcessDefaultPipeNameFormat, session_id));
  }

  // Try to connect to the named pipe.
  base::win::ScopedHandle pipe;
  for (int i = 0; i < kPipeConnectMaxAttempts; ++i) {
    pipe.Set(CreateFile(pipe_name.c_str(),
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL));
    if (pipe.IsValid()) {
      break;
    }

    // Cannot continue retrying if error is something other than
    // ERROR_PIPE_BUSY.
    if (GetLastError() != ERROR_PIPE_BUSY) {
      break;
    }

    // Cannot continue retrying if wait on pipe fails.
    if (!WaitNamedPipe(pipe_name.c_str(), kPipeBusyWaitTimeoutMs)) {
      break;
    }
  }

  if (!pipe.IsValid()) {
    LOG_GETLASTERROR(ERROR) << "Failed to connect to '" << pipe_name << "'";
    return false;
  }

  *pipe_out = pipe.Pass();
  return true;
}

// Copies the process token making it a primary impersonation token.
// The returned handle will have |desired_access| rights.
bool CopyProcessToken(DWORD desired_access, ScopedHandle* token_out) {
  ScopedHandle process_token;
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_DUPLICATE | desired_access,
                        process_token.Receive())) {
    LOG_GETLASTERROR(ERROR) << "Failed to open process token";
    return false;
  }

  ScopedHandle copied_token;
  if (!DuplicateTokenEx(process_token,
                        desired_access,
                        NULL,
                        SecurityImpersonation,
                        TokenPrimary,
                        copied_token.Receive())) {
    LOG_GETLASTERROR(ERROR) << "Failed to duplicate the process token";
    return false;
  }

  *token_out = copied_token.Pass();
  return true;
}

// Creates a copy of the current process with SE_TCB_NAME privilege enabled.
bool CreatePrivilegedToken(ScopedHandle* token_out) {
  ScopedHandle privileged_token;
  DWORD desired_access = TOKEN_ADJUST_PRIVILEGES | TOKEN_IMPERSONATE |
                         TOKEN_DUPLICATE | TOKEN_QUERY;
  if (!CopyProcessToken(desired_access, &privileged_token)) {
    return false;
  }

  // Get the LUID for the SE_TCB_NAME privilege.
  TOKEN_PRIVILEGES state;
  state.PrivilegeCount = 1;
  state.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (!LookupPrivilegeValue(NULL, SE_TCB_NAME, &state.Privileges[0].Luid)) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to lookup the LUID for the SE_TCB_NAME privilege";
    return false;
  }

  // Enable the SE_TCB_NAME privilege.
  if (!AdjustTokenPrivileges(privileged_token, FALSE, &state, 0, NULL, 0)) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to enable SE_TCB_NAME privilege in a token";
    return false;
  }

  *token_out = privileged_token.Pass();
  return true;
}

// Fills the process and thread handles in the passed |process_information|
// structure and resume the process if the caller didn't want to suspend it.
bool ProcessCreateProcessResponse(DWORD creation_flags,
                                  PROCESS_INFORMATION* process_information) {
  // The execution server does not return handles to the created process and
  // thread.
  if (!process_information->hProcess) {
    // N.B. PROCESS_ALL_ACCESS is different in XP and Vista+ versions of
    // the SDK. |desired_access| below is effectively PROCESS_ALL_ACCESS from
    // the XP version of the SDK.
    DWORD desired_access =
        STANDARD_RIGHTS_REQUIRED |
        SYNCHRONIZE |
        PROCESS_TERMINATE |
        PROCESS_CREATE_THREAD |
        PROCESS_SET_SESSIONID |
        PROCESS_VM_OPERATION |
        PROCESS_VM_READ |
        PROCESS_VM_WRITE |
        PROCESS_DUP_HANDLE |
        PROCESS_CREATE_PROCESS |
        PROCESS_SET_QUOTA |
        PROCESS_SET_INFORMATION |
        PROCESS_QUERY_INFORMATION |
        PROCESS_SUSPEND_RESUME;
    process_information->hProcess =
        OpenProcess(desired_access,
                    FALSE,
                    process_information->dwProcessId);
    if (!process_information->hProcess) {
      LOG_GETLASTERROR(ERROR) << "Failed to open the process "
                              << process_information->dwProcessId;
      return false;
    }
  }

  if (!process_information->hThread) {
    // N.B. THREAD_ALL_ACCESS is different in XP and Vista+ versions of
    // the SDK. |desired_access| below is effectively THREAD_ALL_ACCESS from
    // the XP version of the SDK.
    DWORD desired_access =
        STANDARD_RIGHTS_REQUIRED |
        SYNCHRONIZE |
        THREAD_TERMINATE |
        THREAD_SUSPEND_RESUME |
        THREAD_GET_CONTEXT |
        THREAD_SET_CONTEXT |
        THREAD_QUERY_INFORMATION |
        THREAD_SET_INFORMATION |
        THREAD_SET_THREAD_TOKEN |
        THREAD_IMPERSONATE |
        THREAD_DIRECT_IMPERSONATION;
    process_information->hThread =
        OpenThread(desired_access,
                   FALSE,
                   process_information->dwThreadId);
    if (!process_information->hThread) {
      LOG_GETLASTERROR(ERROR) << "Failed to open the thread "
                              << process_information->dwThreadId;
      return false;
    }
  }

  // Resume the thread if the caller didn't want to suspend the process.
  if ((creation_flags & CREATE_SUSPENDED) == 0) {
    if (!ResumeThread(process_information->hThread)) {
      LOG_GETLASTERROR(ERROR) << "Failed to resume the thread "
                              << process_information->dwThreadId;
      return false;
    }
  }

  return true;
}

// Receives the response to a remote process create request.
bool ReceiveCreateProcessResponse(
    HANDLE pipe,
    PROCESS_INFORMATION* process_information_out) {
  struct CreateProcessResponse {
    DWORD size;
    BOOL success;
    DWORD last_error;
    PROCESS_INFORMATION process_information;
  };

  DWORD bytes;
  CreateProcessResponse response;
  if (!ReadFile(pipe, &response, sizeof(response), &bytes, NULL)) {
    LOG_GETLASTERROR(ERROR) << "Failed to receive CreateProcessAsUser response";
    return false;
  }

  // The server sends the data in one chunk so if we didn't received a complete
  // answer something bad happend and there is no point in retrying.
  if (bytes != sizeof(response)) {
    SetLastError(ERROR_RECEIVE_PARTIAL);
    return false;
  }

  if (!response.success) {
    SetLastError(response.last_error);
    return false;
  }

  *process_information_out = response.process_information;
  return true;
}

// Sends a remote process create request to the execution server.
bool SendCreateProcessRequest(
    HANDLE pipe,
    const FilePath::StringType& application_name,
    const CommandLine::StringType& command_line,
    DWORD creation_flags) {
  string16 desktop_name(UTF8ToUTF16(kDefaultDesktopName));

  // |CreateProcessRequest| structure passes the same parameters to
  // the execution server as CreateProcessAsUser() function does. Strings are
  // stored as wide strings immediately after the structure. String pointers are
  // represented as byte offsets to string data from the beginning of
  // the structure.
  struct CreateProcessRequest {
    DWORD size;
    DWORD process_id;
    BOOL use_default_token;
    HANDLE token;
    LPWSTR application_name;
    LPWSTR command_line;
    SECURITY_ATTRIBUTES process_attributes;
    SECURITY_ATTRIBUTES thread_attributes;
    BOOL inherit_handles;
    DWORD creation_flags;
    LPVOID environment;
    LPWSTR current_directory;
    STARTUPINFOW startup_info;
    PROCESS_INFORMATION process_information;
  };

  // Allocate a large enough buffer to hold the CreateProcessRequest structure
  // and three NULL-terminated string parameters.
  size_t size = sizeof(CreateProcessRequest) + sizeof(wchar_t) *
      (application_name.size() + command_line.size() + desktop_name.size() + 3);
  scoped_array<char> buffer(new char[size]);
  memset(buffer.get(), 0, size);

  // Marshal the input parameters.
  CreateProcessRequest* request =
      reinterpret_cast<CreateProcessRequest*>(buffer.get());
  request->size = size;
  request->process_id = GetCurrentProcessId();
  request->use_default_token = TRUE;
  // Always pass CREATE_SUSPENDED to avoid a race between the created process
  // exiting too soon and OpenProcess() call below.
  request->creation_flags = creation_flags | CREATE_SUSPENDED;
  request->startup_info.cb = sizeof(request->startup_info);

  size_t buffer_offset = sizeof(CreateProcessRequest);

  request->application_name = reinterpret_cast<LPWSTR>(buffer_offset);
  std::copy(application_name.begin(),
            application_name.end(),
            reinterpret_cast<wchar_t*>(buffer.get() + buffer_offset));
  buffer_offset += (application_name.size() + 1) * sizeof(wchar_t);

  request->command_line = reinterpret_cast<LPWSTR>(buffer_offset);
  std::copy(command_line.begin(),
            command_line.end(),
            reinterpret_cast<wchar_t*>(buffer.get() + buffer_offset));
  buffer_offset += (command_line.size() + 1) * sizeof(wchar_t);

  request->startup_info.lpDesktop =
      reinterpret_cast<LPWSTR>(buffer_offset);
  std::copy(desktop_name.begin(),
            desktop_name.end(),
            reinterpret_cast<wchar_t*>(buffer.get() + buffer_offset));

  // Pass the request to create a process in the target session.
  DWORD bytes;
  if (!WriteFile(pipe, buffer.get(), size, &bytes, NULL)) {
    LOG_GETLASTERROR(ERROR) << "Failed to send CreateProcessAsUser request";
    return false;
  }

  return true;
}

// Requests the execution server to create a process in the specified session
// using the default (i.e. Winlogon) token. This routine relies on undocumented
// OS functionality and will likely not work on anything but XP or W2K3.
bool CreateRemoteSessionProcess(
    uint32 session_id,
    const FilePath::StringType& application_name,
    const CommandLine::StringType& command_line,
    DWORD creation_flags,
    PROCESS_INFORMATION* process_information_out)
{
  DCHECK_LT(base::win::GetVersion(), base::win::VERSION_VISTA);

  base::win::ScopedHandle pipe;
  if (!ConnectToExecutionServer(session_id, &pipe))
    return false;

  if (!SendCreateProcessRequest(pipe, application_name, command_line,
                                creation_flags)) {
    return false;
  }

  PROCESS_INFORMATION process_information;
  if (!ReceiveCreateProcessResponse(pipe, &process_information))
    return false;

  if (!ProcessCreateProcessResponse(creation_flags, &process_information)) {
    CloseHandlesAndTerminateProcess(&process_information);
    return false;
  }

  *process_information_out = process_information;
  return true;
}

} // namespace

namespace remoting {

// Pipe name prefix used by Chrome IPC channels to convert a channel name into
// a pipe name.
const char kChromePipeNamePrefix[] = "\\\\.\\pipe\\chrome.";

bool CreateConnectedIpcChannel(
    const std::string& channel_name,
    const std::string& pipe_security_descriptor,
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
    IPC::Listener* delegate,
    base::win::ScopedHandle* client_out,
    scoped_ptr<IPC::ChannelProxy>* server_out) {
  // Create the server end of the channel.
  ScopedHandle pipe;
  if (!CreateIpcChannel(channel_name, pipe_security_descriptor, &pipe)) {
    return false;
  }

  // Wrap the pipe into an IPC channel.
  scoped_ptr<IPC::ChannelProxy> server(new IPC::ChannelProxy(
      IPC::ChannelHandle(pipe),
      IPC::Channel::MODE_SERVER,
      delegate,
      io_task_runner));

  // Convert the channel name to the pipe name.
  std::string pipe_name(remoting::kChromePipeNamePrefix);
  pipe_name.append(channel_name);

  SECURITY_ATTRIBUTES security_attributes;
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.lpSecurityDescriptor = NULL;
  security_attributes.bInheritHandle = TRUE;

  // Create the client end of the channel. This code should match the code in
  // IPC::Channel.
  ScopedHandle client;
  client.Set(CreateFile(UTF8ToUTF16(pipe_name).c_str(),
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        &security_attributes,
                        OPEN_EXISTING,
                        SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION |
                            FILE_FLAG_OVERLAPPED,
                        NULL));
  if (!client.IsValid()) {
    LOG_GETLASTERROR(ERROR) << "Failed to connect to '" << pipe_name << "'";
    return false;
  }

  *client_out = client.Pass();
  *server_out = server.Pass();
  return true;
}

bool CreateIpcChannel(
    const std::string& channel_name,
    const std::string& pipe_security_descriptor,
    base::win::ScopedHandle* pipe_out) {
  // Create security descriptor for the channel.
  SECURITY_ATTRIBUTES security_attributes;
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.bInheritHandle = FALSE;

  ULONG security_descriptor_length = 0;
  if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
          UTF8ToUTF16(pipe_security_descriptor).c_str(),
          SDDL_REVISION_1,
          reinterpret_cast<PSECURITY_DESCRIPTOR*>(
              &security_attributes.lpSecurityDescriptor),
          &security_descriptor_length)) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to create a security descriptor for the Chromoting IPC channel";
    return false;
  }

  // Convert the channel name to the pipe name.
  std::string pipe_name(kChromePipeNamePrefix);
  pipe_name.append(channel_name);

  // Create the server end of the pipe. This code should match the code in
  // IPC::Channel with exception of passing a non-default security descriptor.
  base::win::ScopedHandle pipe;
  pipe.Set(CreateNamedPipe(
      UTF8ToUTF16(pipe_name).c_str(),
      PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
      1,
      IPC::Channel::kReadBufferSize,
      IPC::Channel::kReadBufferSize,
      5000,
      &security_attributes));
  if (!pipe.IsValid()) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to create the server end of the Chromoting IPC channel";
    LocalFree(security_attributes.lpSecurityDescriptor);
    return false;
  }

  LocalFree(security_attributes.lpSecurityDescriptor);

  *pipe_out = pipe.Pass();
  return true;
}

// Creates a copy of the current process token for the given |session_id| so
// it can be used to launch a process in that session.
bool CreateSessionToken(uint32 session_id, ScopedHandle* token_out) {
  ScopedHandle session_token;
  DWORD desired_access = TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID |
                         TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY;
  if (!CopyProcessToken(desired_access, &session_token)) {
    return false;
  }

  // Temporarily enable the SE_TCB_NAME privilege as it is required by
  // SetTokenInformation(TokenSessionId).
  ScopedHandle privileged_token;
  if (!CreatePrivilegedToken(&privileged_token)) {
    return false;
  }
  if (!ImpersonateLoggedOnUser(privileged_token)) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to impersonate the privileged token";
    return false;
  }

  // Change the session ID of the token.
  DWORD new_session_id = session_id;
  if (!SetTokenInformation(session_token,
                           TokenSessionId,
                           &new_session_id,
                           sizeof(new_session_id))) {
    LOG_GETLASTERROR(ERROR) << "Failed to change session ID of a token";

    // Revert to the default token.
    CHECK(RevertToSelf());
    return false;
  }

  // Revert to the default token.
  CHECK(RevertToSelf());

  *token_out = session_token.Pass();
  return true;
}

bool LaunchProcessWithToken(const FilePath& binary,
                            const CommandLine::StringType& command_line,
                            HANDLE user_token,
                            bool inherit_handles,
                            DWORD creation_flags,
                            ScopedHandle* process_out,
                            ScopedHandle* thread_out) {
  FilePath::StringType application_name = binary.value();

  base::win::ScopedProcessInformation process_info;
  STARTUPINFOW startup_info;

  string16 desktop_name(UTF8ToUTF16(kDefaultDesktopName));

  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.cb = sizeof(startup_info);
  startup_info.lpDesktop = const_cast<char16*>(desktop_name.c_str());

  BOOL result = CreateProcessAsUser(user_token,
                                    application_name.c_str(),
                                    const_cast<LPWSTR>(command_line.c_str()),
                                    NULL,
                                    NULL,
                                    inherit_handles,
                                    creation_flags,
                                    NULL,
                                    NULL,
                                    &startup_info,
                                    process_info.Receive());

  // CreateProcessAsUser will fail on XP and W2K3 with ERROR_PIPE_NOT_CONNECTED
  // if the user hasn't logged to the target session yet. In such a case
  // we try to talk to the execution server directly emulating what
  // the undocumented and not-exported advapi32!CreateRemoteSessionProcessW()
  // function does. The created process will run under Winlogon'a token instead
  // of |user_token|. Since Winlogon runs as SYSTEM, this suits our needs.
  if (!result &&
      GetLastError() == ERROR_PIPE_NOT_CONNECTED &&
      base::win::GetVersion() < base::win::VERSION_VISTA) {
    DWORD session_id;
    DWORD return_length;
    result = GetTokenInformation(user_token,
                                 TokenSessionId,
                                 &session_id,
                                 sizeof(session_id),
                                 &return_length);
    if (result && session_id != 0) {
      result = CreateRemoteSessionProcess(session_id,
                                          application_name,
                                          command_line,
                                          creation_flags,
                                          process_info.Receive());
    } else {
      // Restore the error status returned by CreateProcessAsUser().
      result = FALSE;
      SetLastError(ERROR_PIPE_NOT_CONNECTED);
    }
  }

  if (!result) {
    LOG_GETLASTERROR(ERROR) <<
        "Failed to launch a process with a user token";
    return false;
  }

  CHECK(process_info.IsValid());
  process_out->Set(process_info.TakeProcessHandle());
  thread_out->Set(process_info.TakeThreadHandle());
  return true;
}

} // namespace remoting
