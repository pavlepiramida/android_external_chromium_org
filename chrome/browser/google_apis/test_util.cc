// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/google_apis/test_util.h"

#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/threading/sequenced_worker_pool.h"
#include "chrome/browser/google_apis/gdata_wapi_parser.h"
#include "chrome/common/chrome_paths.h"
#include "content/public/browser/browser_thread.h"

namespace google_apis {
namespace test_util {

// This class is used to monitor if any task is posted to a message loop.
class TaskObserver : public MessageLoop::TaskObserver {
 public:
  TaskObserver() : posted_(false) {}
  virtual ~TaskObserver() {}

  // MessageLoop::TaskObserver overrides.
  virtual void WillProcessTask(base::TimeTicks time_posted) {}
  virtual void DidProcessTask(base::TimeTicks time_posted) {
    posted_ = true;
  }

  // Returns true if any task was posted.
  bool posted() const { return posted_; }

 private:
  bool posted_;
  DISALLOW_COPY_AND_ASSIGN(TaskObserver);
};

FilePath GetTestFilePath(const std::string& relative_path) {
  FilePath path;
  std::string error;
  PathService::Get(chrome::DIR_TEST_DATA, &path);
  path = path.AppendASCII("chromeos")
      .Append(FilePath::FromUTF8Unsafe(relative_path));
  return path;
}

void RunBlockingPoolTask() {
  while (true) {
    content::BrowserThread::GetBlockingPool()->FlushForTesting();

    TaskObserver task_observer;
    MessageLoop::current()->AddTaskObserver(&task_observer);
    MessageLoop::current()->RunUntilIdle();
    MessageLoop::current()->RemoveTaskObserver(&task_observer);
    if (!task_observer.posted())
      break;
  }
}


scoped_ptr<base::Value> LoadJSONFile(const std::string& relative_path) {
  FilePath path = GetTestFilePath(relative_path);

  std::string error;
  JSONFileValueSerializer serializer(path);
  scoped_ptr<base::Value> value(serializer.Deserialize(NULL, &error));
  LOG_IF(WARNING, !value.get()) << "Failed to parse " << path.value()
                                << ": " << error;
  return value.Pass();
}

void CopyResultsFromGetDataCallback(GDataErrorCode* error_out,
                                    scoped_ptr<base::Value>* value_out,
                                    GDataErrorCode error_in,
                                    scoped_ptr<base::Value> value_in) {
  value_out->swap(value_in);
  *error_out = error_in;
}

void CopyResultsFromGetResourceListCallback(
    GDataErrorCode* error_out,
    scoped_ptr<ResourceList>* resource_list_out,
    GDataErrorCode error_in,
    scoped_ptr<ResourceList> resource_list_in) {
  resource_list_out->swap(resource_list_in);
  *error_out = error_in;
}

}  // namespace test_util
}  // namespace google_apis
