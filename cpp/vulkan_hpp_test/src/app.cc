#include "app.h"

namespace vulkan_hpp_test {

App::App() : instance_(new Instance) {
  uint32_t desired_version = VK_MAKE_VERSION(1, 1, 0);

  // clang-format off
  const std::vector<const char*> device_extensions = {
      VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME,            // Clspvを使う場合に必要
#if !(__APPLE__) // Molten VKでは対応したいないみたい(2022/03/31現在) ValidationLayerでエラーが出るが問題ない？
       VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,     // Clspvを使う場合に必要
#endif // 1(__APPLE__)
      VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME, // Clspvを使う場合に必要

      VK_KHR_8BIT_STORAGE_EXTENSION_NAME,  // shader interfaceに8bitの型用いる
      VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME  // shader内で8bit intを用いる 
                                                 // (SPIR-Vで OpCapability Int8を使えるようにする)
  };

  // clang-format on

  const std::vector<const char*> enabled_layers = {};

  devices_ = CreateDevices(instance_->instance, desired_version, device_extensions,
                           enabled_layers);
}

App::~App() {
}

uint32_t App::GetNumDevices() {
  return devices_.size();
}

}  // namespace vulkan_hpp_test
