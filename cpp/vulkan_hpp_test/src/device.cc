#include "device.h"

#include <string>
#include <unordered_set>
#include <vector>

#include "vulkan/vulkan.hpp"

namespace vulkan_hpp_test {

static std::vector<vk::PhysicalDevice> FilterPhysicalDevices(
    const std::vector<vk::PhysicalDevice>& physical_devices,
    const uint32_t desired_version,
    const std::vector<char*> device_extensions) {
  std::vector<vk::PhysicalDevice> tmp0;
  std::copy_if(physical_devices.begin(), physical_devices.end(),
               std::back_inserter(tmp0),
               [&desired_version](const vk::PhysicalDevice& physical_device) {
                 return physical_device.getProperties().apiVersion >=
                        desired_version;
               });
  if (tmp0.empty()) {
    return tmp0;
  }

  std::vector<vk::PhysicalDevice> tmp1;
  std::copy_if(
      tmp0.begin(), tmp0.end(), std::back_inserter(tmp1),
      [](const vk::PhysicalDevice& physical_device) {
        const vk::PhysicalDeviceType device_type =
            physical_device.getProperties().deviceType;
#ifndef NDEBUG
        if (device_type == vk::PhysicalDeviceType::eDiscreteGpu) {
          printf("Found discrete GPU.");
        } else if (device_type == vk::PhysicalDeviceType::eIntegratedGpu) {
          printf("Found integrated GPU.");
        } else {
          printf("Found unknown device.");
        }
#endif

        return device_type == vk::PhysicalDeviceType::eDiscreteGpu ||
               device_type == vk::PhysicalDeviceType::eIntegratedGpu;
      });

  std::vector<vk::PhysicalDevice> tmp2;
  std::copy_if(tmp1.begin(), tmp1.end(), std::back_inserter(tmp2),
               [&device_extensions](const vk::PhysicalDevice& physical_device) {
                 std::unordered_set<std::string> required_extensions(
                     device_extensions.begin(), device_extensions.end());

                 std::vector<vk::ExtensionProperties> available_extensions =
                     physical_device.enumerateDeviceExtensionProperties<
                         std::allocator<vk::ExtensionProperties>>();
#ifndef NDEBUG
                 printf("----- Available Extensions(%s)-----\n",
                        physical_device.getProperties().deviceName.data());

                 for (const auto& extension : available_extensions) {
                   printf("     -- %s\n", extension.extensionName.data());
                 }
                 printf("-----------------------------------\n");
#endif

                 for (const auto& extension : available_extensions) {
                   required_extensions.erase(extension.extensionName);
                 }
                 for (const auto& extension_name : required_extensions) {
                   fprintf(stderr, "not find Extension : %s in (%s)\n",
                           extension_name.c_str(),
                           physical_device.getProperties().deviceName.data());
                 }

                 return required_extensions.empty();
               });

  return tmp2;
}

static std::vector<vk::PhysicalDevice> FindPhysicalDevices(
    const vk::UniqueInstance& instance, const uint32_t desired_version,
    const std::vector<char*> device_extensions) {
  return FilterPhysicalDevices(
      instance->enumeratePhysicalDevices<std::allocator<vk::PhysicalDevice>>(),
      desired_version, device_extensions);
}

std::vector<vk::UniqueDevice> CreateDevices(
    const vk::UniqueInstance& instance, const uint32_t desired_version,
    const std::vector<char*> device_extensions) {
  const std::vector<vk::PhysicalDevice> physical_devices =
      FindPhysicalDevices(instance, desired_version, device_extensions);

  return {};
}

}  // namespace vulkan_hpp_test
