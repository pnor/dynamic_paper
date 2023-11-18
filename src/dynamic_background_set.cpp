#include "dynamic_background_set.hpp"

namespace dynamic_paper {

DynamicBackgroundData::DynamicBackgroundData(
    std::filesystem::path dataDirectory, BackgroundSetMode mode,
    std::optional<unsigned int> transitionDuration, BackgroundSetOrder order,
    std::vector<std::string> imageNames, std::vector<time_t> times)
    : dataDirectory(dataDirectory), mode(mode),
      transitionDuration(transitionDuration), order(order),
      imageNames(imageNames), times(times) {}

} // namespace dynamic_paper
