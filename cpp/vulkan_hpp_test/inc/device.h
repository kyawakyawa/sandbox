#pragma once
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

//
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

namespace vulkan_hpp_test {

class Instance;

class Device {
public:
  Device() = delete;
  Device(const vk::PhysicalDevice& physical_device, vk::UniqueDevice&& device_,
         std::weak_ptr<Instance> instance);
  ~Device();

  vk::PhysicalDevice physical_device;
  vk::UniqueDevice device;

private:
  std::weak_ptr<Instance> instance_;
  std::unique_ptr<VmaAllocator> vma_allocator_;
  std::unordered_map<uint32_t, std::pair<VkBuffer, VmaAllocation>> buffers_;
};

std::vector<std::shared_ptr<Device>> CreateDevices(
    std::weak_ptr<Instance> wp_instance,
    const std::vector<const char*> device_extensions,
    const std::vector<const char*>& enabled_layers);

}  // namespace vulkan_hpp_test
