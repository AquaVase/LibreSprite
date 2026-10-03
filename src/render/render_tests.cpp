// Aseprite Document Library
// Copyright (c) 2001-2014 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtest/gtest.h>

#include "render/render.h"

#include "doc/cel.h"
#include "doc/context.h"
#include "doc/document.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/palette.h"
#include "doc/primitives.h"

using namespace doc;
using namespace render;

template<typename T>
class RenderAllModes : public testing::Test {
protected:
  RenderAllModes() { }
};

typedef testing::Types<RgbTraits, GrayscaleTraits, IndexedTraits> ImageAllTraits;
TYPED_TEST_CASE(RenderAllModes, ImageAllTraits);

// a b
// c d
#define EXPECT_2X2_PIXELS(image, a, b, c, d) \
  EXPECT_EQ(a, get_pixel(image, 0, 0));      \
  EXPECT_EQ(b, get_pixel(image, 1, 0));      \
  EXPECT_EQ(c, get_pixel(image, 0, 1));      \
  EXPECT_EQ(d, get_pixel(image, 1, 1))

// a b c d
// e f g h
// i j k l
// m n o p
#define EXPECT_4X4_PIXELS(image, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
  EXPECT_EQ(a, get_pixel(image, 0, 0));                                 \
  EXPECT_EQ(b, get_pixel(image, 1, 0));                                 \
  EXPECT_EQ(c, get_pixel(image, 2, 0));                                 \
  EXPECT_EQ(d, get_pixel(image, 3, 0));                                 \
  EXPECT_EQ(e, get_pixel(image, 0, 1));                                 \
  EXPECT_EQ(f, get_pixel(image, 1, 1));                                 \
  EXPECT_EQ(g, get_pixel(image, 2, 1));                                 \
  EXPECT_EQ(h, get_pixel(image, 3, 1));                                 \
  EXPECT_EQ(i, get_pixel(image, 0, 2));                                 \
  EXPECT_EQ(j, get_pixel(image, 1, 2));                                 \
  EXPECT_EQ(k, get_pixel(image, 2, 2));                                 \
  EXPECT_EQ(l, get_pixel(image, 3, 2));                                 \
  EXPECT_EQ(m, get_pixel(image, 0, 3));                                 \
  EXPECT_EQ(n, get_pixel(image, 1, 3));                                 \
  EXPECT_EQ(o, get_pixel(image, 2, 3));                                 \
  EXPECT_EQ(p, get_pixel(image, 3, 3))

TEST(Render, Basic)
{
  Context ctx;
  Document* doc = ctx.documents().add(2, 2, ColorMode::RGB);

  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 2);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 2, 2));
  clear_image(dst, 1);
  EXPECT_2X2_PIXELS(dst, 1, 1, 1, 1);

  Render render;
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  EXPECT_2X2_PIXELS(dst, 2, 2, 2, 2);
}

TYPED_TEST(RenderAllModes, CheckDefaultBackgroundMode)
{
  typedef TypeParam ImageTraits;

  Context ctx;
  Document* doc = ctx.documents().add(2, 2,
    ColorMode(ImageTraits::pixel_format));

  EXPECT_TRUE(!doc->sprite()->layer(0)->isBackground());
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 0);
  put_pixel(src, 1, 1, 1);

  std::unique_ptr<Image> dst(Image::create(ImageTraits::pixel_format, 2, 2));
  clear_image(dst, 1);
  EXPECT_2X2_PIXELS(dst, 1, 1, 1, 1);

  Render render;
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  // Default background mode is to set all pixels to transparent color
  EXPECT_2X2_PIXELS(dst, 0, 0, 0, 1);
}

TEST(Render, DefaultBackgroundModeWithNonzeroTransparentIndex)
{
  Context ctx;
  Document* doc = ctx.documents().add(2, 2, ColorMode::INDEXED);
  doc->sprite()->setTransparentColor(2); // Transparent color is index 2

  EXPECT_TRUE(!doc->sprite()->layer(0)->isBackground());
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 2);
  put_pixel(src, 1, 1, 1);

  std::unique_ptr<Image> dst(Image::create(IMAGE_INDEXED, 2, 2));
  clear_image(dst, 1);
  EXPECT_2X2_PIXELS(dst, 1, 1, 1, 1);

  Render render;
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  EXPECT_2X2_PIXELS(dst, 2, 2, 2, 1); // Indexed transparent

  dst.reset(Image::create(IMAGE_RGB, 2, 2));
  clear_image(dst, 1);
  EXPECT_2X2_PIXELS(dst, 1, 1, 1, 1);
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  color_t c1 = doc->sprite()->palette(0)->entry(1);
  EXPECT_NE(0, c1);
  EXPECT_2X2_PIXELS(dst, 0, 0, 0, c1); // RGB transparent
}

TEST(Render, CheckedBackground)
{
  Context ctx;
  Document* doc = ctx.documents().add(4, 4, ColorMode::RGB);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 4, 4));
  clear_image(dst, 0);

  Render render;
  render.setBgType(BgType::CHECKED);
  render.setBgZoom(true);
  render.setBgColor1(1);
  render.setBgColor2(2);

  render.setBgCheckedSize(gfx::Size(1, 1));
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst,
    1, 2, 1, 2,
    2, 1, 2, 1,
    1, 2, 1, 2,
    2, 1, 2, 1);

  render.setBgCheckedSize(gfx::Size(2, 2));
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst,
    1, 1, 2, 2,
    1, 1, 2, 2,
    2, 2, 1, 1,
    2, 2, 1, 1);

  render.setBgCheckedSize(gfx::Size(3, 3));
  render.renderSprite(dst, doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst,
    1, 1, 1, 2,
    1, 1, 1, 2,
    1, 1, 1, 2,
    2, 2, 2, 1);

  render.setBgCheckedSize(gfx::Size(1, 1));
  render.renderSprite(dst,
    doc->sprite(), frame_t(0),
    gfx::Clip(dst->bounds()),
    Zoom(2, 1));
  EXPECT_4X4_PIXELS(dst,
    1, 1, 2, 2,
    1, 1, 2, 2,
    2, 2, 1, 1,
    2, 2, 1, 1);
}

TEST(Render, ZoomAndDstBounds)
{
  Context ctx;

  // Create this image:
  // 0 0 0
  // 0 4 4
  // 0 4 4
  Document* doc = ctx.documents().add(3, 3, ColorMode::RGB);
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 0);
  fill_rect(src, 1, 1, 2, 2, 4);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 4, 4));
  clear_image(dst, 0);

  Render render;
  render.setBgType(BgType::CHECKED);
  render.setBgZoom(true);
  render.setBgColor1(1);
  render.setBgColor2(2);
  render.setBgCheckedSize(gfx::Size(1, 1));

  render.renderSprite(dst, doc->sprite(), frame_t(0),
    gfx::Clip(1, 1, 0, 0, 2, 2),
    Zoom(1, 1));
  EXPECT_4X4_PIXELS(dst,
    0, 0, 0, 0,
    0, 1, 2, 0,
    0, 2, 4, 0,
    0, 0, 0, 0);
}

TEST(Render, DownscaleDitheredPaletteColors)
{
  Context ctx;
  Document* doc = ctx.documents().add(8, 8, ColorMode::INDEXED);
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  Palette* pal = doc->sprite()->palette(0);
  pal->setEntry(1, rgba(240, 0, 0, 255));
  pal->setEntry(2, rgba(0, 0, 240, 255));
  for (int y=0; y<8; ++y)
    for (int x=0; x<8; ++x)
      put_pixel(src, x, y, (x+y)%2+1);

  for (int factor : { 2, 4 }) {
    std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 8/factor, 8/factor));
    Render render;
    render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                        gfx::Clip(dst->bounds()), Zoom(1, factor));
    for (int y=0; y<dst->height(); ++y)
      for (int x=0; x<dst->width(); ++x)
        EXPECT_EQ(rgba(120, 0, 120, 255), get_pixel(dst.get(), x, y));
  }
}

TEST(Render, DownscaleTransparentPixels)
{
  std::unique_ptr<Image> src(Image::create(IMAGE_RGB, 2, 2));
  clear_image(src.get(), rgba(0, 255, 0, 0));
  put_pixel(src.get(), 1, 1, rgba(240, 0, 0, 255));
  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 1, 1));
  Render render;
  clear_image(dst.get(), 0);
  render.renderImage(dst.get(), src.get(), nullptr, 0, 0, Zoom(1, 2), 255, BlendMode::NORMAL);
  EXPECT_EQ(rgba(240, 0, 0, 64), get_pixel(dst.get(), 0, 0));

  clear_image(dst.get(), rgba(0, 0, 240, 255));
  render.renderImage(dst.get(), src.get(), nullptr, 0, 0, Zoom(1, 2), 255, BlendMode::NORMAL);
  EXPECT_EQ(rgba(60, 0, 180, 255), get_pixel(dst.get(), 0, 0));
}

TEST(Render, DownscaleClippedOddSizedImage)
{
  std::unique_ptr<Image> src(Image::create(IMAGE_RGB, 9, 9));
  for (int y=0; y<9; ++y)
    for (int x=0; x<9; ++x)
      put_pixel(src.get(), x, y, rgba(20*x, 20*y, 0, 255));
  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 1, 1));
  clear_image(dst.get(), 0);
  Render render;
  render.renderImage(dst.get(), src.get(), nullptr, -1, -1, Zoom(1, 4), 255, BlendMode::NORMAL);
  EXPECT_EQ(rgba(110, 110, 0, 255), get_pixel(dst.get(), 0, 0));
}

TEST(Render, DownscaleGrayscale)
{
  std::unique_ptr<Image> src(Image::create(IMAGE_GRAYSCALE, 2, 2));
  clear_image(src.get(), graya(0, 255));
  put_pixel(src.get(), 1, 1, graya(240, 255));
  std::unique_ptr<Image> dst(Image::create(IMAGE_GRAYSCALE, 1, 1));
  clear_image(dst.get(), 0);
  Render render;
  render.renderImage(dst.get(), src.get(), nullptr, 0, 0, Zoom(1, 2), 255, BlendMode::NORMAL);
  EXPECT_EQ(graya(60, 255), get_pixel(dst.get(), 0, 0));
}

TEST(Render, DownscaleIndexedDestinationKeepsIndices)
{
  std::unique_ptr<Image> src(Image::create(IMAGE_INDEXED, 2, 2));
  clear_image(src.get(), 2);
  put_pixel(src.get(), 0, 0, 1);
  std::unique_ptr<Image> dst(Image::create(IMAGE_INDEXED, 1, 1));
  clear_image(dst.get(), 0);
  Render render;
  render.renderImage(dst.get(), src.get(), nullptr, 0, 0, Zoom(1, 2), 255, BlendMode::SRC);
  EXPECT_EQ(1, get_pixel(dst.get(), 0, 0));
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
