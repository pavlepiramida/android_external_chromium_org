// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <string>
#include <vector>

#include "base/file_util.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/process_util.h"
#include "base/run_loop.h"
#include "base/shared_memory.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "components/visitedlink/browser/visitedlink_delegate.h"
#include "components/visitedlink/browser/visitedlink_event_listener.h"
#include "components/visitedlink/browser/visitedlink_master.h"
#include "components/visitedlink/common/visitedlink_messages.h"
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/test/mock_render_process_host.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "content/public/test/test_renderer_host.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

using content::BrowserThread;
using content::MockRenderProcessHost;
using content::RenderViewHostTester;

namespace visitedlink {

namespace {

typedef std::vector<GURL> URLs;

// a nice long URL that we can append numbers to to get new URLs
const char g_test_prefix[] =
  "http://www.google.com/products/foo/index.html?id=45028640526508376&seq=";
const int g_test_count = 1000;

// Returns a test URL for index |i|
GURL TestURL(int i) {
  return GURL(base::StringPrintf("%s%d", g_test_prefix, i));
}

std::vector<VisitedLinkSlave*> g_slaves;

class TestVisitedLinkDelegate : public VisitedLinkDelegate {
 public:
  virtual void RebuildTable(
      const scoped_refptr<URLEnumerator>& enumerator) OVERRIDE;

  void AddURLForRebuild(const GURL& url);

 private:

  URLs rebuild_urls_;
};

void TestVisitedLinkDelegate::RebuildTable(
    const scoped_refptr<URLEnumerator>& enumerator) {
  for (URLs::const_iterator itr = rebuild_urls_.begin();
       itr != rebuild_urls_.end();
       ++itr)
    enumerator->OnURL(*itr);
  enumerator->OnComplete(true);
}

void TestVisitedLinkDelegate::AddURLForRebuild(const GURL& url) {
  rebuild_urls_.push_back(url);
}

class TestURLIterator : public VisitedLinkMaster::URLIterator {
 public:
  explicit TestURLIterator(const URLs& urls);

  virtual const GURL& NextURL() OVERRIDE;
  virtual bool HasNextURL() const OVERRIDE;

 private:
  URLs::const_iterator iterator_;
  URLs::const_iterator end_;
};

TestURLIterator::TestURLIterator(const URLs& urls)
    : iterator_(urls.begin()),
      end_(urls.end()) {
}

const GURL& TestURLIterator::NextURL() {
  return *(iterator_++);
}

bool TestURLIterator::HasNextURL() const {
  return iterator_ != end_;
}

}  // namespace

class TrackingVisitedLinkEventListener : public VisitedLinkMaster::Listener {
 public:
  TrackingVisitedLinkEventListener()
      : reset_count_(0),
        add_count_(0) {}

  virtual void NewTable(base::SharedMemory* table) OVERRIDE {
    if (table) {
      for (std::vector<VisitedLinkSlave>::size_type i = 0;
           i < g_slaves.size(); i++) {
        base::SharedMemoryHandle new_handle = base::SharedMemory::NULLHandle();
        table->ShareToProcess(base::GetCurrentProcessHandle(), &new_handle);
        g_slaves[i]->OnUpdateVisitedLinks(new_handle);
      }
    }
  }
  virtual void Add(VisitedLinkCommon::Fingerprint) OVERRIDE { add_count_++; }
  virtual void Reset() OVERRIDE { reset_count_++; }

  void SetUp() {
    reset_count_ = 0;
    add_count_ = 0;
  }

  int reset_count() const { return reset_count_; }
  int add_count() const { return add_count_; }

 private:
  int reset_count_;
  int add_count_;
};

class VisitedLinkTest : public testing::Test {
 protected:
  // Initializes the visited link objects. Pass in the size that you want a
  // freshly created table to be. 0 means use the default.
  //
  // |suppress_rebuild| is set when we're not testing rebuilding, see
  // the VisitedLinkMaster constructor.
  bool InitVisited(int initial_size, bool suppress_rebuild) {
    // Initialize the visited link system.
    master_.reset(new VisitedLinkMaster(new TrackingVisitedLinkEventListener(),
                                        &delegate_,
                                        true,
                                        suppress_rebuild, visited_file_,
                                        initial_size));
    return master_->Init();
  }

  // May be called multiple times (some tests will do this to clear things,
  // and TearDown will do this to make sure eveything is shiny before quitting.
  void ClearDB() {
    if (master_.get())
      master_.reset(NULL);

    // Wait for all pending file I/O to be completed.
    BrowserThread::GetBlockingPool()->FlushForTesting();
  }

  // Loads the database from disk and makes sure that the same URLs are present
  // as were generated by TestIO_Create(). This also checks the URLs with a
  // slave to make sure it reads the data properly.
  void Reload() {
    // Clean up after our caller, who may have left the database open.
    ClearDB();

    ASSERT_TRUE(InitVisited(0, true));
    master_->DebugValidate();

    // check that the table has the proper number of entries
    int used_count = master_->GetUsedCount();
    ASSERT_EQ(used_count, g_test_count);

    // Create a slave database.
    VisitedLinkSlave slave;
    base::SharedMemoryHandle new_handle = base::SharedMemory::NULLHandle();
    master_->shared_memory()->ShareToProcess(
        base::GetCurrentProcessHandle(), &new_handle);
    slave.OnUpdateVisitedLinks(new_handle);
    g_slaves.push_back(&slave);

    bool found;
    for (int i = 0; i < g_test_count; i++) {
      GURL cur = TestURL(i);
      found = master_->IsVisited(cur);
      EXPECT_TRUE(found) << "URL " << i << "not found in master.";

      found = slave.IsVisited(cur);
      EXPECT_TRUE(found) << "URL " << i << "not found in slave.";
    }

    // test some random URL so we know that it returns false sometimes too
    found = master_->IsVisited(GURL("http://unfound.site/"));
    ASSERT_FALSE(found);
    found = slave.IsVisited(GURL("http://unfound.site/"));
    ASSERT_FALSE(found);

    master_->DebugValidate();

    g_slaves.clear();
  }

  // testing::Test
  virtual void SetUp() {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());

    history_dir_ = temp_dir_.path().AppendASCII("VisitedLinkTest");
    ASSERT_TRUE(file_util::CreateDirectory(history_dir_));

    visited_file_ = history_dir_.Append(FILE_PATH_LITERAL("VisitedLinks"));
  }

  virtual void TearDown() {
    ClearDB();
  }

  base::ScopedTempDir temp_dir_;

  // Filenames for the services;
  base::FilePath history_dir_;
  base::FilePath visited_file_;

  scoped_ptr<VisitedLinkMaster> master_;
  TestVisitedLinkDelegate delegate_;
  content::TestBrowserThreadBundle thread_bundle_;
};

// This test creates and reads some databases to make sure the data is
// preserved throughout those operations.
TEST_F(VisitedLinkTest, DatabaseIO) {
  ASSERT_TRUE(InitVisited(0, true));

  for (int i = 0; i < g_test_count; i++)
    master_->AddURL(TestURL(i));

  // Test that the database was written properly
  Reload();
}

// Checks that we can delete things properly when there are collisions.
TEST_F(VisitedLinkTest, Delete) {
  static const int32 kInitialSize = 17;
  ASSERT_TRUE(InitVisited(kInitialSize, true));

  // Add a cluster from 14-17 wrapping around to 0. These will all hash to the
  // same value.
  const VisitedLinkCommon::Fingerprint kFingerprint0 = kInitialSize * 0 + 14;
  const VisitedLinkCommon::Fingerprint kFingerprint1 = kInitialSize * 1 + 14;
  const VisitedLinkCommon::Fingerprint kFingerprint2 = kInitialSize * 2 + 14;
  const VisitedLinkCommon::Fingerprint kFingerprint3 = kInitialSize * 3 + 14;
  const VisitedLinkCommon::Fingerprint kFingerprint4 = kInitialSize * 4 + 14;
  master_->AddFingerprint(kFingerprint0, false);  // @14
  master_->AddFingerprint(kFingerprint1, false);  // @15
  master_->AddFingerprint(kFingerprint2, false);  // @16
  master_->AddFingerprint(kFingerprint3, false);  // @0
  master_->AddFingerprint(kFingerprint4, false);  // @1

  // Deleting 14 should move the next value up one slot (we do not specify an
  // order).
  EXPECT_EQ(kFingerprint3, master_->hash_table_[0]);
  master_->DeleteFingerprint(kFingerprint3, false);
  VisitedLinkCommon::Fingerprint zero_fingerprint = 0;
  EXPECT_EQ(zero_fingerprint, master_->hash_table_[1]);
  EXPECT_NE(zero_fingerprint, master_->hash_table_[0]);

  // Deleting the other four should leave the table empty.
  master_->DeleteFingerprint(kFingerprint0, false);
  master_->DeleteFingerprint(kFingerprint1, false);
  master_->DeleteFingerprint(kFingerprint2, false);
  master_->DeleteFingerprint(kFingerprint4, false);

  EXPECT_EQ(0, master_->used_items_);
  for (int i = 0; i < kInitialSize; i++)
    EXPECT_EQ(zero_fingerprint, master_->hash_table_[i]) <<
        "Hash table has values in it.";
}

// When we delete more than kBigDeleteThreshold we trigger different behavior
// where the entire file is rewritten.
TEST_F(VisitedLinkTest, BigDelete) {
  ASSERT_TRUE(InitVisited(16381, true));

  // Add the base set of URLs that won't be deleted.
  // Reload() will test for these.
  for (int32 i = 0; i < g_test_count; i++)
    master_->AddURL(TestURL(i));

  // Add more URLs than necessary to trigger this case.
  const int kTestDeleteCount = VisitedLinkMaster::kBigDeleteThreshold + 2;
  URLs urls_to_delete;
  for (int32 i = g_test_count; i < g_test_count + kTestDeleteCount; i++) {
    GURL url(TestURL(i));
    master_->AddURL(url);
    urls_to_delete.push_back(url);
  }

  TestURLIterator iterator(urls_to_delete);
  master_->DeleteURLs(&iterator);
  master_->DebugValidate();

  Reload();
}

TEST_F(VisitedLinkTest, DeleteAll) {
  ASSERT_TRUE(InitVisited(0, true));

  {
    VisitedLinkSlave slave;
    base::SharedMemoryHandle new_handle = base::SharedMemory::NULLHandle();
    master_->shared_memory()->ShareToProcess(
        base::GetCurrentProcessHandle(), &new_handle);
    slave.OnUpdateVisitedLinks(new_handle);
    g_slaves.push_back(&slave);

    // Add the test URLs.
    for (int i = 0; i < g_test_count; i++) {
      master_->AddURL(TestURL(i));
      ASSERT_EQ(i + 1, master_->GetUsedCount());
    }
    master_->DebugValidate();

    // Make sure the slave picked up the adds.
    for (int i = 0; i < g_test_count; i++)
      EXPECT_TRUE(slave.IsVisited(TestURL(i)));

    // Clear the table and make sure the slave picked it up.
    master_->DeleteAllURLs();
    EXPECT_EQ(0, master_->GetUsedCount());
    for (int i = 0; i < g_test_count; i++) {
      EXPECT_FALSE(master_->IsVisited(TestURL(i)));
      EXPECT_FALSE(slave.IsVisited(TestURL(i)));
    }

    // Close the database.
    g_slaves.clear();
    ClearDB();
  }

  // Reopen and validate.
  ASSERT_TRUE(InitVisited(0, true));
  master_->DebugValidate();
  EXPECT_EQ(0, master_->GetUsedCount());
  for (int i = 0; i < g_test_count; i++)
    EXPECT_FALSE(master_->IsVisited(TestURL(i)));
}

// This tests that the master correctly resizes its tables when it gets too
// full, notifies its slaves of the change, and updates the disk.
TEST_F(VisitedLinkTest, Resizing) {
  // Create a very small database.
  const int32 initial_size = 17;
  ASSERT_TRUE(InitVisited(initial_size, true));

  // ...and a slave
  VisitedLinkSlave slave;
  base::SharedMemoryHandle new_handle = base::SharedMemory::NULLHandle();
  master_->shared_memory()->ShareToProcess(
      base::GetCurrentProcessHandle(), &new_handle);
  slave.OnUpdateVisitedLinks(new_handle);
  g_slaves.push_back(&slave);

  int32 used_count = master_->GetUsedCount();
  ASSERT_EQ(used_count, 0);

  for (int i = 0; i < g_test_count; i++) {
    master_->AddURL(TestURL(i));
    used_count = master_->GetUsedCount();
    ASSERT_EQ(i + 1, used_count);
  }

  // Verify that the table got resized sufficiently.
  int32 table_size;
  VisitedLinkCommon::Fingerprint* table;
  master_->GetUsageStatistics(&table_size, &table);
  used_count = master_->GetUsedCount();
  ASSERT_GT(table_size, used_count);
  ASSERT_EQ(used_count, g_test_count) <<
                "table count doesn't match the # of things we added";

  // Verify that the slave got the resize message and has the same
  // table information.
  int32 child_table_size;
  VisitedLinkCommon::Fingerprint* child_table;
  slave.GetUsageStatistics(&child_table_size, &child_table);
  ASSERT_EQ(table_size, child_table_size);
  for (int32 i = 0; i < table_size; i++) {
    ASSERT_EQ(table[i], child_table[i]);
  }

  master_->DebugValidate();
  g_slaves.clear();

  // This tests that the file is written correctly by reading it in using
  // a new database.
  Reload();
}

// Tests that if the database doesn't exist, it will be rebuilt from history.
TEST_F(VisitedLinkTest, Rebuild) {
  // Add half of our URLs to history. This needs to be done before we
  // initialize the visited link DB.
  int history_count = g_test_count / 2;
  for (int i = 0; i < history_count; i++)
    delegate_.AddURLForRebuild(TestURL(i));

  // Initialize the visited link DB. Since the visited links file doesn't exist
  // and we don't suppress history rebuilding, this will load from history.
  ASSERT_TRUE(InitVisited(0, false));

  // While the table is rebuilding, add the rest of the URLs to the visited
  // link system. This isn't guaranteed to happen during the rebuild, so we
  // can't be 100% sure we're testing the right thing, but in practice is.
  // All the adds above will generally take some time queuing up on the
  // history thread, and it will take a while to catch up to actually
  // processing the rebuild that has queued behind it. We will generally
  // finish adding all of the URLs before it has even found the first URL.
  for (int i = history_count; i < g_test_count; i++)
    master_->AddURL(TestURL(i));

  // Add one more and then delete it.
  master_->AddURL(TestURL(g_test_count));
  URLs urls_to_delete;
  urls_to_delete.push_back(TestURL(g_test_count));
  TestURLIterator iterator(urls_to_delete);
  master_->DeleteURLs(&iterator);

  // Wait for the rebuild to complete. The task will terminate the message
  // loop when the rebuild is done. There's no chance that the rebuild will
  // complete before we set the task because the rebuild completion message
  // is posted to the message loop; until we Run() it, rebuild can not
  // complete.
  base::RunLoop run_loop;
  master_->set_rebuild_complete_task(run_loop.QuitClosure());
  run_loop.Run();

  // Test that all URLs were written to the database properly.
  Reload();

  // Make sure the extra one was *not* written (Reload won't test this).
  EXPECT_FALSE(master_->IsVisited(TestURL(g_test_count)));
}

// Test that importing a large number of URLs will work
TEST_F(VisitedLinkTest, BigImport) {
  ASSERT_TRUE(InitVisited(0, false));

  // Before the table rebuilds, add a large number of URLs
  int total_count = VisitedLinkMaster::kDefaultTableSize + 10;
  for (int i = 0; i < total_count; i++)
    master_->AddURL(TestURL(i));

  // Wait for the rebuild to complete.
  base::RunLoop run_loop;
  master_->set_rebuild_complete_task(run_loop.QuitClosure());
  run_loop.Run();

  // Ensure that the right number of URLs are present
  int used_count = master_->GetUsedCount();
  ASSERT_EQ(used_count, total_count);
}

TEST_F(VisitedLinkTest, Listener) {
  ASSERT_TRUE(InitVisited(0, true));

  // Add test URLs.
  for (int i = 0; i < g_test_count; i++) {
    master_->AddURL(TestURL(i));
    ASSERT_EQ(i + 1, master_->GetUsedCount());
  }

  // Delete an URL.
  URLs urls_to_delete;
  urls_to_delete.push_back(TestURL(0));
  TestURLIterator iterator(urls_to_delete);
  master_->DeleteURLs(&iterator);

  // ... and all of the remaining ones.
  master_->DeleteAllURLs();

  TrackingVisitedLinkEventListener* listener =
      static_cast<TrackingVisitedLinkEventListener*>(master_->GetListener());

  // Verify that VisitedLinkMaster::Listener::Add was called for each added URL.
  EXPECT_EQ(g_test_count, listener->add_count());
  // Verify that VisitedLinkMaster::Listener::Reset was called both when one and
  // all URLs are deleted.
  EXPECT_EQ(2, listener->reset_count());
}

class VisitCountingContext : public content::TestBrowserContext {
 public:
  VisitCountingContext()
      : add_count_(0),
        add_event_count_(0),
        reset_event_count_(0),
        new_table_count_(0) {}

  void CountAddEvent(int by) {
    add_count_ += by;
    add_event_count_++;
  }

  void CountResetEvent() {
    reset_event_count_++;
  }

  void CountNewTable() {
    new_table_count_++;
  }

  int add_count() const { return add_count_; }
  int add_event_count() const { return add_event_count_; }
  int reset_event_count() const { return reset_event_count_; }
  int new_table_count() const { return new_table_count_; }

 private:
  int add_count_;
  int add_event_count_;
  int reset_event_count_;
  int new_table_count_;
};

// Stub out as little as possible, borrowing from RenderProcessHost.
class VisitRelayingRenderProcessHost : public MockRenderProcessHost {
 public:
  explicit VisitRelayingRenderProcessHost(
      content::BrowserContext* browser_context)
          : MockRenderProcessHost(browser_context), widgets_(0) {
    content::NotificationService::current()->Notify(
        content::NOTIFICATION_RENDERER_PROCESS_CREATED,
        content::Source<RenderProcessHost>(this),
        content::NotificationService::NoDetails());
  }
  virtual ~VisitRelayingRenderProcessHost() {
    content::NotificationService::current()->Notify(
        content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
        content::Source<content::RenderProcessHost>(this),
        content::NotificationService::NoDetails());
  }

  virtual void WidgetRestored() OVERRIDE { widgets_++; }
  virtual void WidgetHidden() OVERRIDE { widgets_--; }
  virtual int VisibleWidgetCount() const OVERRIDE { return widgets_; }

  virtual bool Send(IPC::Message* msg) OVERRIDE {
    VisitCountingContext* counting_context =
        static_cast<VisitCountingContext*>(
            GetBrowserContext());

    if (msg->type() == ChromeViewMsg_VisitedLink_Add::ID) {
      PickleIterator iter(*msg);
      std::vector<uint64> fingerprints;
      CHECK(IPC::ReadParam(msg, &iter, &fingerprints));
      counting_context->CountAddEvent(fingerprints.size());
    } else if (msg->type() == ChromeViewMsg_VisitedLink_Reset::ID) {
      counting_context->CountResetEvent();
    } else if (msg->type() == ChromeViewMsg_VisitedLink_NewTable::ID) {
      counting_context->CountNewTable();
    }

    delete msg;
    return true;
  }

 private:
  int widgets_;

  DISALLOW_COPY_AND_ASSIGN(VisitRelayingRenderProcessHost);
};

class VisitedLinkRenderProcessHostFactory
    : public content::RenderProcessHostFactory {
 public:
  VisitedLinkRenderProcessHostFactory()
      : content::RenderProcessHostFactory() {}
  virtual content::RenderProcessHost* CreateRenderProcessHost(
      content::BrowserContext* browser_context,
      content::SiteInstance* site_instance) const OVERRIDE {
    return new VisitRelayingRenderProcessHost(browser_context);
  }

 private:

  DISALLOW_COPY_AND_ASSIGN(VisitedLinkRenderProcessHostFactory);
};

class VisitedLinkEventsTest : public content::RenderViewHostTestHarness {
 public:
  virtual void SetUp() {
    browser_context_.reset(new VisitCountingContext());
    master_.reset(new VisitedLinkMaster(context(), &delegate_, true));
    master_->Init();
    SetRenderProcessHostFactory(&vc_rph_factory_);
    content::RenderViewHostTestHarness::SetUp();
  }

  VisitCountingContext* context() const {
    return static_cast<VisitCountingContext*>(browser_context_.get());
  }

  VisitedLinkMaster* master() const {
    return master_.get();
  }

  void WaitForCoalescense() {
    // Let the timer fire.
    //
    // TODO(ajwong): This is horrid! What is the right synchronization method?
    base::RunLoop run_loop;
    base::MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        run_loop.QuitClosure(),
        base::TimeDelta::FromMilliseconds(110));
    run_loop.Run();
  }

 protected:
  VisitedLinkRenderProcessHostFactory vc_rph_factory_;

 private:
  TestVisitedLinkDelegate delegate_;
  scoped_ptr<VisitedLinkMaster> master_;
};

TEST_F(VisitedLinkEventsTest, Coalescense) {
  // add some URLs to master.
  // Add a few URLs.
  master()->AddURL(GURL("http://acidtests.org/"));
  master()->AddURL(GURL("http://google.com/"));
  master()->AddURL(GURL("http://chromium.org/"));
  // Just for kicks, add a duplicate URL. This shouldn't increase the resulting
  master()->AddURL(GURL("http://acidtests.org/"));

  // Make sure that coalescing actually occurs. There should be no links or
  // events received by the renderer.
  EXPECT_EQ(0, context()->add_count());
  EXPECT_EQ(0, context()->add_event_count());

  WaitForCoalescense();

  // We now should have 3 entries added in 1 event.
  EXPECT_EQ(3, context()->add_count());
  EXPECT_EQ(1, context()->add_event_count());

  // Test whether the coalescing continues by adding a few more URLs.
  master()->AddURL(GURL("http://google.com/chrome/"));
  master()->AddURL(GURL("http://webkit.org/"));
  master()->AddURL(GURL("http://acid3.acidtests.org/"));

  WaitForCoalescense();

  // We should have 6 entries added in 2 events.
  EXPECT_EQ(6, context()->add_count());
  EXPECT_EQ(2, context()->add_event_count());

  // Test whether duplicate entries produce add events.
  master()->AddURL(GURL("http://acidtests.org/"));

  WaitForCoalescense();

  // We should have no change in results.
  EXPECT_EQ(6, context()->add_count());
  EXPECT_EQ(2, context()->add_event_count());

  // Ensure that the coalescing does not resume after resetting.
  master()->AddURL(GURL("http://build.chromium.org/"));
  master()->DeleteAllURLs();

  WaitForCoalescense();

  // We should have no change in results except for one new reset event.
  EXPECT_EQ(6, context()->add_count());
  EXPECT_EQ(2, context()->add_event_count());
  EXPECT_EQ(1, context()->reset_event_count());
}

TEST_F(VisitedLinkEventsTest, Basics) {
  RenderViewHostTester::For(rvh())->CreateRenderView(base::string16(),
                                 MSG_ROUTING_NONE,
                                 -1);

  // Add a few URLs.
  master()->AddURL(GURL("http://acidtests.org/"));
  master()->AddURL(GURL("http://google.com/"));
  master()->AddURL(GURL("http://chromium.org/"));

  WaitForCoalescense();

  // We now should have 1 add event.
  EXPECT_EQ(1, context()->add_event_count());
  EXPECT_EQ(0, context()->reset_event_count());

  master()->DeleteAllURLs();

  WaitForCoalescense();

  // We should have no change in add results, plus one new reset event.
  EXPECT_EQ(1, context()->add_event_count());
  EXPECT_EQ(1, context()->reset_event_count());
}

TEST_F(VisitedLinkEventsTest, TabVisibility) {
  RenderViewHostTester::For(rvh())->CreateRenderView(base::string16(),
                                 MSG_ROUTING_NONE,
                                 -1);

  // Simulate tab becoming inactive.
  RenderViewHostTester::For(rvh())->SimulateWasHidden();

  // Add a few URLs.
  master()->AddURL(GURL("http://acidtests.org/"));
  master()->AddURL(GURL("http://google.com/"));
  master()->AddURL(GURL("http://chromium.org/"));

  WaitForCoalescense();

  // We shouldn't have any events.
  EXPECT_EQ(0, context()->add_event_count());
  EXPECT_EQ(0, context()->reset_event_count());

  // Simulate the tab becoming active.
  RenderViewHostTester::For(rvh())->SimulateWasShown();

  // We should now have 3 add events, still no reset events.
  EXPECT_EQ(1, context()->add_event_count());
  EXPECT_EQ(0, context()->reset_event_count());

  // Deactivate the tab again.
  RenderViewHostTester::For(rvh())->SimulateWasHidden();

  // Add a bunch of URLs (over 50) to exhaust the link event buffer.
  for (int i = 0; i < 100; i++)
    master()->AddURL(TestURL(i));

  WaitForCoalescense();

  // Again, no change in events until tab is active.
  EXPECT_EQ(1, context()->add_event_count());
  EXPECT_EQ(0, context()->reset_event_count());

  // Activate the tab.
  RenderViewHostTester::For(rvh())->SimulateWasShown();

  // We should have only one more reset event.
  EXPECT_EQ(1, context()->add_event_count());
  EXPECT_EQ(1, context()->reset_event_count());
}

// Tests that VisitedLink ignores renderer process creation notification for a
// different context.
TEST_F(VisitedLinkEventsTest, IgnoreRendererCreationFromDifferentContext) {
  VisitCountingContext different_context;
  VisitRelayingRenderProcessHost different_process_host(&different_context);

  content::NotificationService::current()->Notify(
      content::NOTIFICATION_RENDERER_PROCESS_CREATED,
      content::Source<content::RenderProcessHost>(&different_process_host),
      content::NotificationService::NoDetails());
  WaitForCoalescense();

  EXPECT_EQ(0, different_context.new_table_count());

}

}  // namespace visitedlink
