#pragma once
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vulkan_hpp_test {

class Instance;

class Device {
public:
  Device() = delete;
  Device(vk::UniqueDevice&& device_, std::weak_ptr<Instance> instance);
  ~Device();

  vk::UniqueDevice device;

private:
  std::weak_ptr<Instance> instance_;
};

std::vector<std::shared_ptr<Device>> CreateDevices(
    std::weak_ptr<Instance> wp_instance, const uint32_t desired_version,
    const std::vector<const char*> device_extensions,
    const std::vector<const char*>& enabled_layers);

}  // namespace vulkan_hpp_test
