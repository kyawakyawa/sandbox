#pragma once
#include <atomic>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
//
#include <vulkan/vulkan.hpp>
//
#include "buffer.h"

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

  /*handle, VkBuffer*/ std::pair<uint32_t, std::unique_ptr<VkBuffer>>
  CreateVkBuffer(const std::weak_ptr<Buffer> buffer, const size_t size_byte);

  void ReturnendBuffer(const uint32_t handle, std::unique_ptr<VkBuffer> vk_buffer);

  vk::PhysicalDevice physical_device;
  vk::UniqueDevice device;

private:

  std::weak_ptr<Instance> instance_;
  std::unique_ptr<VmaAllocator> vma_allocator_;
  std::unordered_map<uint32_t, std::pair<std::weak_ptr<Buffer>, VmaAllocation>>
      buffers_;
  std::atomic_uint32_t buffer_conter_{0};
};

std::vector<std::shared_ptr<Device>> CreateDevices(
    std::weak_ptr<Instance> wp_instance,
    const std::vector<const char*> device_extensions,
    const std::vector<const char*>& enabled_layers);

}  // namespace vulkan_hpp_test
