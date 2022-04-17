#pragma once
#include <stdio.h>

#include <memory>
#include <vector>
//
#include <vulkan/vulkan.h>

#define ENABLE_PYBIND11

#ifdef ENABLE_PYBIND11
#include "pybind11/numpy.h"
#endif  // ENABLE_PYBIND11

namespace vulkan_hpp_test {

class Device;

class CpuBuffer;

class Buffer : public std::enable_shared_from_this<Buffer> {
public:
  Buffer();
  ~Buffer();
  Buffer(const Buffer&) = delete;
  Buffer(Buffer&& other);

  bool Allocate(const uint32_t device_id,
                std::vector<std::weak_ptr<Device>> devices, size_t size_byte);
  bool FromCpuMemory(const uint8_t* src, size_t size_byte);

  std::shared_ptr<CpuBuffer> ToCpuBuffer();

  std::pair<uint32_t, std::unique_ptr<VkBuffer>> ReturnVkBuffer();

private:
  std::pair<uint32_t, std::unique_ptr<VkBuffer>> Reset();

  uint32_t handle_ = -1;
  std::unique_ptr<VkBuffer> vk_buffer_;
  uint32_t device_id_ = -1;
  std::vector<std::weak_ptr<Device>> devices_;
  // std::weak_ptr<Device> device_;
  size_t size_byte_;
};

class CpuBuffer {
public:
  CpuBuffer();
  ~CpuBuffer();
  CpuBuffer(const CpuBuffer&) = delete;
  CpuBuffer(CpuBuffer&& other);

  bool Allocate(std::vector<std::weak_ptr<Device>> devices, size_t size_byte);

  std::shared_ptr<Buffer> ToDeviceBuffer(const uint32_t device_id);

  bool From(std::vector<std::weak_ptr<Device>> devices,
            std::unique_ptr<uint8_t[]> buf, const size_t size_byte);

#ifdef ENABLE_PYBIND11
  bool FromNumpy(const pybind11::array& a);
  pybind11::array_t<float> ToNumpyf32();
#endif  // ENABLE_PYBIND11

private:
  std::unique_ptr<uint8_t[]> buf_;
  std::vector<std::weak_ptr<Device>> devices_;
  size_t size_byte_;
};

}  // namespace vulkan_hpp_test
