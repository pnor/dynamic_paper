#include "magick_compositor.hpp"

#include <Magick++.h>

namespace dynamic_paper {

void compositeUsingImageMagick(
    const std::filesystem::path &startImagePath,
    const std::filesystem::path &endImagePath,
    const std::filesystem::path &destinationImagePath,
    const unsigned int percentage) {

  Magick::Image destImage;
  destImage.read(startImagePath.c_str());
  Magick::Image transpImage;
  transpImage.read(endImagePath.c_str());

  transpImage.alphaChannel(Magick::ActivateAlphaChannel);

  const double percentageDouble = static_cast<double>(percentage) / 100.0;

  transpImage.evaluate(Magick::AlphaChannel,
                       Magick::MagickEvaluateOperator::SetEvaluateOperator,
                       percentageDouble);

  destImage.alphaChannel(Magick::ActivateAlphaChannel);
  destImage.evaluate(Magick::AlphaChannel,
                     Magick::MagickEvaluateOperator::SetEvaluateOperator,
                     (1 - percentageDouble));

  destImage.composite(transpImage, 0, 0, Magick::OverCompositeOp);

  destImage.write(destinationImagePath.c_str());
}

} // namespace dynamic_paper
