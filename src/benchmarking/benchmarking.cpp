#include <filesystem>
#include <iostream>
#include <utility>

#include "src/file_util.hpp"
#include "src/nolint/cimg_compositor.hpp"

using namespace dynamic_paper;

namespace {

const std::filesystem::path START_IMG =
    "./files/backgrounds/benchmarking/start.jpg";
const std::filesystem::path END_IMG =
    "./files/backgrounds/benchmarking/end.jpg";

const std::array<std::pair<uint8_t, std::filesystem::path>, 4>
    compositeImagesPaths = {
        std::make_pair(
            20, std::filesystem::path(
                    "./files/backgrounds/benchmarking/composite-20.jpg")),
        std::make_pair(
            40, std::filesystem::path(
                    "./files/backgrounds/benchmarking/composite-40.jpg")),
        std::make_pair(
            60, std::filesystem::path(
                    "./files/backgrounds/benchmarking/composite-60.jpg")),
        std::make_pair(
            80, std::filesystem::path(
                    "./files/backgrounds/benchmarking/composite-80.jpg")),
};

} // namespace

auto main(int argc, char *argv[]) -> int {
  for (const std::pair<uint8_t, std::filesystem::path> &percentageAndPath :
       compositeImagesPaths) {
    compositeUsingCImg(START_IMG, END_IMG, percentageAndPath.second,
                       percentageAndPath.first);
  }
}
