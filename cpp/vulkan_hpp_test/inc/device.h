#pragma once
#include <memory>

namespace vulkan_hpp_test {

class Instance;

class Device {
public:
  Device();
  ~Device();

private:
  std::weak_ptr<Instance> instance_;
};

void CreateDevice(std::shared_ptr<Device>* devices);

}  // namespace vulkan_hpp_test
