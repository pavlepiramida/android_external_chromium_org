// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NACL_BROWSER_NACL_HOST_MESSAGE_FILTER_H_
#define COMPONENTS_NACL_BROWSER_NACL_HOST_MESSAGE_FILTER_H_

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/browser_message_filter.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

class GURL;

namespace nacl {
struct NaClLaunchParams;
struct PnaclCacheInfo;
}

namespace net {
class HostResolver;
class URLRequestContextGetter;
}

namespace nacl {

// This class filters out incoming Chrome-specific IPC messages for the renderer
// process on the IPC thread.
class NaClHostMessageFilter : public content::BrowserMessageFilter {
 public:
  NaClHostMessageFilter(int render_process_id,
                        bool is_off_the_record,
                        const base::FilePath& profile_directory,
                        net::URLRequestContextGetter* request_context);

  // content::BrowserMessageFilter methods:
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnChannelClosing() OVERRIDE;

  int render_process_id() { return render_process_id_; }
  bool off_the_record() { return off_the_record_; }
  const base::FilePath& profile_directory() const { return profile_directory_; }
  net::HostResolver* GetHostResolver();

 private:
  friend class content::BrowserThread;
  friend class base::DeleteHelper<NaClHostMessageFilter>;

  virtual ~NaClHostMessageFilter();

  void OnLaunchNaCl(const NaClLaunchParams& launch_params,
                    IPC::Message* reply_msg);
  void LaunchNaClContinuation(const nacl::NaClLaunchParams& launch_params,
                              IPC::Message* reply_msg,
                              ppapi::PpapiPermissions permissions);
  void OnGetReadonlyPnaclFd(const std::string& filename,
                            bool is_executable,
                            IPC::Message* reply_msg);
  void OnNaClCreateTemporaryFile(IPC::Message* reply_msg);
  void OnNaClGetNumProcessors(int* num_processors);
  void OnGetNexeFd(int render_view_id,
                   int pp_instance,
                   const PnaclCacheInfo& cache_info);
  void OnTranslationFinished(int instance, bool success);
  void OnMissingArchError(int render_view_id);
  void OnOpenNaClExecutable(int render_view_id,
                            const GURL& file_url,
                            IPC::Message* reply_msg);
  void SyncReturnTemporaryFile(IPC::Message* reply_msg,
                               base::File file);
  void AsyncReturnTemporaryFile(int pp_instance,
                                const base::File& file,
                                bool is_hit);
  void OnNaClDebugEnabledForURL(const GURL& nmf_url, bool* should_debug);

  int render_process_id_;

  // off_the_record_ is copied from the profile partly so that it can be
  // read on the IO thread.
  bool off_the_record_;
  base::FilePath profile_directory_;
  scoped_refptr<net::URLRequestContextGetter> request_context_;

  base::WeakPtrFactory<NaClHostMessageFilter> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(NaClHostMessageFilter);
};

}  // namespace nacl

#endif  // COMPONENTS_NACL_BROWSER_NACL_HOST_MESSAGE_FILTER_H_
