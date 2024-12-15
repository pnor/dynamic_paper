#include <filesystem>
#include <iostream>
#include <utility>

// #include <CImg.h>
#include <Magick++.h>

// #include "Tracy.hpp"
#include "tracy/Tracy.hpp"

#include "src/file_util.hpp"
#include "src/nolint/cimg_compositor.hpp"

using namespace dynamic_paper;

namespace {

const std::filesystem::path START_IMG =
    "./files/backgrounds/benchmarking/start.jpg";
const std::filesystem::path END_IMG =
    "./files/backgrounds/benchmarking/end.jpg";

// void compositeUsingCImg(const std::filesystem::path &startImagePath,
//                         const std::filesystem::path &endImagePath,
//                         const std::filesystem::path &destinationImagePath,
//                         const unsigned int percentage) {
//
//   using namespace cimg_library;
//   CImg<unsigned char> baseImage = CImg<>(startImagePath.c_str());
//   CImg<unsigned char> compositeImage = CImg<>(endImagePath.c_str());
//   baseImage.draw_image(compositeImage, static_cast<float>(percentage) /
//   100.0F); baseImage.save(destinationImagePath.c_str());
// }

// void compositeUsingCImgTakeCImgs(
//     const cimg_library::CImg<unsigned char> &startImage,
//     const cimg_library::CImg<unsigned char> &endImage,
//     const std::filesystem::path &destinationImagePath,
//     const unsigned int percentage) {
//   ZoneScoped;
//
//   using namespace cimg_library;
//   FrameMarkStart("Comp - Copying Image");
//   CImg<unsigned char> compositeImage = startImage;
//   FrameMarkEnd("Comp - Copying Image");
//   FrameMarkStart("Comp - Drawing Image");
//   compositeImage.draw_image(endImage, static_cast<float>(percentage) /
//   100.0f); FrameMarkEnd("Comp - Drawing Image"); FrameMarkStart("Comp -
//   Saving Image");
//   // compositeImage.save(destinationImagePath.c_str());
//   compositeImage.save_jpeg(destinationImagePath.c_str());
//   FrameMarkEnd("Comp - Saving Image");
// }

void compositeUsingImageMagick(
    const Magick::Image &startImage, const Magick::Image &endImage,
    const std::filesystem::path &destinationImagePath,
    const unsigned int percentage) {
  ZoneScoped;

  FrameMarkStart("Comp - Copying Image");
  Magick::Image destImage = startImage;
  Magick::Image transpImage = endImage;
  FrameMarkEnd("Comp - Copying Image");

  FrameMarkStart("Comp - Drawing Image");
  transpImage.alphaChannel(Magick::ActivateAlphaChannel);
  const double perc = static_cast<double>(percentage) / 100.0;
  transpImage.evaluate(Magick::AlphaChannel,
                       Magick::MagickEvaluateOperator::SetEvaluateOperator,
                       perc);

  destImage.alphaChannel(Magick::ActivateAlphaChannel);
  destImage.evaluate(Magick::AlphaChannel,
                     Magick::MagickEvaluateOperator::SetEvaluateOperator,
                     (1 - perc));

  destImage.composite(transpImage, 0, 0, Magick::OverCompositeOp);
  // destImage.composite(transpImage, 0, 0, Magick::BlendCompositeOp);
  // transpImage.composite(destImage, 0, 0, Magick::DstOverCompositeOp);
  FrameMarkEnd("Comp - Drawing Image");

  // destImage.pi

  FrameMarkStart("Comp - Saving Image");
  destImage.write(destinationImagePath.c_str());
  FrameMarkEnd("Comp - Saving Image");
}

const std::array<std::pair<uint8_t, std::filesystem::path>, 4> compositeImagesPaths =
    // {
    //         std::make_pair(
    //             20, std::filesystem::path(
    //                     "./files/backgrounds/benchmarking/composite-20.jpg")),
    //         std::make_pair(
    //             40, std::filesystem::path(
    //                     "./files/backgrounds/benchmarking/composite-40.jpg")),
    //         std::make_pair(
    //             60, std::filesystem::path(
    //                     "./files/backgrounds/benchmarking/composite-60.jpg")),
    //         std::make_pair(
    //             80, std::filesystem::path(
    //                     "./files/backgrounds/benchmarking/composite-80.jpg")),
    // };
    {
        std::make_pair(20, std::filesystem::path("/tmp/composite-20.jpg")),
        std::make_pair(40, std::filesystem::path("/tmp/composite-40.jpg")),
        std::make_pair(60, std::filesystem::path("/tmp/composite-60.jpg")),
        std::make_pair(80, std::filesystem::path("/tmp/composite-80.jpg")),
};
} // namespace

auto main(int argc, char *argv[]) -> int {
  ZoneScoped;
  Magick::InitializeMagick(*argv);
  const bool result = Magick::EnableOpenCL();
  std::cout << "Open CL was enabled? " << (result ? "true" : "false")
            << std::endl;

  FrameMarkStart("Startup");

  //  using namespace cimg_library;
  // const CImg<unsigned char> startImage = CImg<>(START_IMG.c_str());
  // const CImg<unsigned char> endImage = CImg<>(END_IMG.c_str());

  Magick::Image startImage;
  Magick::Image endImage;
  startImage.read(START_IMG.c_str());
  endImage.read(END_IMG.c_str());

  FrameMarkEnd("Startup");

  // ---

  FrameMarkStart("Composite Loop");

  for (const std::pair<uint8_t, std::filesystem::path> &percentageAndPath :
       compositeImagesPaths) {
    ZoneScoped;

    const auto &percentage = percentageAndPath.first;
    const auto &path = percentageAndPath.second;

    // compositeUsingCImgTakeCImgs(startImage, endImage, path, percentage);
    compositeUsingImageMagick(startImage, endImage, path, percentage);
  }

  FrameMarkEnd("Composite Loop");
}

// TODO directly use image magick
// (cimg is literally calling it)
// https://imagemagick.org/script/magick++.php
