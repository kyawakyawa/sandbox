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

  uint32_t GetVersion();

  vk::UniqueInstance instance;

private:
  uint32_t version_;
};

}  // namespace vulkan_hpp_test
