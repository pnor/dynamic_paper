#include "static_background_set.hpp"

namespace dynamic_paper {

StaticBackgroundData::StaticBackgroundData(std::filesystem::path dataDirectory,
                                           BackgroundSetMode mode,
                                           std::vector<std::string> imageNames)
    : dataDirectory(dataDirectory), mode(mode), imageNames(imageNames) {}

void StaticBackgroundData::show() const {
  // TODO
  // choose random image to load
  // load the file
  // set it using wallpaper thing and the background set mode
}

} // namespace dynamic_paper
