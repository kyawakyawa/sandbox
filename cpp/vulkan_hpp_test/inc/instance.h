#pragma once

#include <vulkan/vulkan.hpp>

namespace vulkan_hpp_test {
class Instance {
public:
  Instance() = delete;
  Instance(const uint32_t desired_version,
           const std::vector<const char*>& enabled_layers,
           const std::vector<const char*>& enabled_instance_extentions);
  ~Instance();

  vk::UniqueInstance instance;

private:
};

}  // namespace vulkan_hpp_test
