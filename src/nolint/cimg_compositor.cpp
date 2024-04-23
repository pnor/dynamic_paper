#include "cimg_compositor.hpp"

#include <CImg.h>

namespace dynamic_paper {

void compositeUsingCImg(const std::filesystem::path &startImagePath,
                        const std::filesystem::path &endImagePath,
                        const std::filesystem::path &destinationImagePath,
                        const unsigned int percentage) {

  using namespace cimg_library;
  CImg<unsigned char> baseImage = CImg<>(startImagePath.c_str());
  CImg<unsigned char> compositeImage = CImg<>(endImagePath.c_str());
  baseImage.draw_image(compositeImage, static_cast<float>(percentage) / 100.0F);
  baseImage.save(destinationImagePath.c_str());
}

} // namespace dynamic_paper
