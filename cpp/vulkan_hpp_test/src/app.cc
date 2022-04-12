#include "app.h"

#include <string.h>

#include <exception>

namespace vulkan_hpp_test {

#ifndef NDEBUG
static const bool kEnableValidationLayers = true;
#else
static const bool kEnableValidationLayers = false;
#endif

static void FindLayersAndInstanceExtentions(
    std::vector<const char*>* enabled_layers,
    std::vector<const char*>* enabled_instance_extentions) {
  if (kEnableValidationLayers) {
    // We get all supported layers with vkEnumerateInstanceLayerProperties.
    std::vector<vk::LayerProperties> layer_properties =
        vk::enumerateInstanceLayerProperties<
            std::allocator<vk::LayerProperties>>();

    // And then we simply check if VK_LAYER_KHRONOS_validation is among the
    // supported layers.

    bool found_layer = false;
    for (vk::LayerProperties prop : layer_properties) {
      if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
        found_layer = true;
        break;
      }
    }

    if (!found_layer) {
      throw std::runtime_error(
          "Layer VK_LAYER_KHRONOS_validation not supported\n");
    }
    enabled_layers->emplace_back(
        "VK_LAYER_KHRONOS_validation");  // Alright, we can use this layer.

    // We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    // in order to be able to print the warnings emitted by the validation
    // layer.

    // So again, we just check if the extension is among the supported
    // extensions.
    std::vector<vk::ExtensionProperties> extension_properties =
        vk::enumerateInstanceExtensionProperties<
            std::allocator<vk::ExtensionProperties>>();

    bool found_extension = false;
    for (const vk::ExtensionProperties& prop : extension_properties) {
      if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0) {
        found_extension = true;
        break;
      }
    }

    if (!found_extension) {
      throw std::runtime_error(
          "Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
    }
    enabled_instance_extentions->emplace_back(
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }
}

App::App() {
  FindLayersAndInstanceExtentions(&enabled_layers_,
                                  &enabled_instance_extentions_);
  const uint32_t desired_version = VK_MAKE_VERSION(1, 1, 0);
  instance_.reset(new Instance(desired_version, enabled_layers_,
                               enabled_instance_extentions_));

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

  devices_ = CreateDevices(instance_, device_extensions, enabled_layers_);
}

App::~App() {}

uint32_t App::GetNumDevices() { return devices_.size(); }

}  // namespace vulkan_hpp_test
