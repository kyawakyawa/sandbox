#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vulkan_hpp_test {

class Instance;

class Device {
public:
  Device() = delete;
  Device(vk::UniqueDevice&& device);
  ~Device();

private:
  std::weak_ptr<Instance> instance_;
  vk::UniqueDevice device_;
};

std::vector<std::shared_ptr<Device>> CreateDevices(
    const vk::UniqueInstance& instance, const uint32_t desired_version,
    const std::vector<const char*> device_extensions,
    const std::vector<const char*>& enabled_layers);

}  // namespace vulkan_hpp_test
