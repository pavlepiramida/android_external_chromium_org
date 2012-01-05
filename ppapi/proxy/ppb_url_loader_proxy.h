// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_PPB_URL_LOADER_PROXY_H_
#define PPAPI_PPB_URL_LOADER_PROXY_H_

#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_size.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb_url_loader.h"
#include "ppapi/c/trusted/ppb_url_loader_trusted.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/proxy/interface_proxy.h"
#include "ppapi/proxy/proxy_non_thread_safe_ref_count.h"
#include "ppapi/shared_impl/host_resource.h"

namespace ppapi {

struct PPB_URLRequestInfo_Data;

namespace proxy {

struct PPBURLLoader_UpdateProgress_Params;

class PPB_URLLoader_Proxy : public InterfaceProxy {
 public:
  PPB_URLLoader_Proxy(Dispatcher* dispatcher);
  virtual ~PPB_URLLoader_Proxy();

  static const Info* GetTrustedInfo();

  static PP_Resource CreateProxyResource(PP_Instance instance);

  // URLLoader objects are normally allocated by the Create function, but
  // they are also provided to PPP_Instance.OnMsgHandleDocumentLoad. This
  // function allows the proxy for DocumentLoad to create the correct plugin
  // proxied info for the given browser-supplied URLLoader resource ID.
  static PP_Resource TrackPluginResource(
      const HostResource& url_loader_resource);

  // InterfaceProxy implementation.
  virtual bool OnMessageReceived(const IPC::Message& msg);

  // URLLoader objects are sent from places other than just URLLoader.Create,
  // in particular when doing a full-frame plugin. This function does the
  // necessary setup in the host before the resource is sent. Call this any
  // time you're sending a new URLLoader that the plugin hasn't seen yet.
  void PrepareURLLoaderForSendingToPlugin(PP_Resource resource);

  static const ApiID kApiID = API_ID_PPB_URL_LOADER;

 private:
  // Data associated with callbacks for ReadResponseBody.
  struct ReadCallbackInfo;

  // Plugin->renderer message handlers.
  void OnMsgCreate(PP_Instance instance,
                   HostResource* result);
  void OnMsgOpen(const HostResource& loader,
                 const PPB_URLRequestInfo_Data& data);
  void OnMsgFollowRedirect(const HostResource& loader);
  void OnMsgGetResponseInfo(const HostResource& loader,
                            HostResource* result);
  void OnMsgReadResponseBody(const HostResource& loader,
                             int32_t bytes_to_read);
  void OnMsgFinishStreamingToFile(const HostResource& loader);
  void OnMsgClose(const HostResource& loader);
  void OnMsgGrantUniversalAccess(const HostResource& loader);

  // Renderer->plugin message handlers.
  void OnMsgUpdateProgress(
      const PPBURLLoader_UpdateProgress_Params& params);
  void OnMsgReadResponseBodyAck(const HostResource& host_resource,
                                int32_t result,
                                const std::string& data);
  void OnMsgCallbackComplete(const HostResource& host_resource, int32_t result);

  // Handles callbacks for read complete messages. Takes ownership of the info
  // pointer.
  void OnReadCallback(int32_t result, ReadCallbackInfo* info);

  // Handles callback for everything but reads.
  void OnCallback(int32_t result, const HostResource& resource);

  pp::CompletionCallbackFactory<PPB_URLLoader_Proxy,
                                ProxyNonThreadSafeRefCount> callback_factory_;

  // Valid only in the host, this lazily-initialized pointer indicates the
  // URLLoaderTrusted interface.
  const PPB_URLLoaderTrusted* host_urlloader_trusted_interface_;
};

}  // namespace proxy
}  // namespace ppapi

#endif  // PPAPI_PPB_URL_LOADER_PROXY_H_
