#pragma once
#include <memory>
//
#include <vulkan/vulkan.h>

namespace vulkan_hpp_test {

class Device;

class Buffer : public std::enable_shared_from_this<Buffer> {
public:
  Buffer();
  ~Buffer();
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&& other);

  bool Allocate(std::weak_ptr<Device> device, size_t size_byte);

  std::pair<uint32_t, std::unique_ptr<VkBuffer>> ReturnVkBuffer();

private:
  std::pair<uint32_t, std::unique_ptr<VkBuffer>> Reset();

  uint32_t handle_ = -1;
  std::unique_ptr<VkBuffer> vk_buffer_;
  std::weak_ptr<Device> device_;
  size_t size_byte_;
};

}  // namespace vulkan_hpp_test
