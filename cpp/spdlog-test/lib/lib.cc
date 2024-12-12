#include "lib.h"

#include <spdlog/spdlog.h>

#include "logger.h"
namespace spdlog_test {
char logger_name[] = "spdlog_test";
void process() {
  LOG_DEBUG("This");
  LOG_INFO("is");
  LOG_WARN("a");
  LOG_ERROR("test.");
}
}  // namespace spdlog_test
