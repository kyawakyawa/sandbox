#pragma once

#include <spdlog/spdlog.h>

#include <filesystem>

#define LOGGER_NAME "spdlog_test"

#define LOG_DEBUG(fmt, ...)                                                \
  if (auto logger = ::spdlog::get(LOGGER_NAME)) {                          \
    logger->debug("[{}: {}, in func \"{}()\"]: " fmt,                      \
                  ::std::filesystem::relative(__FILE__).c_str(), __LINE__, \
                  __func__ __VA_OPT__(, ) __VA_ARGS__);                    \
  }

#define LOG_INFO(fmt, ...)                                                \
  if (auto logger = ::spdlog::get(LOGGER_NAME)) {                         \
    logger->info("[{}: {}, in func \"{}()\"]: " fmt,                      \
                 ::std::filesystem::relative(__FILE__).c_str(), __LINE__, \
                 __func__ __VA_OPT__(, ) __VA_ARGS__);                    \
  }

#define LOG_WARN(fmt, ...)                                                \
  if (auto logger = ::spdlog::get(LOGGER_NAME)) {                         \
    logger->warn("[{}: {}, in func \"{}()\"]: " fmt,                      \
                 ::std::filesystem::relative(__FILE__).c_str(), __LINE__, \
                 __func__ __VA_OPT__(, ) __VA_ARGS__);                    \
  }

#define LOG_ERROR(fmt, ...)                                                \
  if (auto logger = ::spdlog::get(LOGGER_NAME)) {                          \
    logger->error("[{}: {}, in func \"{}()\"]: " fmt,                      \
                  ::std::filesystem::relative(__FILE__).c_str(), __LINE__, \
                  __func__ __VA_OPT__(, ) __VA_ARGS__);                    \
  }
