#include "instance.h"

#include "vulkan/vulkan.hpp"

namespace vulkan_hpp_test {

Instance::Instance() {
  vk::ApplicationInfo app_info("Compute", VK_MAKE_VERSION(1, 0, 0), "No Engine",
                               VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_1);

  vk::InstanceCreateInfo create_info({}, &app_info, 0, nullptr, 0, nullptr);

  instance = vk::createInstanceUnique(create_info, nullptr);

#ifndef NDEBUG
  printf("--- available instance extensions ---\n");

  for (const auto& extension : vk::enumerateInstanceExtensionProperties()) {
    printf("\t%s\n", extension.extensionName.data());
  }
#endif  // NDEBUG
}

Instance::~Instance() = default;

}  // namespace vulkan_hpp_test
