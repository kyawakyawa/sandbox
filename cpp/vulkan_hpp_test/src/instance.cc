#include "instance.h"

#include "vulkan/vulkan.hpp"

namespace vulkan_hpp_test {

Instance::Instance(
    const uint32_t desired_version,
    const std::vector<const char*>& enabled_layers,
    const std::vector<const char*>& enabled_instance_extentions) {
  vk::ApplicationInfo app_info("Compute", 0, "No Engine", 0, desired_version);

  vk::InstanceCreateInfo create_info(
      {}, &app_info, enabled_layers.size(), enabled_layers.data(),
      enabled_instance_extentions.size(), enabled_instance_extentions.data());

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
