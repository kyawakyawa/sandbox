#include "device.h"

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "instance.h"
#include "vulkan/vulkan.hpp"

namespace vulkan_hpp_test {

static uint32_t GetComputeQueueFamilyIndex(
    const vk::PhysicalDevice& physical_device) {
  std::vector<vk::QueueFamilyProperties> queue_families =
      physical_device.getQueueFamilyProperties<
          std::allocator<vk::QueueFamilyProperties>>();

  uint32_t ret = 0;
  for (const vk::QueueFamilyProperties props : queue_families) {
    if (props.queueCount > 0 &&
        (props.queueFlags & vk::QueueFlagBits::eCompute)) {
      // found a queue with compute. We're done!
      break;
    }
  }
  if (ret == queue_families.size()) {
    throw std::runtime_error(
        "could not find a queue family that supports operations");
  }

  return ret;
}

static std::vector<vk::PhysicalDevice> FilterPhysicalDevices(
    const std::vector<vk::PhysicalDevice>& physical_devices,
    const uint32_t desired_version,
    const std::vector<const char*> device_extensions) {
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
          printf("Found discrete GPU. [%s]\n",
                 physical_device.getProperties().deviceName.data());
        } else if (device_type == vk::PhysicalDeviceType::eIntegratedGpu) {
          printf("Found integrated GPU. [%s]\n",
                 physical_device.getProperties().deviceName.data());
        } else {
          printf("Found unknown device. [%s]\n",
                 physical_device.getProperties().deviceName.data());
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
    const std::vector<const char*> device_extensions) {
  return FilterPhysicalDevices(
      instance->enumeratePhysicalDevices<std::allocator<vk::PhysicalDevice>>(),
      desired_version, device_extensions);
}

static std::vector<std::pair<vk::PhysicalDevice, vk::UniqueDevice>>
CreateVkDevices(const vk::UniqueInstance& instance,
                const uint32_t desired_version,
                const std::vector<const char*> device_extensions,
                const std::vector<const char*>& enabled_layers) {
  const std::vector<vk::PhysicalDevice> physical_devices =
      FindPhysicalDevices(instance, desired_version, device_extensions);

  std::vector<std::pair<vk::PhysicalDevice, vk::UniqueDevice>> ret;
  for (const vk::PhysicalDevice& physical_device : physical_devices) {
    // When creating the device, we also specify what queues it has.
    //  DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags flags_ = {}, uint32_t
    //  queueFamilyIndex_ = {}, uint32_t queueCount_ = {}, const float
    //  *pQueuePriorities_ = {})~

    // vk::DeviceQueueCreate queue_create_info(vk::DeviceQueueCreateFlags:: )
    //
    float queue_priorities =
        1.0;  // we only have one queue, so this is not that imporant.
    vk::DeviceQueueCreateInfo queue_create_info(
        /*flags*/ vk::DeviceQueueCreateFlags() /*FIXME*/,
        /*queueFamilyIndex_*/ GetComputeQueueFamilyIndex(physical_device),
        /*queueCount*/ 1, /* const floatpQueuePriorities_*/ &queue_priorities);

    // さらに、SPIR-Vで OpCapability Int8 に対応するために
    // vk::PhysicalDeviceShaderFloat16Int8Features を使う
    vk::PhysicalDevice8BitStorageFeatures device_8bit_storage_features = {};
    vk::PhysicalDeviceShaderFloat16Int8Features
        device_shader_float16_int8_features = {};
    // vk::PhysicalDevice8BitStorageFeaturesとvk::PhysicalDeviceShaderFloat16Int8Features
    // を調べるためにvk::PhysicalDeviceFeatures2を作成
    // vk::PhysicalDeviceFeatures2とvk::PhysicalDeviceFeaturesの違いはpNextが使えるか使えないか
    // 既存のvk::PhysicalDeviceFeaturesはメンバにある
    // vk::PhysicalDeviceFeatures2は
    // pNextにVkPhysicalDevice8BitStorageFeaturesを指定
    // さらにそのpNextにVkPhysicalDeviceShaderFloat16Int8Featuresを指定
    vk::PhysicalDeviceFeatures2 device_features2 = {};
    device_features2.setPNext(&device_8bit_storage_features);
    device_8bit_storage_features.setPNext(&device_shader_float16_int8_features);

    physical_device.getFeatures2(&device_features2);

#ifndef NDEBUG
    printf("----- VkPhysicalDevice8BitStorageFeatures -----\n");
    printf("     - storageBuffer8BitAccess           -> %s.\n",
           device_8bit_storage_features.storageBuffer8BitAccess ? "OK" : "NO");
    printf("     - uniformAndStorageBuffer8BitAccess -> %s.\n",
           device_8bit_storage_features.uniformAndStorageBuffer8BitAccess
               ? "OK"
               : "NO");
    printf("     - storagePushConstant8              -> %s.\n",
           device_8bit_storage_features.storagePushConstant8 ? "OK" : "NO");
#endif
    // 8bitのstrage bufferが利用できない場合、例外を投げる
    if (!device_8bit_storage_features.storageBuffer8BitAccess) {
      throw std::runtime_error("Cannot use 8bit strage Buffer\n");
    }
#ifndef NDEBUG
    printf("----- VkPhysicalDeviceShaderFloat16Int8Features -----\n");
    printf("     - shaderFloat16 -> %s.\n",
           device_shader_float16_int8_features.shaderFloat16 ? "OK" : "NO");
    printf("     - shaderInt8    -> %s.\n",
           device_shader_float16_int8_features.shaderInt8 ? "OK" : "NO");
#endif
    // SPIR-Vで OpCapability Int8が利用できない場合、例外を投げる
    if (!device_shader_float16_int8_features.shaderInt8) {
      throw std::runtime_error("Cannot use OpCapability Int8\n");
    }

    // Now we create the logical device. The logical device allows us to
    // interact with the physical device.

    vk::DeviceCreateInfo device_create_info(
        /*flags_*/ vk::DeviceCreateFlags() /*FIXME*/,
        /*queueCreateInfoCount_*/ 1,
        /*pQueueCreateInfos_*/ &queue_create_info,
        /*enabledLayerCount_*/ enabled_layers.size(),
        /*ppEnabledLayerNames_*/ enabled_layers.data(),
        /*enabledExtensionCount_*/ device_extensions.size(),
        /*ppEnabledExtensionNames_*/ device_extensions.data(),
        /*pEnabledFeatures_*/ &(device_features2.features)
#if 1
    );
    device_create_info.setPNext(&device_8bit_storage_features);
#else
            ,                            /*pNext_*/
        &device_8bit_storage_features);  // vk::PhysicalDevice8BitStorageFeaturesはpNextに指定する
                                         // 先程、vk::PhysicalDevice8BitStorageFeaturesの
                                         // pNextにvk::PhysicalDeviceShaderFloat16Int8Featuresも渡したので、
                                         // そちらもvkCreateDeviceに渡される
#endif

    ret.emplace_back(physical_device,
                     physical_device.createDeviceUnique(device_create_info));
  }

  return ret;
}

std::vector<std::shared_ptr<Device>> CreateDevices(
    std::weak_ptr<Instance> wp_instance,
    const std::vector<const char*> device_extensions,
    const std::vector<const char*>& enabled_layers) {
  std::shared_ptr<Instance> instance = wp_instance.lock();
  if (!instance) {
    return {};
  }
  std::vector<std::pair<vk::PhysicalDevice, vk::UniqueDevice>> devices =
      CreateVkDevices(instance->instance, instance->GetVersion(),
                      device_extensions, enabled_layers);

  std::vector<std::shared_ptr<Device>> ret;
  ret.reserve(devices.size());

  for (size_t i = 0; i < devices.size(); ++i) {
    ret.emplace_back(
        new Device(devices[i].first, std::move(devices[i].second), instance));
  }

  return ret;
}

Device::Device(const vk::PhysicalDevice& physical_device_,
               vk::UniqueDevice&& device_, std::weak_ptr<Instance> instance) {
  physical_device = physical_device_;
  device          = std::move(device_);
  instance_       = instance;

  std::shared_ptr<Instance> sp_instance = instance_.lock();

  if (sp_instance) {
    VmaVulkanFunctions vulkan_functions    = {};
    vulkan_functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkan_functions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.vulkanApiVersion =
        sp_instance->GetVersion();  // TODO(any) Is this OK?
    allocator_create_info.physicalDevice   = physical_device;
    allocator_create_info.device           = device.get();
    allocator_create_info.instance         = sp_instance->instance.get();
    allocator_create_info.pVulkanFunctions = &vulkan_functions;

    vma_allocator_.reset(new VmaAllocator);
    vmaCreateAllocator(&allocator_create_info, vma_allocator_.get());
  } else {
    // TODO(anyone): Handle error

    // TODO(any) delete
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size  = 512;
    buffer_create_info.usage =
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;  // buffer is used as a storage
                                             // buffer.
    //  buffer_create_info.sharingMode =
    //      VK_SHARING_MODE_EXCLUSIVE;  // buffer is exclusive to a single queue
    //                                  // family at a time.
    //                                  //

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage                   = VMA_MEMORY_USAGE_AUTO;

    buffers_[0] = {};
    vmaCreateBuffer(*vma_allocator_, &buffer_create_info,
                    &allocation_create_info, &(buffers_[0].first),
                    &(buffers_[0].second), nullptr);
  }
}

Device::~Device() {
  for (auto& [hadle, buffer_allocation] : buffers_) {
    vmaDestroyBuffer(*vma_allocator_, buffer_allocation.first,
                     buffer_allocation.second);
  }
  vmaDestroyAllocator(*vma_allocator_);
}

}  // namespace vulkan_hpp_test
