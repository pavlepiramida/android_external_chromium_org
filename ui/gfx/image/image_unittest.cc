// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_png_rep.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_unittest_util.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#include "ui/gfx/gtk_util.h"
#elif defined(OS_IOS)
#include "base/mac/foundation_util.h"
#include "skia/ext/skia_utils_ios.h"
#elif defined(OS_MACOSX)
#include "base/mac/mac_util.h"
#include "skia/ext/skia_utils_mac.h"
#endif

namespace {

#if defined(TOOLKIT_VIEWS) || defined(OS_ANDROID)
const bool kUsesSkiaNatively = true;
#else
const bool kUsesSkiaNatively = false;
#endif

class ImageTest : public testing::Test {
};

namespace gt = gfx::test;

TEST_F(ImageTest, EmptyImage) {
  // Test the default constructor.
  gfx::Image image;
  EXPECT_EQ(0U, image.RepresentationCount());
  EXPECT_TRUE(image.IsEmpty());

  // Test the copy constructor.
  gfx::Image imageCopy(image);
  EXPECT_TRUE(imageCopy.IsEmpty());

  // Test calling SwapRepresentations() with an empty image.
  gfx::Image image2(gt::CreateBitmap(25, 25));
  EXPECT_FALSE(image2.IsEmpty());

  image.SwapRepresentations(&image2);
  EXPECT_FALSE(image.IsEmpty());
  EXPECT_TRUE(image2.IsEmpty());
}

// Test constructing a gfx::Image from an empty PlatformImage.
TEST_F(ImageTest, EmptyImageFromEmptyPlatformImage) {
#if defined(OS_IOS) || defined(OS_MACOSX) || defined(TOOLKIT_GTK)
  gfx::Image image1(NULL);
  EXPECT_TRUE(image1.IsEmpty());
  EXPECT_EQ(0U, image1.RepresentationCount());
#endif

  // SkBitmap, gfx::ImageSkia, and ImagePNGRep are available on all platforms.
  SkBitmap bitmap;
  EXPECT_TRUE(bitmap.empty());
  gfx::Image image2(bitmap);
  EXPECT_TRUE(image2.IsEmpty());
  EXPECT_EQ(0U, image2.RepresentationCount());

  gfx::ImageSkia image_skia;
  EXPECT_TRUE(image_skia.isNull());
  gfx::Image image3(image_skia);
  EXPECT_TRUE(image3.IsEmpty());
  EXPECT_EQ(0U, image3.RepresentationCount());

  std::vector<gfx::ImagePNGRep> image_png_reps;
  gfx::Image image4(image_png_reps);
  EXPECT_TRUE(image4.IsEmpty());
  EXPECT_EQ(0U, image4.RepresentationCount());
}

// The resulting Image should be empty when it is created using obviously
// invalid data.
TEST_F(ImageTest, EmptyImageFromObviouslyInvalidPNGImage) {
  std::vector<gfx::ImagePNGRep> image_png_reps1;
  image_png_reps1.push_back(gfx::ImagePNGRep(NULL, ui::SCALE_FACTOR_100P));
  gfx::Image image1(image_png_reps1);
  EXPECT_TRUE(image1.IsEmpty());
  EXPECT_EQ(0U, image1.RepresentationCount());

  std::vector<gfx::ImagePNGRep> image_png_reps2;
  image_png_reps2.push_back(gfx::ImagePNGRep(
      new base::RefCountedBytes(), ui::SCALE_FACTOR_100P));
  gfx::Image image2(image_png_reps2);
  EXPECT_TRUE(image2.IsEmpty());
  EXPECT_EQ(0U, image2.RepresentationCount());
}

TEST_F(ImageTest, SkiaToSkia) {
  gfx::Image image(gt::CreateBitmap(25, 25));
  const SkBitmap* bitmap = image.ToSkBitmap();
  EXPECT_TRUE(bitmap);
  EXPECT_FALSE(bitmap->isNull());
  EXPECT_EQ(1U, image.RepresentationCount());

  // Make sure double conversion doesn't happen.
  bitmap = image.ToSkBitmap();
  EXPECT_TRUE(bitmap);
  EXPECT_FALSE(bitmap->isNull());
  EXPECT_EQ(1U, image.RepresentationCount());

  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepSkia));
  if (!kUsesSkiaNatively)
    EXPECT_FALSE(image.HasRepresentation(gt::GetPlatformRepresentationType()));
}

TEST_F(ImageTest, SkiaRefToSkia) {
  gfx::Image image(gt::CreateBitmap(25, 25));
  const SkBitmap* bitmap = image.ToSkBitmap();
  EXPECT_TRUE(bitmap);
  EXPECT_FALSE(bitmap->isNull());
  EXPECT_EQ(1U, image.RepresentationCount());

  EXPECT_EQ(bitmap, image.ToSkBitmap());
  if (!kUsesSkiaNatively)
    EXPECT_FALSE(image.HasRepresentation(gt::GetPlatformRepresentationType()));
}

TEST_F(ImageTest, SkiaToSkiaRef) {
  gfx::Image image(gt::CreateBitmap(25, 25));

  const SkBitmap* bitmap = image.ToSkBitmap();
  EXPECT_FALSE(bitmap->isNull());
  EXPECT_EQ(1U, image.RepresentationCount());

  const SkBitmap* bitmap1 = image.ToSkBitmap();
  EXPECT_FALSE(bitmap1->isNull());
  EXPECT_EQ(1U, image.RepresentationCount());

  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepSkia));
  if (!kUsesSkiaNatively)
    EXPECT_FALSE(image.HasRepresentation(gt::GetPlatformRepresentationType()));
}

TEST_F(ImageTest, EmptyImageToPNG) {
  gfx::Image image;
  scoped_refptr<base::RefCountedBytes> png_bytes = image.As1xPNGBytes();
  EXPECT_TRUE(png_bytes.get());
  EXPECT_FALSE(png_bytes->size());
}

// Check that getting the 1x PNG bytes from images which do not have a 1x
// representation returns NULL.
TEST_F(ImageTest, ImageNo1xToPNG) {
  // Image with 2x only.
  const int kSize2x = 50;
  gfx::ImageSkia image_skia;
  image_skia.AddRepresentation(gfx::ImageSkiaRep(gt::CreateBitmap(
      kSize2x, kSize2x), ui::SCALE_FACTOR_200P));
  gfx::Image image1(image_skia);
  scoped_refptr<base::RefCountedBytes> png_bytes1 = image1.As1xPNGBytes();
  EXPECT_TRUE(png_bytes1.get());
  EXPECT_FALSE(png_bytes1->size());

  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(
      gt::CreatePNGBytes(kSize2x), ui::SCALE_FACTOR_200P));
  gfx::Image image2(image_png_reps);
  scoped_refptr<base::RefCountedBytes> png_bytes2 = image2.As1xPNGBytes();
  EXPECT_TRUE(png_bytes2.get());
  EXPECT_FALSE(png_bytes2->size());
}

// Check that for an image initialized with multi resolution PNG data,
// As1xPNGBytes() returns the 1x bytes.
TEST_F(ImageTest, CreateExtractPNGBytes) {
  const int kSize1x = 25;
  const int kSize2x = 50;

  scoped_refptr<base::RefCountedBytes> bytes1x = gt::CreatePNGBytes(kSize1x);
  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(bytes1x, ui::SCALE_FACTOR_100P));
  image_png_reps.push_back(gfx::ImagePNGRep(
      gt::CreatePNGBytes(kSize2x), ui::SCALE_FACTOR_200P));

  gfx::Image image(image_png_reps);

  EXPECT_TRUE(std::equal(bytes1x->front(), bytes1x->front() + bytes1x->size(),
                         image.As1xPNGBytes()->front()));
}

TEST_F(ImageTest, MultiResolutionImageSkiaToPNG) {
  const int kSize1x = 25;
  const int kSize2x = 50;

  SkBitmap bitmap_1x = gt::CreateBitmap(kSize1x, kSize1x);
  gfx::ImageSkia image_skia;
  image_skia.AddRepresentation(gfx::ImageSkiaRep(bitmap_1x,
                                                 ui::SCALE_FACTOR_100P));
  image_skia.AddRepresentation(gfx::ImageSkiaRep(gt::CreateBitmap(
      kSize2x, kSize2x), ui::SCALE_FACTOR_200P));
  gfx::Image image(image_skia);

  EXPECT_TRUE(gt::IsEqual(image.As1xPNGBytes(), bitmap_1x));
  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepPNG));
}

TEST_F(ImageTest, MultiResolutionPNGToImageSkia) {
  const int kSize1x = 25;
  const int kSize2x = 50;

  scoped_refptr<base::RefCountedBytes> bytes1x = gt::CreatePNGBytes(kSize1x);
  scoped_refptr<base::RefCountedBytes> bytes2x = gt::CreatePNGBytes(kSize2x);

  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(bytes1x, ui::SCALE_FACTOR_100P));
  image_png_reps.push_back(gfx::ImagePNGRep(bytes2x, ui::SCALE_FACTOR_200P));
  gfx::Image image(image_png_reps);

  std::vector<ui::ScaleFactor> scale_factors;
  scale_factors.push_back(ui::SCALE_FACTOR_100P);
  scale_factors.push_back(ui::SCALE_FACTOR_200P);
  gfx::ImageSkia image_skia = image.AsImageSkia();
  EXPECT_TRUE(gt::ImageSkiaStructureMatches(image_skia, kSize1x, kSize1x,
                                            scale_factors));
  EXPECT_TRUE(gt::IsEqual(bytes1x,
      image_skia.GetRepresentation(ui::SCALE_FACTOR_100P).sk_bitmap()));
  EXPECT_TRUE(gt::IsEqual(bytes2x,
      image_skia.GetRepresentation(ui::SCALE_FACTOR_200P).sk_bitmap()));
}

TEST_F(ImageTest, MultiResolutionPNGToPlatform) {
  const int kSize1x = 25;
  const int kSize2x = 50;

  scoped_refptr<base::RefCountedBytes> bytes1x = gt::CreatePNGBytes(kSize1x);
  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(bytes1x, ui::SCALE_FACTOR_100P));
  image_png_reps.push_back(gfx::ImagePNGRep(
      gt::CreatePNGBytes(kSize2x), ui::SCALE_FACTOR_200P));

  gfx::Image from_png(image_png_reps);
  gfx::Image from_platform(gt::CopyPlatformType(from_png));
  EXPECT_TRUE(gt::IsEqual(bytes1x, from_platform.AsBitmap()));
}


TEST_F(ImageTest, PlatformToPNGEncodeAndDecode) {
  gfx::Image image(gt::CreatePlatformImage());
  scoped_refptr<base::RefCountedBytes> png_data = image.As1xPNGBytes();
  EXPECT_TRUE(png_data.get());
  EXPECT_TRUE(png_data->size());
  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepPNG));

  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(png_data, ui::SCALE_FACTOR_100P));
  gfx::Image from_png(image_png_reps);

  EXPECT_TRUE(from_png.HasRepresentation(gfx::Image::kImageRepPNG));
  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(from_png)));
}

// The platform types use the platform provided encoding/decoding of PNGs. Make
// sure these work with the Skia Encode/Decode.
TEST_F(ImageTest, PNGEncodeFromSkiaDecodeToPlatform) {
  // Force the conversion sequence skia to png to platform_type.
  gfx::Image from_skia(gt::CreateBitmap(25, 25));
  scoped_refptr<base::RefCountedBytes> png_bytes =
      from_skia.As1xPNGBytes();

  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(png_bytes, ui::SCALE_FACTOR_100P));
  gfx::Image from_png(image_png_reps);

  gfx::Image from_platform(gt::CopyPlatformType(from_png));

  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(from_platform)));
  EXPECT_TRUE(gt::IsEqual(png_bytes, from_platform.AsBitmap()));
}

TEST_F(ImageTest, PNGEncodeFromPlatformDecodeToSkia) {
  // Force the conversion sequence platform_type to png to skia.
  gfx::Image from_platform(gt::CreatePlatformImage());
  scoped_refptr<base::RefCountedBytes> png_bytes = from_platform.As1xPNGBytes();
  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(png_bytes, ui::SCALE_FACTOR_100P));
  gfx::Image from_png(image_png_reps);

  EXPECT_TRUE(gt::IsEqual(from_platform.AsBitmap(), from_png.AsBitmap()));
}

TEST_F(ImageTest, PNGDecodeToSkiaFailure) {
  scoped_refptr<base::RefCountedBytes> invalid_bytes(
      new base::RefCountedBytes());
  invalid_bytes->data().push_back('0');
  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(
      invalid_bytes, ui::SCALE_FACTOR_100P));
  gfx::Image image(image_png_reps);
  gt::CheckImageIndicatesPNGDecodeFailure(image);
}

TEST_F(ImageTest, PNGDecodeToPlatformFailure) {
  scoped_refptr<base::RefCountedBytes> invalid_bytes(
      new base::RefCountedBytes());
  invalid_bytes->data().push_back('0');
  std::vector<gfx::ImagePNGRep> image_png_reps;
  image_png_reps.push_back(gfx::ImagePNGRep(
      invalid_bytes, ui::SCALE_FACTOR_100P));
  gfx::Image from_png(image_png_reps);
  gfx::Image from_platform(gt::CopyPlatformType(from_png));
  gt::CheckImageIndicatesPNGDecodeFailure(from_platform);
}

TEST_F(ImageTest, SkiaToPlatform) {
  gfx::Image image(gt::CreateBitmap(25, 25));
  const size_t kRepCount = kUsesSkiaNatively ? 1U : 2U;

  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepSkia));
  if (!kUsesSkiaNatively)
    EXPECT_FALSE(image.HasRepresentation(gt::GetPlatformRepresentationType()));

  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(image)));
  EXPECT_EQ(kRepCount, image.RepresentationCount());

  const SkBitmap* bitmap = image.ToSkBitmap();
  EXPECT_FALSE(bitmap->isNull());
  EXPECT_EQ(kRepCount, image.RepresentationCount());

  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepSkia));
  EXPECT_TRUE(image.HasRepresentation(gt::GetPlatformRepresentationType()));
}

TEST_F(ImageTest, PlatformToSkia) {
  gfx::Image image(gt::CreatePlatformImage());
  const size_t kRepCount = kUsesSkiaNatively ? 1U : 2U;

  EXPECT_TRUE(image.HasRepresentation(gt::GetPlatformRepresentationType()));
  if (!kUsesSkiaNatively)
    EXPECT_FALSE(image.HasRepresentation(gfx::Image::kImageRepSkia));

  const SkBitmap* bitmap = image.ToSkBitmap();
  EXPECT_TRUE(bitmap);
  EXPECT_FALSE(bitmap->isNull());
  EXPECT_EQ(kRepCount, image.RepresentationCount());

  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(image)));
  EXPECT_EQ(kRepCount, image.RepresentationCount());

  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepSkia));
}

TEST_F(ImageTest, PlatformToPlatform) {
  gfx::Image image(gt::CreatePlatformImage());
  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(image)));
  EXPECT_EQ(1U, image.RepresentationCount());

  // Make sure double conversion doesn't happen.
  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(image)));
  EXPECT_EQ(1U, image.RepresentationCount());

  EXPECT_TRUE(image.HasRepresentation(gt::GetPlatformRepresentationType()));
  if (!kUsesSkiaNatively)
    EXPECT_FALSE(image.HasRepresentation(gfx::Image::kImageRepSkia));
}

TEST_F(ImageTest, PlatformToSkiaToCopy) {
  const SkBitmap* bitmap;

  {
    gfx::Image image(gt::CreatePlatformImage());
    bitmap = image.CopySkBitmap();
  }

  EXPECT_TRUE(bitmap);
  EXPECT_FALSE(bitmap->isNull());

  delete bitmap;
}

#if defined(TOOLKIT_GTK)
TEST_F(ImageTest, SkiaToGdkCopy) {
  GdkPixbuf* pixbuf;

  {
    gfx::Image image(gt::CreateBitmap(25, 25));
    pixbuf = image.CopyGdkPixbuf();
  }

  EXPECT_TRUE(pixbuf);
  g_object_unref(pixbuf);
}

TEST_F(ImageTest, SkiaToCairoCreatesGdk) {
  gfx::Image image(gt::CreateBitmap(25, 25));
  EXPECT_FALSE(image.HasRepresentation(gfx::Image::kImageRepGdk));
  EXPECT_TRUE(image.ToCairo());
  EXPECT_TRUE(image.HasRepresentation(gfx::Image::kImageRepGdk));
}
#endif

#if defined(OS_IOS)
TEST_F(ImageTest, SkiaToCocoaTouchCopy) {
  UIImage* ui_image;

  {
    gfx::Image image(gt::CreateBitmap(25, 25));
    ui_image = image.CopyUIImage();
  }

  EXPECT_TRUE(ui_image);
  base::mac::NSObjectRelease(ui_image);
}
#elif defined(OS_MACOSX)
TEST_F(ImageTest, SkiaToCocoaCopy) {
  NSImage* ns_image;

  {
    gfx::Image image(gt::CreateBitmap(25, 25));
    ns_image = image.CopyNSImage();
  }

  EXPECT_TRUE(ns_image);
  base::mac::NSObjectRelease(ns_image);
}
#endif

TEST_F(ImageTest, CheckSkiaColor) {
  gfx::Image image(gt::CreatePlatformImage());

  const SkBitmap* bitmap = image.ToSkBitmap();
  SkAutoLockPixels auto_lock(*bitmap);
  gt::CheckColor(bitmap->getColor(10, 10), false);
}

TEST_F(ImageTest, SkBitmapConversionPreservesOrientation) {
  const int width = 50;
  const int height = 50;
  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
  bitmap.allocPixels();
  bitmap.eraseRGB(0, 255, 0);

  // Paint the upper half of the image in red (lower half is in green).
  SkCanvas canvas(bitmap);
  SkPaint red;
  red.setColor(SK_ColorRED);
  canvas.drawRect(SkRect::MakeWH(width, height / 2), red);
  {
    SCOPED_TRACE("Checking color of the initial SkBitmap");
    gt::CheckColor(bitmap.getColor(10, 10), true);
    gt::CheckColor(bitmap.getColor(10, 40), false);
  }

  // Convert from SkBitmap to a platform representation, then check the upper
  // half of the platform image to make sure it is red, not green.
  gfx::Image from_skbitmap(bitmap);
  {
    SCOPED_TRACE("Checking color of the platform image");
    gt::CheckColor(
        gt::GetPlatformImageColor(gt::ToPlatformType(from_skbitmap), 10, 10),
        true);
    gt::CheckColor(
        gt::GetPlatformImageColor(gt::ToPlatformType(from_skbitmap), 10, 40),
        false);
  }

  // Force a conversion back to SkBitmap and check that the upper half is red.
  gfx::Image from_platform(gt::CopyPlatformType(from_skbitmap));
  const SkBitmap* bitmap2 = from_platform.ToSkBitmap();
  SkAutoLockPixels auto_lock(*bitmap2);
  {
    SCOPED_TRACE("Checking color after conversion back to SkBitmap");
    gt::CheckColor(bitmap2->getColor(10, 10), true);
    gt::CheckColor(bitmap2->getColor(10, 40), false);
  }
}

TEST_F(ImageTest, SkBitmapConversionPreservesTransparency) {
  const int width = 50;
  const int height = 50;
  SkBitmap bitmap;
  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
  bitmap.allocPixels();
  bitmap.setIsOpaque(false);
  bitmap.eraseARGB(0, 0, 255, 0);

  // Paint the upper half of the image in red (lower half is transparent).
  SkCanvas canvas(bitmap);
  SkPaint red;
  red.setColor(SK_ColorRED);
  canvas.drawRect(SkRect::MakeWH(width, height / 2), red);
  {
    SCOPED_TRACE("Checking color of the initial SkBitmap");
    gt::CheckColor(bitmap.getColor(10, 10), true);
    gt::CheckIsTransparent(bitmap.getColor(10, 40));
  }

  // Convert from SkBitmap to a platform representation, then check the upper
  // half of the platform image to make sure it is red, not green.
  gfx::Image from_skbitmap(bitmap);
  {
    SCOPED_TRACE("Checking color of the platform image");
    gt::CheckColor(
        gt::GetPlatformImageColor(gt::ToPlatformType(from_skbitmap), 10, 10),
        true);
    gt::CheckIsTransparent(
        gt::GetPlatformImageColor(gt::ToPlatformType(from_skbitmap), 10, 40));
  }

  // Force a conversion back to SkBitmap and check that the upper half is red.
  gfx::Image from_platform(gt::CopyPlatformType(from_skbitmap));
  const SkBitmap* bitmap2 = from_platform.ToSkBitmap();
  SkAutoLockPixels auto_lock(*bitmap2);
  {
    SCOPED_TRACE("Checking color after conversion back to SkBitmap");
    gt::CheckColor(bitmap2->getColor(10, 10), true);
    gt::CheckIsTransparent(bitmap.getColor(10, 40));
  }
}

TEST_F(ImageTest, SwapRepresentations) {
  const size_t kRepCount = kUsesSkiaNatively ? 1U : 2U;

  gfx::Image image1(gt::CreateBitmap(25, 25));
  const SkBitmap* bitmap1 = image1.ToSkBitmap();
  EXPECT_EQ(1U, image1.RepresentationCount());

  gfx::Image image2(gt::CreatePlatformImage());
  const SkBitmap* bitmap2 = image2.ToSkBitmap();
  gt::PlatformImage platform_image = gt::ToPlatformType(image2);
  EXPECT_EQ(kRepCount, image2.RepresentationCount());

  image1.SwapRepresentations(&image2);

  EXPECT_EQ(bitmap2, image1.ToSkBitmap());
  EXPECT_TRUE(gt::PlatformImagesEqual(platform_image,
                                      gt::ToPlatformType(image1)));
  EXPECT_EQ(bitmap1, image2.ToSkBitmap());
  EXPECT_EQ(kRepCount, image1.RepresentationCount());
  EXPECT_EQ(1U, image2.RepresentationCount());
}

TEST_F(ImageTest, Copy) {
  const size_t kRepCount = kUsesSkiaNatively ? 1U : 2U;

  gfx::Image image1(gt::CreateBitmap(25, 25));
  gfx::Image image2(image1);

  EXPECT_EQ(1U, image1.RepresentationCount());
  EXPECT_EQ(1U, image2.RepresentationCount());
  EXPECT_EQ(image1.ToSkBitmap(), image2.ToSkBitmap());

  EXPECT_TRUE(gt::IsPlatformImageValid(gt::ToPlatformType(image2)));
  EXPECT_EQ(kRepCount, image2.RepresentationCount());
  EXPECT_EQ(kRepCount, image1.RepresentationCount());
}

TEST_F(ImageTest, Assign) {
  gfx::Image image1(gt::CreatePlatformImage());
  gfx::Image image2 = image1;

  EXPECT_EQ(1U, image1.RepresentationCount());
  EXPECT_EQ(1U, image2.RepresentationCount());
  EXPECT_EQ(image1.ToSkBitmap(), image2.ToSkBitmap());
}

TEST_F(ImageTest, MultiResolutionImageSkia) {
  const int kWidth1x = 10;
  const int kHeight1x = 12;
  const int kWidth2x = 20;
  const int kHeight2x = 24;

  gfx::ImageSkia image_skia;
  image_skia.AddRepresentation(gfx::ImageSkiaRep(
      gt::CreateBitmap(kWidth1x, kHeight1x),
      ui::SCALE_FACTOR_100P));
  image_skia.AddRepresentation(gfx::ImageSkiaRep(
      gt::CreateBitmap(kWidth2x, kHeight2x),
      ui::SCALE_FACTOR_200P));

  std::vector<ui::ScaleFactor> scale_factors;
  scale_factors.push_back(ui::SCALE_FACTOR_100P);
  scale_factors.push_back(ui::SCALE_FACTOR_200P);
  EXPECT_TRUE(gt::ImageSkiaStructureMatches(image_skia, kWidth1x, kHeight1x,
                                            scale_factors));

  // Check that the image has a single representation.
  gfx::Image image(image_skia);
  EXPECT_EQ(1u, image.RepresentationCount());
}

TEST_F(ImageTest, RemoveFromMultiResolutionImageSkia) {
  const int kWidth2x = 20;
  const int kHeight2x = 24;

  gfx::ImageSkia image_skia;

  image_skia.AddRepresentation(gfx::ImageSkiaRep(
      gt::CreateBitmap(kWidth2x, kHeight2x), ui::SCALE_FACTOR_200P));
  EXPECT_EQ(1u, image_skia.image_reps().size());

  image_skia.RemoveRepresentation(ui::SCALE_FACTOR_100P);
  EXPECT_EQ(1u, image_skia.image_reps().size());

  image_skia.RemoveRepresentation(ui::SCALE_FACTOR_200P);
  EXPECT_EQ(0u, image_skia.image_reps().size());
}

// Tests that gfx::Image does indeed take ownership of the SkBitmap it is
// passed.
TEST_F(ImageTest, OwnershipTest) {
  gfx::Image image;
  {
    SkBitmap bitmap(gt::CreateBitmap(10, 10));
    EXPECT_TRUE(!bitmap.isNull());
    image = gfx::Image(gfx::ImageSkia(
        gfx::ImageSkiaRep(bitmap, ui::SCALE_FACTOR_100P)));
  }
  EXPECT_TRUE(!image.ToSkBitmap()->isNull());
}

// Integration tests with UI toolkit frameworks require linking against the
// Views library and cannot be here (ui_unittests doesn't include it). They
// instead live in /chrome/browser/ui/tests/ui_gfx_image_unittest.cc.

}  // namespace
