#pragma once

namespace dynamic_paper {

// General Config
constexpr std::string_view BACKGROUND_SET_CONFIG_FILE = "background_config";
constexpr std::string_view HOOK_SCRIPT_KEY = "hook_script";
constexpr std::string_view IMAGE_CACHE_DIR_KEY = "cache_dir";
constexpr std::string_view LOGGING_KEY = "logging_level";
constexpr std::string_view LOG_FILE_KEY = "log_file";
constexpr std::string_view LATITUDE_KEY = "latitude";
constexpr std::string_view LONGITUDE_KEY = "longitude";
constexpr std::string_view SUNRISE_TIME_KEY = "sunrise";
constexpr std::string_view SUNSET_TIME_KEY = "sunset";
constexpr std::string_view USE_CONFIG_FILE_LOCATION_KEY = "use_config_file_location";
constexpr std::string_view METHOD_KEY = "method";
constexpr std::string_view SCRIPT_STRING = "script";
constexpr std::string_view WALLUTILS_STRING = "wallutils";

// Background Set Config
constexpr std::string_view DYNAMIC_STRING = "dynamic";
constexpr std::string_view STATIC_STRING = "static";

constexpr std::string_view LINEAR_STRING = "linear";
constexpr std::string_view RANDOM_STRING = "random";

constexpr std::string_view CENTER_STRING = "center";
constexpr std::string_view FILL_STRING = "fill";

constexpr std::string_view SUNWAIT_STRING = "sunwait";

constexpr std::string_view INFO_LOGGING_STRING = "info";
constexpr std::string_view WARNING_LOGGING_STRING = "warning";
constexpr std::string_view ERROR_LOGGING_STRING = "error";
constexpr std::string_view DEBUG_LOGGING_STRING = "debug";
constexpr std::string_view CRITICAL_LOGGING_STRING = "critical";
constexpr std::string_view TRACE_LOGGING_STRING = "trace";
constexpr std::string_view OFF_LOGGING_STRING = "off";

// YAML Dict Key
constexpr std::string_view IMAGE_DIRECTORY = "image_directory";
constexpr std::string_view IMAGES = "images";
constexpr std::string_view IMAGE = "image";
constexpr std::string_view MODE = "mode";
constexpr std::string_view ORDER = "order";
constexpr std::string_view TIMES = "times";
constexpr std::string_view TRANSITION_LENGTH = "transition_length";
constexpr std::string_view TYPE = "type";
constexpr std::string_view NUM_TRANSITION_STEPS = "number_transition_steps";
constexpr std::string_view IN_PLACE = "in_place";

} // namespace dynamic_paper
