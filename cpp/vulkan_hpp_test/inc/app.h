#pragma once
#include <stdint.h>

#include <vector>

#include "device.h"
#include "instance.h"

namespace vulkan_hpp_test {

class App {
public:
  App();
  ~App();

  uint32_t GetNumDevices();

private:
  std::shared_ptr<Instance> instance_;
  std::vector<std::shared_ptr<Device>> devices_;

  std::vector<const char*> enabled_layers_;
  std::vector<const char*> enabled_instance_extentions_;
};

}  // namespace vulkan_hpp_test
