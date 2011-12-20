// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/profiler_ui.h"

#include <string>

// When testing the javacript code, it is cumbersome to have to keep
// re-building the resouces package and reloading the browser. To solve
// this, enable the following flag to read the webapp's source files
// directly off disk, so all you have to do is refresh the page to
// test the modifications.
//#define USE_SOURCE_FILES_DIRECTLY

#include "base/bind.h"
#include "base/tracked_objects.h"
#include "chrome/browser/metrics/tracking_synchronizer.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/chrome_web_ui_data_source.h"
#include "chrome/common/url_constants.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/browser/trace_controller.h"
#include "content/public/browser/browser_thread.h"
#include "grit/browser_resources.h"
#include "grit/generated_resources.h"

#ifdef USE_SOURCE_FILES_DIRECTLY
#include "base/base_paths.h"
#include "base/file_util.h"
#include "base/memory/ref_counted_memory.h"
#include "base/path_service.h"
#endif  //  USE_SOURCE_FILES_DIRECTLY

using content::BrowserThread;
using chrome_browser_metrics::TrackingSynchronizer;

namespace {

#ifdef USE_SOURCE_FILES_DIRECTLY

class ProfilerWebUIDataSource : public ChromeURLDataManager::DataSource {
 public:
  ProfilerWebUIDataSource()
      : DataSource(chrome::kChromeUIProfilerHost, MessageLoop::current()) {
  }

 protected:
  // ChromeURLDataManager
  virtual std::string GetMimeType(const std::string& path) const OVERRIDE {
    if (EndsWith(path, ".js", false))
      return "application/javascript";
    return "text/html";
  }

  virtual void StartDataRequest(const std::string& path,
                                bool is_incognito,
                                int request_id) OVERRIDE {
    FilePath base_path;
    PathService::Get(base::DIR_SOURCE_ROOT, &base_path);
    base_path = base_path.AppendASCII("chrome");
    base_path = base_path.AppendASCII("browser");
    base_path = base_path.AppendASCII("resources");
    base_path = base_path.AppendASCII("profiler");

    // If no resource was specified, default to profiler.html.
    std::string filename = path.empty() ? "profiler.html" : path;

    FilePath file_path;
    file_path = base_path.AppendASCII(filename);

    // Read the file synchronously and send it as the response.
    base::ThreadRestrictions::ScopedAllowIO allow;
    std::string file_contents;
    if (!file_util::ReadFileToString(file_path, &file_contents))
      LOG(ERROR) << "Couldn't read file: " << file_path.value();
    scoped_refptr<base::RefCountedString> response =
        new base::RefCountedString();
    response->data() = file_contents;
    SendResponse(request_id, response);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ProfilerWebUIDataSource);
};

ChromeURLDataManager::DataSource* CreateProfilerHTMLSource() {
  return new ProfilerWebUIDataSource();
}

#else  // USE_SOURCE_FILES_DIRECTLY

ChromeWebUIDataSource* CreateProfilerHTMLSource() {
  ChromeWebUIDataSource* source =
      new ChromeWebUIDataSource(chrome::kChromeUIProfilerHost);

  source->set_json_path("strings.js");
  source->add_resource_path("profiler.js", IDR_PROFILER_JS);
  source->set_default_resource(IDR_PROFILER_HTML);
  return source;
}

#endif

// This class receives javascript messages from the renderer.
// Note that the WebUI infrastructure runs on the UI thread, therefore all of
// this class's methods are expected to run on the UI thread.
class ProfilerMessageHandler : public WebUIMessageHandler {
 public:
  ProfilerMessageHandler() {}

  // WebUIMessageHandler implementation.
  virtual WebUIMessageHandler* Attach(WebUI* web_ui) OVERRIDE;
  virtual void RegisterMessages() OVERRIDE;

  // Messages.
  void OnGetData(const ListValue* list);
  void OnResetData(const ListValue* list);

 private:
  DISALLOW_COPY_AND_ASSIGN(ProfilerMessageHandler);
};

WebUIMessageHandler* ProfilerMessageHandler::Attach(WebUI* web_ui) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  WebUIMessageHandler* result = WebUIMessageHandler::Attach(web_ui);
  return result;
}

void ProfilerMessageHandler::RegisterMessages() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  web_ui_->RegisterMessageCallback("getData",
      base::Bind(&ProfilerMessageHandler::OnGetData,base::Unretained(this)));
  web_ui_->RegisterMessageCallback("resetData",
      base::Bind(&ProfilerMessageHandler::OnResetData,
                 base::Unretained(this)));
}

void ProfilerMessageHandler::OnGetData(const ListValue* list) {
  ProfilerUI* profiler_ui = reinterpret_cast<ProfilerUI*>(web_ui_);
  profiler_ui->GetData();
}

void ProfilerMessageHandler::OnResetData(const ListValue* list) {
  tracked_objects::ThreadData::ResetAllThreadData();
}

}  // namespace

ProfilerUI::ProfilerUI(TabContents* contents) : ChromeWebUI(contents) {
  ui_weak_ptr_factory_.reset(new base::WeakPtrFactory<ProfilerUI>(this));
  ui_weak_ptr_ = ui_weak_ptr_factory_->GetWeakPtr();

  AddMessageHandler((new ProfilerMessageHandler())->Attach(this));

  // Set up the chrome://profiler/ source.
  Profile::FromBrowserContext(contents->browser_context())->
      GetChromeURLDataManager()->AddDataSource(CreateProfilerHTMLSource());
}

ProfilerUI::~ProfilerUI() {
}

void ProfilerUI::GetData() {
  TrackingSynchronizer::FetchProfilerDataAsynchronously(ui_weak_ptr_);
}

void ProfilerUI::ReceivedData(base::Value* value) {
  // Send the data to the renderer.
  scoped_ptr<Value> data_values(value);
  CallJavascriptFunction("g_browserBridge.receivedData", *data_values.get());
}

