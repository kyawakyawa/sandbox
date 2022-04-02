#include <vulkan/vulkan.hpp>

namespace vulkan_hpp_test {
class Instance {
public:
  Instance();

private:
  vk::UniqueInstance instance_;
};

}  // namespace vulkan_hpp_test
