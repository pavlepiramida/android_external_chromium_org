// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/image_loader.h"

#include "base/files/file_path.h"
#include "base/json/json_file_value_serializer.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/test_browser_thread.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/notification_types.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_icon_set.h"
#include "extensions/common/extension_paths.h"
#include "extensions/common/extension_resource.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_handlers/icons_handler.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_family.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/size.h"

using content::BrowserThread;
using content::NotificationService;

namespace extensions {

class ImageLoaderTest : public testing::Test {
 public:
  ImageLoaderTest()
      : image_loaded_count_(0),
        quit_in_image_loaded_(false),
        ui_thread_(BrowserThread::UI, &ui_loop_),
        file_thread_(BrowserThread::FILE),
        io_thread_(BrowserThread::IO),
        notification_service_(NotificationService::Create()) {}

  void OnImageLoaded(const gfx::Image& image) {
    image_loaded_count_++;
    if (quit_in_image_loaded_)
      base::MessageLoop::current()->Quit();
    image_ = image;
  }

  void OnImageFamilyLoaded(const gfx::ImageFamily& image_family) {
    image_loaded_count_++;
    if (quit_in_image_loaded_)
      base::MessageLoop::current()->Quit();
    image_family_ = image_family;
  }

  void WaitForImageLoad() {
    quit_in_image_loaded_ = true;
    base::MessageLoop::current()->Run();
    quit_in_image_loaded_ = false;
  }

  int image_loaded_count() {
    int result = image_loaded_count_;
    image_loaded_count_ = 0;
    return result;
  }

  scoped_refptr<Extension> CreateExtension(const char* dir_name,
                                           Manifest::Location location) {
    // Create and load an extension.
    base::FilePath extension_dir;
    if (!PathService::Get(DIR_TEST_DATA, &extension_dir)) {
      EXPECT_FALSE(true);
      return NULL;
    }
    extension_dir = extension_dir.AppendASCII(dir_name);
    int error_code = 0;
    std::string error;
    JSONFileValueSerializer serializer(
        extension_dir.AppendASCII("manifest.json"));
    scoped_ptr<base::DictionaryValue> valid_value(
        static_cast<base::DictionaryValue*>(serializer.Deserialize(&error_code,
                                                                   &error)));
    EXPECT_EQ(0, error_code) << error;
    if (error_code != 0)
      return NULL;

    EXPECT_TRUE(valid_value.get());
    if (!valid_value)
      return NULL;

    return Extension::Create(
        extension_dir, location, *valid_value, Extension::NO_FLAGS, &error);
  }

  gfx::Image image_;
  gfx::ImageFamily image_family_;

 private:
  virtual void SetUp() OVERRIDE {
    testing::Test::SetUp();
    file_thread_.Start();
    io_thread_.Start();
  }

  int image_loaded_count_;
  bool quit_in_image_loaded_;
  base::MessageLoop ui_loop_;
  content::TestBrowserThread ui_thread_;
  content::TestBrowserThread file_thread_;
  content::TestBrowserThread io_thread_;
  scoped_ptr<NotificationService> notification_service_;
};

// Tests loading an image works correctly.
TEST_F(ImageLoaderTest, LoadImage) {
  scoped_refptr<Extension> extension(
      CreateExtension("image_loader", Manifest::INVALID_LOCATION));
  ASSERT_TRUE(extension.get() != NULL);

  ExtensionResource image_resource =
      IconsInfo::GetIconResource(extension.get(),
                                 extension_misc::EXTENSION_ICON_SMALLISH,
                                 ExtensionIconSet::MATCH_EXACTLY);
  gfx::Size max_size(extension_misc::EXTENSION_ICON_SMALLISH,
                     extension_misc::EXTENSION_ICON_SMALLISH);
  ImageLoader loader;
  loader.LoadImageAsync(extension.get(),
                        image_resource,
                        max_size,
                        base::Bind(&ImageLoaderTest::OnImageLoaded,
                                   base::Unretained(this)));

  // The image isn't cached, so we should not have received notification.
  EXPECT_EQ(0, image_loaded_count());

  WaitForImageLoad();

  // We should have gotten the image.
  EXPECT_FALSE(image_.IsEmpty());
  EXPECT_EQ(1, image_loaded_count());

  // Check that the image was loaded.
  EXPECT_EQ(extension_misc::EXTENSION_ICON_SMALLISH,
            image_.ToSkBitmap()->width());
}

// Tests deleting an extension while waiting for the image to load doesn't cause
// problems.
TEST_F(ImageLoaderTest, DeleteExtensionWhileWaitingForCache) {
  scoped_refptr<Extension> extension(
      CreateExtension("image_loader", Manifest::INVALID_LOCATION));
  ASSERT_TRUE(extension.get() != NULL);

  ExtensionResource image_resource =
      IconsInfo::GetIconResource(extension.get(),
                                 extension_misc::EXTENSION_ICON_SMALLISH,
                                 ExtensionIconSet::MATCH_EXACTLY);
  gfx::Size max_size(extension_misc::EXTENSION_ICON_SMALLISH,
                     extension_misc::EXTENSION_ICON_SMALLISH);
  ImageLoader loader;
  std::set<int> sizes;
  sizes.insert(extension_misc::EXTENSION_ICON_SMALLISH);
  loader.LoadImageAsync(extension.get(),
                        image_resource,
                        max_size,
                        base::Bind(&ImageLoaderTest::OnImageLoaded,
                                   base::Unretained(this)));

  // The image isn't cached, so we should not have received notification.
  EXPECT_EQ(0, image_loaded_count());

  // Send out notification the extension was uninstalled.
  UnloadedExtensionInfo details(extension.get(),
                                UnloadedExtensionInfo::REASON_UNINSTALL);
  content::NotificationService::current()->Notify(
      NOTIFICATION_EXTENSION_UNLOADED_DEPRECATED,
      content::NotificationService::AllSources(),
      content::Details<UnloadedExtensionInfo>(&details));

  // Chuck the extension, that way if anyone tries to access it we should crash
  // or get valgrind errors.
  extension = NULL;

  WaitForImageLoad();

  // Even though we deleted the extension, we should still get the image.
  // We should still have gotten the image.
  EXPECT_EQ(1, image_loaded_count());

  // Check that the image was loaded.
  EXPECT_EQ(extension_misc::EXTENSION_ICON_SMALLISH,
            image_.ToSkBitmap()->width());
}

// Tests loading multiple dimensions of the same image.
TEST_F(ImageLoaderTest, MultipleImages) {
  scoped_refptr<Extension> extension(
      CreateExtension("image_loader", Manifest::INVALID_LOCATION));
  ASSERT_TRUE(extension.get() != NULL);

  std::vector<ImageLoader::ImageRepresentation> info_list;
  int sizes[] = {extension_misc::EXTENSION_ICON_BITTY,
                 extension_misc::EXTENSION_ICON_SMALLISH, };
  for (size_t i = 0; i < arraysize(sizes); ++i) {
    ExtensionResource resource = IconsInfo::GetIconResource(
        extension.get(), sizes[i], ExtensionIconSet::MATCH_EXACTLY);
    info_list.push_back(ImageLoader::ImageRepresentation(
        resource,
        ImageLoader::ImageRepresentation::RESIZE_WHEN_LARGER,
        gfx::Size(sizes[i], sizes[i]),
        ui::SCALE_FACTOR_NONE));
  }

  ImageLoader loader;
  loader.LoadImagesAsync(extension.get(), info_list,
                         base::Bind(&ImageLoaderTest::OnImageLoaded,
                                    base::Unretained(this)));

  // The image isn't cached, so we should not have received notification.
  EXPECT_EQ(0, image_loaded_count());

  WaitForImageLoad();

  // We should have gotten the image.
  EXPECT_EQ(1, image_loaded_count());

  // Check that all images were loaded.
  std::vector<gfx::ImageSkiaRep> image_reps =
      image_.ToImageSkia()->image_reps();
  ASSERT_EQ(2u, image_reps.size());

  const gfx::ImageSkiaRep* img_rep1 = &image_reps[0];
  const gfx::ImageSkiaRep* img_rep2 = &image_reps[1];
  EXPECT_EQ(extension_misc::EXTENSION_ICON_BITTY,
            img_rep1->pixel_width());
  EXPECT_EQ(extension_misc::EXTENSION_ICON_SMALLISH,
            img_rep2->pixel_width());
}

// Tests loading multiple dimensions of the same image into an image family.
TEST_F(ImageLoaderTest, LoadImageFamily) {
  scoped_refptr<Extension> extension(
      CreateExtension("image_loader", Manifest::INVALID_LOCATION));
  ASSERT_TRUE(extension.get() != NULL);

  std::vector<ImageLoader::ImageRepresentation> info_list;
  int sizes[] = {extension_misc::EXTENSION_ICON_BITTY,
                 extension_misc::EXTENSION_ICON_SMALLISH, };
  for (size_t i = 0; i < arraysize(sizes); ++i) {
    ExtensionResource resource = IconsInfo::GetIconResource(
        extension.get(), sizes[i], ExtensionIconSet::MATCH_EXACTLY);
    info_list.push_back(ImageLoader::ImageRepresentation(
        resource,
        ImageLoader::ImageRepresentation::NEVER_RESIZE,
        gfx::Size(sizes[i], sizes[i]),
        ui::SCALE_FACTOR_100P));
  }

  // Add a second icon of 200P which should get grouped with the smaller icon's
  // ImageSkia.
  ExtensionResource resource =
      IconsInfo::GetIconResource(extension.get(),
                                 extension_misc::EXTENSION_ICON_SMALLISH,
                                 ExtensionIconSet::MATCH_EXACTLY);
  info_list.push_back(ImageLoader::ImageRepresentation(
      resource,
      ImageLoader::ImageRepresentation::NEVER_RESIZE,
      gfx::Size(extension_misc::EXTENSION_ICON_BITTY,
                extension_misc::EXTENSION_ICON_BITTY),
      ui::SCALE_FACTOR_200P));

  ImageLoader loader;
  loader.LoadImageFamilyAsync(extension.get(),
                              info_list,
                              base::Bind(&ImageLoaderTest::OnImageFamilyLoaded,
                                         base::Unretained(this)));

  // The image isn't cached, so we should not have received notification.
  EXPECT_EQ(0, image_loaded_count());

  WaitForImageLoad();

  // We should have gotten the image.
  EXPECT_EQ(1, image_loaded_count());

  // Check that all images were loaded.
  for (size_t i = 0; i < arraysize(sizes); ++i) {
    const gfx::Image* image = image_family_.GetBest(sizes[i], sizes[i]);
    EXPECT_EQ(sizes[i], image->Width());
  }

  // Check the smaller image has 2 representations of different scale factors.
  std::vector<gfx::ImageSkiaRep> image_reps =
      image_family_.GetBest(extension_misc::EXTENSION_ICON_BITTY,
                            extension_misc::EXTENSION_ICON_BITTY)
          ->ToImageSkia()
          ->image_reps();

  ASSERT_EQ(2u, image_reps.size());

  const gfx::ImageSkiaRep* img_rep1 = &image_reps[0];
  const gfx::ImageSkiaRep* img_rep2 = &image_reps[1];
  EXPECT_EQ(extension_misc::EXTENSION_ICON_BITTY, img_rep1->pixel_width());
  EXPECT_EQ(1.0f, img_rep1->scale());
  EXPECT_EQ(extension_misc::EXTENSION_ICON_SMALLISH, img_rep2->pixel_width());
  EXPECT_EQ(2.0f, img_rep2->scale());
}

}  // namespace extensions
