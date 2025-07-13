#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

#include "lib.h"

int main() {
  printf("Without spdlog\n");
  spdlog_test::process();
  printf("With spdlog\n");

  auto console = spdlog::stdout_color_mt("spdlog_test");
  console.get()->set_level(spdlog::level::debug);
  spdlog_test::process();

  return 0;
}
