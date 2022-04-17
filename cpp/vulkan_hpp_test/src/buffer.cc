#include "buffer.h"

#include <assert.h>

#include "device.h"

namespace vulkan_hpp_test {

Buffer::Buffer() { Reset(); }
Buffer::Buffer(Buffer&& other) {
  handle_    = other.handle_;
  vk_buffer_ = std::move(other.vk_buffer_);
  device_id_ = other.device_id_;
  devices_   = std::move(other.devices_);
  other.devices_.clear();

  size_byte_ = other.size_byte_;

  other.Reset();
}

Buffer::~Buffer() {
  if (vk_buffer_.get() != nullptr) {
    std::shared_ptr<Device> sp_device = devices_.at(device_id_).lock();
    // TODO(any) :Check nullptr and handle error
    sp_device->ReturnendBuffer(handle_, std::move(vk_buffer_));
  }
  Reset();
  assert(size_byte_ == uint32_t(0));
};

bool Buffer::Allocate(const uint32_t device_id,
                      std::vector<std::weak_ptr<Device>> devices,
                      size_t size_byte) {
  if (vk_buffer_.get() != nullptr) {
    // TODO(any) : Handle error
    return false;
  }
  if (device_id >= devices.size()) {
    // TODO(any) : Handle error
    return false;
  }

  device_id_ = device_id;
  devices_   = devices;
  size_byte_ = size_byte;

  std::shared_ptr<Device> sp_device = devices_.at(device_id_).lock();
  // TODO(any) :Check nullptr and handle error

  auto [handle, vk_buffer] =
      sp_device->CreateVkBuffer(shared_from_this(), size_byte_);

  handle_    = handle;
  vk_buffer_ = std::move(vk_buffer);

  return true;
}

bool Buffer::FromCpuMemory(const uint8_t* src, size_t size_byte) {
  // TODO(any) :Check nullptr and handle error
  std::shared_ptr<Device> sp_device = devices_.at(device_id_).lock();

  sp_device->FromCpuMemory(handle_, src, size_byte);

  return true;
}

std::shared_ptr<CpuBuffer> Buffer::ToCpuBuffer() {
  std::shared_ptr<CpuBuffer> ret(new CpuBuffer());

  std::shared_ptr<Device> sp_device = devices_.at(device_id_).lock();

  std::unique_ptr<uint8_t[]> buf = sp_device->ToCpuMemory(handle_, size_byte_);

  const bool success = ret->From(devices_, std::move(buf), size_byte_);

  if (!success) {
    return std::shared_ptr<CpuBuffer>(nullptr);
  }

  return ret;
}

std::pair<uint32_t, std::unique_ptr<VkBuffer>> Buffer::ReturnVkBuffer() {
  return Reset();
}

std::pair<uint32_t, std::unique_ptr<VkBuffer>> Buffer::Reset() {
  std::pair<uint32_t, std::unique_ptr<VkBuffer>> ret(uint32_t(-1), nullptr);
  if (vk_buffer_.get() != nullptr) {
    ret.first  = handle_;
    ret.second = std::move(vk_buffer_);
    vk_buffer_.reset(nullptr);
  }
  device_id_ = -1;
  devices_.clear();
  handle_    = -1;
  size_byte_ = 0;

  return ret;
}

/////////////////// CpuBuffer ////////////////////////

CpuBuffer::CpuBuffer() : size_byte_(0) {}
CpuBuffer::~CpuBuffer() { size_byte_ = 0; }

CpuBuffer::CpuBuffer(CpuBuffer&& other) {
  size_byte_ = other.size_byte_;
  buf_       = std::move(other.buf_);
}

bool CpuBuffer::Allocate(std::vector<std::weak_ptr<Device>> devices,
                         size_t size_byte) {
  devices_ = devices;

  if (buf_.get() != nullptr) {
    // TODO(any) : Handle error
    return false;
  }
  buf_.reset(new uint8_t[size_byte]);
  size_byte_ = size_byte;

  return true;
}
std::shared_ptr<Buffer> CpuBuffer::ToDeviceBuffer(const uint32_t device_id) {
  // TODO (any) Check device_id and handle error
  std::shared_ptr<Buffer> ret(new Buffer());
  ret->Allocate(device_id, devices_, size_byte_);

  const bool success = ret->FromCpuMemory(buf_.get(), size_byte_);

  if (!success) {
    return std::shared_ptr<Buffer>(nullptr);
  }

  return ret;
}

bool CpuBuffer::From(std::vector<std::weak_ptr<Device>> devices,
                     std::unique_ptr<uint8_t[]> buf, const size_t size_byte) {
  devices_ = devices;

  if (buf_.get() != nullptr) {
    // TODO(any) : Handle error
    return false;
  }
  size_byte_ = size_byte;
  buf_       = std::move(buf);

  return true;
}

#ifdef ENABLE_PYBIND11

template <typename T>
void Copy(uint8_t* dst, const T* src, const size_t n) {
  T* p = reinterpret_cast<T*>(dst);
  for (size_t i = 0; i < n; ++i) {
    p[i] = src[i];
  }
}

size_t FetchSizeType(const pybind11::array& a) {
  namespace py = pybind11;

  size_t size_type = 1;
  auto i8ty        = py::dtype::of<int8_t>();
  auto i16ty       = py::dtype::of<int16_t>();
  auto i32ty       = py::dtype::of<int32_t>();
  auto i64ty       = py::dtype::of<int64_t>();

  auto u8ty  = py::dtype::of<uint8_t>();
  auto u16ty = py::dtype::of<uint16_t>();
  auto u32ty = py::dtype::of<uint32_t>();
  auto u64ty = py::dtype::of<uint64_t>();

  auto f32ty = py::dtype::of<float>();
  auto f64ty = py::dtype::of<double>();

  if (i8ty.is(a.dtype()) || u8ty.is(a.dtype())) {
    size_type = 1;
  } else if (i16ty.is(a.dtype()) || u16ty.is(a.dtype())) {
    size_type = 2;
  } else if (i32ty.is(a.dtype()) || u32ty.is(a.dtype()) ||
             f32ty.is(a.dtype())) {
    size_type = 4;
  } else if (i64ty.is(a.dtype()) || u64ty.is(a.dtype()) ||
             f64ty.is(a.dtype())) {
    size_type = 8;
  }

  return size_type;
}

bool CpuBuffer::FromNumpy(const pybind11::array& a) {
  namespace py = pybind11;

  const size_t size_type = FetchSizeType(a);

  ssize_t n = 1;
  for (ssize_t i = 0; i < a.ndim(); ++i) {
    n *= a.shape()[i];
  }

  const uint32_t size_byte = n * size_type;

  if (size_byte_ < size_byte) {
    return false;
  }

  const py::buffer_info b = a.request();
  const void* ptr         = b.ptr;
  memcpy(buf_.get(), ptr, size_byte);

  // if (i8ty.is(a.dtype())) {
  //   using T = int8_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (i16ty.is(a.dtype())) {
  //   using T = int16_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (i32ty.is(a.dtype())) {
  //   using T = int32_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (i64ty.is(a.dtype())) {
  //   using T = int64_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (u8ty.is(a.dtype())) {
  //   using T = uint8_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (u16ty.is(a.dtype())) {
  //   using T = uint16_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (u32ty.is(a.dtype())) {
  //   using T = uint32_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (u64ty.is(a.dtype())) {
  //   using T = uint64_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }
  // else if (u64ty.is(a.dtype())) {
  //   using T = uint64_t;
  //   T* ptr  = reinterpret_cast<T*>(b.ptr);
  //   Copy<T>(buf_.get(), ptr, n);
  // }

  return true;
}
pybind11::array_t<float> CpuBuffer::ToNumpyf32() {
  namespace py = pybind11;
  assert(size_byte_ % 4 == 0);
  // TODO(any) Handle error

  py::array_t<float> a;
  a.resize({(size_byte_ + sizeof(float) - 1) / sizeof(float)});

  py::buffer_info b = a.request();
  void* ptr         = b.ptr;
  memcpy(ptr, buf_.get(), size_byte_);

  return a;
}

#endif  // ENABLE_PYBIND11

//////////////////////////////////////////////////////

}  // namespace vulkan_hpp_test
