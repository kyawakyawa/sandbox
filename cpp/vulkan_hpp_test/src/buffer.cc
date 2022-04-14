#include "buffer.h"

#include <assert.h>

#include "device.h"

namespace vulkan_hpp_test {

Buffer::Buffer() { Reset(); }
Buffer::Buffer(Buffer&& other) {
  handle_    = other.handle_;
  vk_buffer_ = std::move(other.vk_buffer_);
  device_    = other.device_;
  size_byte_ = other.size_byte_;

  other.Reset();
}

Buffer::~Buffer() {
  auto [handle, vk_buffer] = Reset();
  if (vk_buffer.get() != nullptr) {
    std::shared_ptr<Device> sp_device = device_.lock();
    // TODO(any) :Check nullptr and handle error
    sp_device->ReturnendBuffer(handle, std::move(vk_buffer));
  }
  assert(size_byte_ == uint32_t(0));
};

bool Buffer::Allocate(std::weak_ptr<Device> device, size_t size_byte) {
  if (vk_buffer_.get() != nullptr) {
    // TODO(any) : Handle error
    return false;
  }
  device_    = device;
  size_byte_ = size_byte;

  std::shared_ptr<Device> sp_device = device_.lock();
  // TODO(any) :Check nullptr and handle error

  auto [handle, vk_buffer] =
      sp_device->CreateVkBuffer(shared_from_this(), size_byte_);

  handle_    = handle;
  vk_buffer_ = std::move(vk_buffer);

  return true;
}

std::pair<uint32_t,std::unique_ptr<VkBuffer>> Buffer::ReturnVkBuffer() { return Reset(); }

std::pair<uint32_t, std::unique_ptr<VkBuffer>> Buffer::Reset() {
  std::pair<uint32_t, std::unique_ptr<VkBuffer>> ret(uint32_t(-1), nullptr);
  if (vk_buffer_.get() != nullptr) {
    ret.first  = handle_;
    ret.second = std::move(vk_buffer_);
    vk_buffer_.reset(nullptr);
  }
  handle_    = -1;
  size_byte_ = 0;

  return ret;
}

}  // namespace vulkan_hpp_test
