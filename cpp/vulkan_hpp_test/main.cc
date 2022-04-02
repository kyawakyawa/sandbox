#include <cstdlib>
#include <stdexcept>

#include "instance.h"

// Vulkan
#include <vulkan/vulkan.hpp>

static void app(void) { vulkan_hpp_test::Instance instance; }

int main() {
  try {
    app();
  } catch (const std::exception& e) {
    fprintf(stderr, "%s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
