#pragma  once

#include <vulkan/vulkan.hpp>

namespace vulkan_hpp_test {
class Instance {
public:
  Instance();
  ~Instance();

  vk::UniqueInstance instance;
private:
};

}  // namespace vulkan_hpp_test
