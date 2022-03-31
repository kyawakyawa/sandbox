/*
The MIT License (MIT)

Copyright (c) 2017 Eric Arnebäck

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <assert.h>
#include <string.h>
#include <vulkan/vulkan.h>

#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "lodepng.h"  //Used for png encoding.

const int WORKGROUP_SIZE = 32;  // Workgroup size in compute shader.

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                                               \
  {                                                                      \
    VkResult res = (f);                                                  \
    if (res != VK_SUCCESS) {                                             \
      printf("Fatal : VkResult is %d in %s at line %d\n", res, __FILE__, \
             __LINE__);                                                  \
      assert(res == VK_SUCCESS);                                         \
    }                                                                    \
  }

/*
The application launches a compute shader that renders the mandelbrot set,
by rendering it into a storage buffer.
The storage buffer is then read from the GPU, and saved as .png.
*/
class ComputeApplication {
private:
  // The pixels of the rendered mandelbrot set are in this format:
  struct Pixel {
    float r, g, b, a;
  };

  /*
  In order to use Vulkan, you must create an instance.
  */
  VkInstance instance;

  VkDebugReportCallbackEXT debugReportCallback;
  /*
  The physical device is some device on the system that supports usage of
  Vulkan. Often, it is simply a graphics card that supports Vulkan.
  */
  VkPhysicalDevice physical_device_;
  /*
  Then we have the logical device VkDevice, which basically allows
  us to interact with the physical device.
  */
  VkDevice device;

  // clang-format off
  const std::vector<const char*> device_extensions_ = {
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

  /*
  The pipeline specifies the pipeline that all graphics and compute commands
  pass though in Vulkan.

  We will be creating a simple compute pipeline in this application.
  */
  VkPipeline pipeline_;
  VkPipelineLayout pipeline_layout_;
  VkShaderModule compute_shader_module_;

  /*
  The command buffer is used to record commands, that will be submitted to a
  queue.

  To allocate such command buffers, we use a command pool.
  */
  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;

  /*

  Descriptors represent resources in shaders. They allow us to use things like
  uniform buffers, storage buffers and images in GLSL.

  A single descriptor represents a single resource, and several descriptors are
  organized into descriptor sets, which are basically just collections of
  descriptors.
  */
  VkDescriptorPool descriptor_pool_;
  VkDescriptorSet descriptor_set_;
  VkDescriptorSetLayout descriptor_set_layout_;

  /*
  buffer

  The memory that backs the buffer is bufferMemory.
  */
  VkBuffer src_buffer_, dst_buffer_;
  VkDeviceSize src_buffer_size_, dst_buffer_size_;
  VkDeviceMemory src_buffer_memory_, dst_buffer_memory_;

  uint32_t bufferSize;  // size of `buffer` in bytes.

  std::vector<const char*> enabledLayers;

  /*
  In order to execute commands on a device(GPU), the commands must be submitted
  to a queue. The commands are stored in a command buffer, and this command
  buffer is given to the queue.

  There will be different kinds of queues on the device. Not all queues support
  graphics operations, for instance. For this application, we at least want a
  queue that supports compute operations.
  */
  VkQueue queue;  // a queue supporting compute operations.

  /*
  Groups of queues that have the same capabilities(for instance, they all
  supports graphics and computer operations), are grouped into queue families.

  When submitting a command buffer, you must specify to which queue in the
  family you are submitting to. This variable keeps track of the index of that
  queue in its family.
  */
  uint32_t queueFamilyIndex;

  // other ////////////
  const std::string input_filepath_;
  const std::string output_filepath_;

  std::vector<unsigned char> input_img_buf_;
  uint32_t input_img_width_;
  uint32_t input_img_height_;

  struct MyPushConstant {
    uint32_t w;
    uint32_t h;
    float sigma;
  };

public:
  ComputeApplication() = delete;
  ComputeApplication(const std::string input_filepath,
                     const std::string output_filepath);
  void run() {
    loadSrcPng();

    glayscaleSrcImg();

    // Initialize vulkan:
    createInstance();
    findPhysicalDevice();
    createDevice();
    createBuffer();
    printf("Create Buffer.\n");
    createDescriptorSetLayout();
    printf("Create DescriptorSetLayout.\n");
    createDescriptorSet();
    printf("Create DescriptorSet.\n");
    createComputePipeline();
    printf("Create Pipeline.\n");
    createCommandBuffer();
    printf("Create Command Buffer\n");
    uploadSrcImgToDevice();
    printf("Upload source image to GPU\n");

    // Finally, run the recorded command buffer.
    runCommandBuffer();
    printf("Computation is finished\n");

    saveFilterdImage();
    printf("Save filtered image as [%s].\n", output_filepath_.c_str());

    // Clean up all vulkan resources.
    cleanup();
  }

  void loadSrcPng(void) {
    unsigned width, height;
    unsigned error =
        lodepng::decode(input_img_buf_, width, height, input_filepath_);
    if (error) {
      std::runtime_error("Faild to load image. (" + input_filepath_ + ")");
    }

#ifndef NDEBUG
    printf("Input image is loaded. (%s)\n", input_filepath_.c_str());
#endif
    input_img_width_  = width;
    input_img_height_ = height;
  }

  void glayscaleSrcImg(void) {
    uint32_t ch =
        input_img_buf_.size() / (input_img_width_ * input_img_height_);
#ifndef NDEBUG
    printf("number of channnel: %u\n", ch);
#endif
    if (ch == 1) {
      return;
    }
    if (ch == 2) {
      throw std::runtime_error("Image is broken.");
    }
    std::vector<unsigned char> tmp(input_img_buf_.begin(),
                                   input_img_buf_.end());
    input_img_buf_.resize(input_img_width_ * input_img_height_);
    for (int y = 0; y < input_img_height_; ++y) {
      for (int x = 0; x < input_img_width_; ++x) {
        float vf = 0.f;
        vf += 0.3f * float(tmp[y * input_img_width_ * ch + x * ch + 0]);
        vf += 0.59f * float(tmp[y * input_img_width_ * ch + x * ch + 1]);
        vf += 0.11f * float(tmp[y * input_img_width_ * ch + x * ch + 2]);
        input_img_buf_[y * input_img_width_ + x] =
            static_cast<unsigned char>(vf);
      }
    }
  }

  void saveFilterdImage() {
    std::vector<decltype(input_img_buf_)::value_type> tmp(input_img_width_ *
                                                          input_img_height_);

    void* mapped_memory = nullptr;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, dst_buffer_memory_, 0, dst_buffer_size_, 0,
                &mapped_memory);

    decltype(input_img_buf_)::value_type* pmapped_memory =
        reinterpret_cast<typename decltype(input_img_buf_)::value_type*>(
            mapped_memory);

    memcpy(tmp.data(), pmapped_memory,
           sizeof(decltype(tmp)::value_type) * tmp.size());

    vkUnmapMemory(device, dst_buffer_memory_);

    printf("Download dst image from GPU\n");

    std::vector<decltype(input_img_buf_)::value_type> output_img_buf(
        input_img_width_ * input_img_height_ * 4);
    for (int i = 0; i < input_img_width_ * input_img_height_; ++i) {
      decltype(input_img_buf_)::value_type v = tmp[i];
      output_img_buf[i * 4 + 0]              = v;
      output_img_buf[i * 4 + 1]              = v;
      output_img_buf[i * 4 + 2]              = v;
      output_img_buf[i * 4 + 3]              = 255;
    }

    // Now we save the acquired color data to a .png.
    unsigned error = lodepng::encode(output_filepath_.c_str(), output_img_buf,
                                     input_img_width_, input_img_height_);
    if (error) printf("encoder error %d: %s", error, lodepng_error_text(error));
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFn(
      VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
      uint64_t object, size_t location, int32_t messageCode,
      const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);

    return VK_FALSE;
  }

  void createInstance() {
    std::vector<const char*> enabledExtensions;

    /*
    By enabling validation layers, Vulkan will emit warnings if the API
    is used incorrectly. We shall enable the layer VK_LAYER_KHRONOS_validation,
    which is basically a collection of several useful validation layers.
    */
    if (enableValidationLayers) {
      /*
      We get all supported layers with vkEnumerateInstanceLayerProperties.
      */
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, NULL);

      std::vector<VkLayerProperties> layerProperties(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

      /*
      And then we simply check if VK_LAYER_KHRONOS_validation is among the
      supported layers.
      */
      bool foundLayer = false;
      for (VkLayerProperties prop : layerProperties) {
        if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
          foundLayer = true;
          break;
        }
      }

      if (!foundLayer) {
        throw std::runtime_error(
            "Layer VK_LAYER_KHRONOS_validation not supported\n");
      }
      enabledLayers.push_back(
          "VK_LAYER_KHRONOS_validation");  // Alright, we can use this layer.

      /*
      We need to enable an extension named VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
      in order to be able to print the warnings emitted by the validation layer.

      So again, we just check if the extension is among the supported
      extensions.
      */

      uint32_t extensionCount;

      vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
      std::vector<VkExtensionProperties> extensionProperties(extensionCount);
      vkEnumerateInstanceExtensionProperties(NULL, &extensionCount,
                                             extensionProperties.data());

      bool foundExtension = false;
      for (VkExtensionProperties prop : extensionProperties) {
        if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) ==
            0) {
          foundExtension = true;
          break;
        }
      }

      if (!foundExtension) {
        throw std::runtime_error(
            "Extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");
      }
      enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    /*
    Next, we actually create the instance.

    */

    /*
    Contains application info. This is actually not that important.
    The only real important field is apiVersion.
    */
    VkApplicationInfo applicationInfo  = {};
    applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName   = "Hello world app";
    applicationInfo.applicationVersion = 0;
    applicationInfo.pEngineName        = "awesomeengine";
    applicationInfo.engineVersion      = 0;
    applicationInfo.apiVersion =
        VK_API_VERSION_1_1;  // VkPhysicalDeviceFeatures2の利用のため

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags                = 0;
    createInfo.pApplicationInfo     = &applicationInfo;

    // Give our desired layers and extensions to vulkan.
    createInfo.enabledLayerCount       = enabledLayers.size();
    createInfo.ppEnabledLayerNames     = enabledLayers.data();
    createInfo.enabledExtensionCount   = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    /*
    Actually create the instance.
    Having created the instance, we can actually start using vulkan.
    */
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, NULL, &instance));

    /*
    Register a callback function for the extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, so that warnings emitted from the
    validation layer are actually printed.
    */
    if (enableValidationLayers) {
      VkDebugReportCallbackCreateInfoEXT createInfo = {};
      createInfo.sType =
          VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
      createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                         VK_DEBUG_REPORT_WARNING_BIT_EXT |
                         VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
      createInfo.pfnCallback = &debugReportCallbackFn;

      // We have to explicitly load this function.
      auto vkCreateDebugReportCallbackEXT =
          (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
              instance, "vkCreateDebugReportCallbackEXT");
      if (vkCreateDebugReportCallbackEXT == nullptr) {
        throw std::runtime_error(
            "Could not load vkCreateDebugReportCallbackEXT");
      }

      // Create and register callback.
      VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(
          instance, &createInfo, NULL, &debugReportCallback));
    }
  }

  void findPhysicalDevice() {
    /*
    In this function, we find a physical device that can be used with Vulkan.
    */

    /*
    So, first we will list all physical devices on the system with
    vkEnumeratePhysicalDevices .
    */
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
      throw std::runtime_error("could not find a device with vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    /*
    Next, we choose a device that can be used for our purposes.

    With VkPhysicalDeviceFeatures(), we can retrieve a fine-grained list of
    physical features supported by the device. However, in this demo, we are
    simply launching a simple compute shader, and there are no special physical
    features demanded for this task.

    With VkPhysicalDeviceProperties(), we can obtain a list of physical device
    properties. Most importantly, we obtain a list of physical device
    limitations. For this application, we launch a compute shader, and the
    maximum size of the workgroups and total number of compute shader
    invocations is limited by the physical device, and we should ensure that the
    limitations named maxComputeWorkGroupCount, maxComputeWorkGroupInvocations
    and maxComputeWorkGroupSize are not exceeded by our application.  Moreover,
    we are using a storage buffer in the compute shader, and we should ensure
    that it is not larger than the device can handle, by checking the limitation
    maxStorageBufferRange.

    However, in our application, the workgroup size and total number of shader
    invocations is relatively small, and the storage buffer is not that large,
    and thus a vast majority of devices will be able to handle it. This can be
    verified by looking at some devices at_ http://vulkan.gpuinfo.org/

    Therefore, to keep things simple and clean, we will not perform any such
    checks here, and just pick the first physical device in the list. But in a
    real and serious application, those limitations should certainly be taken
    into account.

    */

    bool found_device = false;
    for (VkPhysicalDevice device : devices) {
      if (IsDeviceSuitable(
              device)) {  // 一番最初に条件を満たすデバイスを見つけたらそれを選択
        physical_device_ = device;
        found_device     = true;
        break;
      }
    }
    if (!found_device) {
      throw std::runtime_error("Physical Device not found\n");
    }
  }

  bool IsDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;

    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    // グラフィックカードか？
    const bool condition0 =
        /* グラフィックカード */
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
        /* 統合GPU */
        device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

#ifndef NDEBUG
    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      printf("グラフィックカードが検出されました\n");
    } else if (device_properties.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      printf("統合GPUが検出されました\n");
    }
#endif

    // 拡張機能に対応しているか
    const bool extensions_supported = CheckDeviceExtensionSupport(device);

    return condition0 && extensions_supported;
  }
  bool CheckDeviceExtensionSupport(const VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                         available_extensions.data());

    std::unordered_set<std::string> required_extensions(
        device_extensions_.begin(), device_extensions_.end());

#ifndef NDEBUG
    printf("----- Available Extensions -----\n");
    for (const auto& extension : available_extensions) {
      printf("     -- %s\n", extension.extensionName);
    }
#endif

    for (const auto& extension : available_extensions) {
      required_extensions.erase(extension.extensionName);
    }

    for (const auto& extension_name : required_extensions) {
      fprintf(stderr, "not find Extension : %s\n", extension_name.c_str());
    }

    return required_extensions.empty();
  }

  // Returns the index of a queue family that supports compute operations.
  uint32_t getComputeQueueFamilyIndex() {
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_,
                                             &queueFamilyCount, NULL);

    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device_, &queueFamilyCount, queueFamilies.data());

    // Now find a family that supports compute.
    uint32_t i = 0;
    for (; i < queueFamilies.size(); ++i) {
      VkQueueFamilyProperties props = queueFamilies[i];

      if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
        // found a queue with compute. We're done!
        break;
      }
    }

    if (i == queueFamilies.size()) {
      throw std::runtime_error(
          "could not find a queue family that supports operations");
    }

    return i;
  }

  void createDevice() {
    /*
    We create the logical device in this function.
    */

    /*
    When creating the device, we also specify what queues it has.
    */
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyIndex = getComputeQueueFamilyIndex();  // find queue family with
                                                      // compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount =
        1;  // create one queue in this family. We don't need more.
    float queuePriorities =
        1.0;  // we only have one queue, so this is not that imporant.
    queueCreateInfo.pQueuePriorities = &queuePriorities;

    // Device Feature を指定する
    //
    // 8bitのshader interfaceに対応するために
    // VkPhysicalDevice8BitStorageFeaturesを使う
    VkPhysicalDevice8BitStorageFeatures device_8bit_storage_features = {};
    device_8bit_storage_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;

    // さらに、SPIR-Vで OpCapability Int8 に対応するために
    // VkPhysicalDeviceShaderFloat16Int8Features を使う
    VkPhysicalDeviceShaderFloat16Int8Features
        device_shader_float16_int8_features = {};
    device_shader_float16_int8_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;

    // VkPhysicalDevice8BitStorageFeaturesとVkPhysicalDeviceShaderFloat16Int8Features
    // を調べるためにVkPhysicalDeviceFeatures2を作成
    // VkPhysicalDeviceFeatures2とVkPhysicalDeviceFeaturesの違いpNextが使えるか使えないか
    // 既存のVkPhysicalDeviceFeaturesはメンバにある
    // pNextにVkPhysicalDevice8BitStorageFeaturesを指定
    // さらにそのpNextにVkPhysicalDeviceShaderFloat16Int8Featuresを指定
    VkPhysicalDeviceFeatures2 device_features2 = {};
    device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    device_features2.pNext =
        reinterpret_cast<void*>(&device_8bit_storage_features);
    device_8bit_storage_features.pNext =
        reinterpret_cast<void*>(&(device_shader_float16_int8_features));

    // Featureの情報を得る
    vkGetPhysicalDeviceFeatures2(physical_device_, &device_features2);

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

    /*
    Now we create the logical device. The logical device allows us to interact
    with the physical device.
    */
    VkDeviceCreateInfo device_create_info = {};

    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.enabledLayerCount =
        enabledLayers
            .size();  // need to specify validation layers here as well.
    device_create_info.ppEnabledLayerNames = enabledLayers.data();
    device_create_info.pQueueCreateInfos =
        &queueCreateInfo;  // when creating the logical device, we also specify
                           // what queues it has.
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures     = &(device_features2.features);

    device_create_info.pNext = &(
        device_8bit_storage_features);  // VkPhysicalDevice8BitStorageFeaturesはpNextに指定する
                                        // 先程、VkPhysicalDevice8BitStorageFeaturesの
                                        // pNextにVkPhysicalDeviceShaderFloat16Int8Featuresも渡したので、
                                        // そちらもvkCreateDeviceに渡される

    // 拡張を指定
    device_create_info.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions_.size());
    device_create_info.ppEnabledExtensionNames = device_extensions_.data();

    VK_CHECK_RESULT(vkCreateDevice(physical_device_, &device_create_info, NULL,
                                   &device));  // create logical device.

    // Get a handle to the only member of the queue family.
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
  }

  // find memory type with desired properties.
  uint32_t findMemoryType(uint32_t memoryTypeBits,
                          VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memoryProperties);

    /*
    How does this search work?
    See the documentation of VkPhysicalDeviceMemoryProperties for a detailed
    description.
    */
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
      if ((memoryTypeBits & (1 << i)) &&
          ((memoryProperties.memoryTypes[i].propertyFlags & properties) ==
           properties))
        return i;
    }
    return -1;
  }

  void createBuffer() {
    /*
    We will now create a buffer.
    */

    /////////////////// src(input) buffer ////////////////////////////////////
    VkBufferCreateInfo src_buffer_create_info = {};
    src_buffer_size_ =
        sizeof(decltype(input_img_buf_)::value_type) * input_img_buf_.size();
    src_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    src_buffer_create_info.size  = src_buffer_size_;
    src_buffer_create_info.usage =
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;  // buffer is used as a storage
                                             // buffer.
    src_buffer_create_info.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;  // buffer is exclusive to a single queue
                                    // family at a time.
                                    //
    VK_CHECK_RESULT(vkCreateBuffer(device, &src_buffer_create_info, NULL,
                                   &src_buffer_));  // create buffer.

    //バッファ自身でメモリを確保しないので、手動で確保する必要がある

    // まずバッファが要求するメモリ要件を調べる
    VkMemoryRequirements src_memory_requirements;
    vkGetBufferMemoryRequirements(
        /*VkDevice device                          =*/device,
        /*VkBuffer buffer                          =*/src_buffer_,
        /*VkMemoryRequirements *pMemoryRequirements=*/&src_memory_requirements);

    // バッファのためメモリ確保のためにメモリ要件を用いる
    VkMemoryAllocateInfo src_allocate_info = {};
    src_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    src_allocate_info.allocationSize =
        src_memory_requirements.size;  // 要求されたメモリを指定する

    // 確保できるメモリにはいくつか種類があり、選択する必要がある
    //
    // 1) メモリ要件を満たすもの(memory_requirements.memoryTypeBit)
    // 2) このプログラムの用途を満たすもの
    //
    //  (vkMapMemoryを使ってCPUからGPUにバッファメモリを読み込めるようするため、
    //   VK_MEMORY_PROPERTY_HOST_VISIBLE_BITを設定する)
    src_allocate_info.memoryTypeIndex =
        findMemoryType(src_memory_requirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // デバイス上のメモリを確保する
    VK_CHECK_RESULT(vkAllocateMemory(
        /*VkDevice device                          =*/device,
        /*const VkMemoryAllocateInfo *pAllocateInfo=*/&src_allocate_info,
        /*const VkAllocationCallbacks *pAllocator  =*/
        nullptr,  // ここにアロケータを渡すとCPUメモリも確保できる
        /*VkDeviceMemory *pMemory                  =*/&src_buffer_memory_));

    // 確保したメモリとバッファを関連付ける
    // これによって実際のメモリによってバッファが使えるようになる
    VK_CHECK_RESULT(
        vkBindBufferMemory(/*VkDevice device          =*/device,
                           /*VkBuffer buffer          =*/src_buffer_,
                           /*VkDeviceMemory memory    =*/src_buffer_memory_,
                           /*VkDeviceSize memoryOffset=*/0))
    /////////////////// dst(output) buffer ///////////////////////////////////
    VkBufferCreateInfo dst_buffer_create_info = {};
    dst_buffer_size_ =
        sizeof(decltype(input_img_buf_)::value_type) *
        input_img_buf_.size();  // Output is the same size as input.

    dst_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    dst_buffer_create_info.size  = dst_buffer_size_,
    dst_buffer_create_info.usage =
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;  // buffer is used as a storage
                                             // buffer.
    dst_buffer_create_info.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;  // buffer is exclusive to a single queue
                                    // family at a time.

    VK_CHECK_RESULT(vkCreateBuffer(device, &dst_buffer_create_info, NULL,
                                   &dst_buffer_));  // create buffer.

    //バッファ自身でメモリを確保しないので、手動で確保する必要がある

    // まずバッファが要求するメモリ要件を調べる
    VkMemoryRequirements dst_memory_requirements;
    vkGetBufferMemoryRequirements(
        /*VkDevice device                          =*/device,
        /*VkBuffer buffer                          =*/dst_buffer_,
        /*VkMemoryRequirements *pMemoryRequirements=*/&dst_memory_requirements);

    // バッファのためメモリ確保のためにメモリ要件を用いる
    VkMemoryAllocateInfo dst_allocate_info = {};
    dst_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    dst_allocate_info.allocationSize =
        dst_memory_requirements.size;  // 要求されたメモリを指定する

    // 確保できるメモリにはいくつか種類があり、選択する必要がある
    //
    // 1) メモリ要件を満たすもの(memory_requirements.memoryTypeBit)
    // 2) このプログラムの用途を満たすもの
    //
    //  (vkMapMemoryを使ってGPUからCPUにバッファメモリを読み込めるようするため、
    //   VK_MEMORY_PROPERTY_HOST_VISIBLE_BITを設定する)
    //
    // また、VK_MEMORY_PROPERTY_HOST_COHERENT_BITを設定しておくと、
    // デバイス（GPU）によって書き込まれたメモリは、余分なフラッシュコマンドを呼び出さなくても、
    // ホスト（CPU）から簡単に見えるようになる
    // 従って、便利なので、このフラグを設定する。
    dst_allocate_info.memoryTypeIndex =
        findMemoryType(dst_memory_requirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // デバイス上のメモリを確保する
    VK_CHECK_RESULT(vkAllocateMemory(
        /*VkDevice device                          =*/device,
        /*const VkMemoryAllocateInfo *pAllocateInfo=*/&dst_allocate_info,
        /*const VkAllocationCallbacks *pAllocator  =*/
        nullptr,  // ここにアロケータを渡すとCPUメモリも確保できる
        /*VkDeviceMemory *pMemory                  =*/&dst_buffer_memory_));

    VK_CHECK_RESULT(
        vkBindBufferMemory(/*VkDevice device          =*/device,
                           /*VkBuffer buffer          =*/dst_buffer_,
                           /*VkDeviceMemory memory    =*/dst_buffer_memory_,
                           /*VkDeviceSize memoryOffset=*/0))
  }

  void createDescriptorSetLayout() {
    // この関数で  descriptor setのレイアウトを指定する
    // これは descriptorとシェーダーのリソースを結びつけることを可能にする

    // binding point 0と1に紐付けるVK_DESCRIPTOR_TYPE_STORAGE_BUFFERを指定する
    // これはcompute shader の
    //
    // layout(set=*, binding = 0) buffer buf
    //
    // に結び付けられる
    //
    // dst -> 0
    // src -> 1

    std::vector<VkDescriptorSetLayoutBinding>
        bindings;  // ここにVkDescriptorSetLayoutBindingを入れてVkDescriptorSetLayoutCreateInfo
                   // に渡す

    VkDescriptorSetLayoutBinding dst_descriptor_set_layout_binding = {};
    dst_descriptor_set_layout_binding.binding = 0;  // binding = 0
    dst_descriptor_set_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dst_descriptor_set_layout_binding.descriptorCount = 1;
    dst_descriptor_set_layout_binding.stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT;  // Computeシェーダーで利用できるようにする
    bindings.emplace_back(dst_descriptor_set_layout_binding);

    VkDescriptorSetLayoutBinding src_descriptor_set_layout_binding = {};
    src_descriptor_set_layout_binding.binding = 1;  // binding = 1
    src_descriptor_set_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    src_descriptor_set_layout_binding.descriptorCount = 1;
    src_descriptor_set_layout_binding.stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT;  // Computeシェーダーで利用できるようにする
    bindings.emplace_back(src_descriptor_set_layout_binding);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount =
        bindings.size();  // VkDescriptorSetLayoutBindingの数
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();

    // Descriptor setを作成する
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(
        device, &descriptorSetLayoutCreateInfo, NULL, &descriptor_set_layout_));
  }

  void createDescriptorSet() {
    // この関数でdescriptor setを確保する
    // まずdescriptor poolを作る
    //
    // bindingするbufferの数以上のpool sizeを作る
    std::vector<VkDescriptorPoolSize> pool_sizes;

    // dst
    pool_sizes.emplace_back();  // 追加
    VkDescriptorPoolSize& dst_descriptor_pool_size =
        pool_sizes.back();  // 追加したものへの参照
    dst_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dst_descriptor_pool_size.descriptorCount = 1;

    // src
    pool_sizes.emplace_back();  //追加
    VkDescriptorPoolSize& src_descriptor_pool_size =
        pool_sizes.back();  // 追加したものへの参照
    src_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    src_descriptor_pool_size.descriptorCount = 1;

    // Pool作成の情報
    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
    descriptor_pool_create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.maxSets =
        1;  //  // poolから1つのdescriptor setを確保すれば良い
    descriptor_pool_create_info.poolSizeCount = pool_sizes.size();
    descriptor_pool_create_info.pPoolSizes    = pool_sizes.data();

    // Descriptor poolを作成する
    VK_CHECK_RESULT(vkCreateDescriptorPool(
        /*VkDevice device                              =*/device,
        /*const VkDescriptorPoolCreateInfo *pCreateInfo=*/
        &descriptor_pool_create_info,
        /*const VkAllocationCallbacks *pAllocator      =*/nullptr,
        /*VkDescriptorPool *pDescriptorPool            =*/&descriptor_pool_));

    // Poolが確保されたら、descriptor setをallocateする
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
    descriptor_set_allocate_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool =
        descriptor_pool_;  // どのプールから確保するか
    descriptor_set_allocate_info.descriptorSetCount =
        1;  // 一つのdescriptor setを確保する
    descriptor_set_allocate_info.pSetLayouts =
        &descriptor_set_layout_;  // レイアウトを指定

    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(
        /*VkDevice device                                 =*/device,
        /*const VkDescriptorSetAllocateInfo *pAllocateInfo=*/
        &descriptor_set_allocate_info,
        /*VkDescriptorSet *pDescriptorSets                =*/&descriptor_set_));

    //  descriptorとストレージバッファを紐付ける
    //  descriptorのsetを更新するために vkUpdateDescriptorSets()関数を用いる

    // descriptor setに書き込むものをセットしていく
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;

    // descriptorに結びつけるバッファを指定する(dst)
    write_descriptor_sets.emplace_back();             //追加
    VkWriteDescriptorSet& dst_write_descriptor_set =  //追加したものへの参照
        write_descriptor_sets.back();

    VkDescriptorBufferInfo dst_descriptor_buffer_info = {};
    dst_descriptor_buffer_info.buffer                 = dst_buffer_;
    dst_descriptor_buffer_info.offset = 0;  // オフセットはなし
    dst_descriptor_buffer_info.range  = dst_buffer_size_;

    dst_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    dst_write_descriptor_set.dstSet =
        descriptor_set_;  //  書き込むdescriptor set
    dst_write_descriptor_set.dstBinding      = 0;  // binding=0に紐付ける
    dst_write_descriptor_set.descriptorCount = 1;
    dst_write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;  // storage buffer.
    dst_write_descriptor_set.pBufferInfo = &dst_descriptor_buffer_info;

    // descriptorに結びつけるバッファを指定する(src)
    write_descriptor_sets.emplace_back();             //追加
    VkWriteDescriptorSet& src_write_descriptor_set =  // 追加したものへの参照
        write_descriptor_sets.back();

    VkDescriptorBufferInfo src_descriptor_buffer_info = {};
    src_descriptor_buffer_info.buffer                 = src_buffer_;
    src_descriptor_buffer_info.offset = 0;  // オフセットはなし
    src_descriptor_buffer_info.range  = src_buffer_size_;

    src_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    src_write_descriptor_set.dstSet =
        descriptor_set_;  //  書き込むdescriptor set
    src_write_descriptor_set.dstBinding      = 1;  // binding=1に紐付ける
    src_write_descriptor_set.descriptorCount = 1;
    src_write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;  // storage buffer.
    src_write_descriptor_set.pBufferInfo = &src_descriptor_buffer_info;

    // descriptor setの更新を行う
    vkUpdateDescriptorSets(
        /*VkDevice device                              =*/device,
        /*uint32_t descriptorWriteCount                =*/
        write_descriptor_sets.size(),
        /*const VkWriteDescriptorSet *pDescriptorWrites=*/
        write_descriptor_sets.data(),
        /*uint32_t descriptorCopyCount                 =*/0,  // コピーはしない
        /*const VkCopyDescriptorSet *pDescriptorCopies =*/nullptr);
  }

  // Read file into array of bytes, and cast to uint32_t*, then return.
  // The data has been padded, so that it fits into an array uint32_t.
  uint32_t* readFile(uint32_t& length, const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
      printf("Could not find or open file: %s\n", filename);
    }

    // get file size.
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    long filesizepadded = long(ceil(filesize / 4.0)) * 4;

    // read file contents.
    char* str = new char[filesizepadded];
    fread(str, filesize, sizeof(char), fp);
    fclose(fp);

    // data padding.
    for (int i = filesize; i < filesizepadded; i++) {
      str[i] = 0;
    }

    length = filesizepadded;
    return (uint32_t*)str;
  }

  void createComputePipeline() {
    // この関数でcompute pipeline を作る

    // まずSPIR-Vからshader modelを作成する
    uint32_t file_length;
    uint32_t* code = readFile(file_length, "./spirv/c/gaussian_filter.spv");

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pCode    = code;
    create_info.codeSize = file_length;
    VK_CHECK_RESULT(vkCreateShaderModule(device, &create_info, nullptr,
                                         &compute_shader_module_));
    delete[] code;

    // 次にcomputeパイプラインを作る
    // graphicsパイプラインよりcomputeパイプラインはシンプルである
    // compute shaderは一つのステージのみである
    //
    // まずcomputeシェーダーのステージを指定する
    // エントリーポイントは gaussian_filter7x7_glayscale
    VkPipelineShaderStageCreateInfo shader_stage_create_info = {};
    shader_stage_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage_create_info.module = compute_shader_module_;
    shader_stage_create_info.pName  = "gaussian_filter7x7_glayscale";

    // PipelineLayoutはPipelineがdescriptor setにアクセスすることを可能にする
    // よって先に作ったdescriptor set layoutを指定する
    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts    = &descriptor_set_layout_;

    // 更に __globalでない引数は push constantsを用いて与えるのでその設定を行う

    // PipelineLayoutで使用されるpush constantsのrangeを定義。
    // 仕様では128 bitまでの大きさに対応となっているので、
    // それより大きい場合はBufferを使うべき
    //
    // 具体的な値を指定する必要はない(Command Bufferの作成時に行う)
    MyPushConstant my_push_constant = {};

    VkPushConstantRange push_constant;
    push_constant.offset = 0;  // オフセット
    push_constant.size   = sizeof(MyPushConstant);
    push_constant.stageFlags =
        VK_SHADER_STAGE_COMPUTE_BIT;  // Compute shaderで利用

    pipeline_layout_create_info.pPushConstantRanges    = &push_constant;
    pipeline_layout_create_info.pushConstantRangeCount = 1;

    // PipelineLayoutを作成する
    VK_CHECK_RESULT(vkCreatePipelineLayout(
        /*VkDevice device                              =*/device,
        /*const VkPipelineLayoutCreateInfo *pCreateInfo=*/
        &pipeline_layout_create_info,
        /*const VkAllocationCallbacks *pAllocator      =*/nullptr,
        /*VkPipelineLayout *pPipelineLayout            =*/&pipeline_layout_));

    VkComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.stage = shader_stage_create_info;
    pipeline_create_info.layout = pipeline_layout_;

    // 最後にcomputeパイプラインを作成する
    VK_CHECK_RESULT(vkCreateComputePipelines(
        /*VkDevice device                                */ device,
        /*VkPipelineCache pipelineCache                  */ VK_NULL_HANDLE,
        /*uint32_t createInfoCount                       */ 1,
        /*const VkComputePipelineCreateInfo *pCreateInfos*/
        &pipeline_create_info,
        /*const VkAllocationCallbacks *pAllocator        */ nullptr,
        /*VkPipeline *pPipelines                         */ &pipeline_));
  }

  void createCommandBuffer() {
    /*
    We are getting closer to the end. In order to send commands to the
    device(GPU), we must first record commands into a command buffer. To
    allocate a command buffer, we must first create a command pool. So let us do
    that.
    */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    // the queue family of this command pool. All command buffers allocated from
    // this command pool, must be submitted to queues of this family ONLY.
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL,
                                        &commandPool));

    /*
    Now allocate a command buffer from the command pool.
    */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool =
        commandPool;  // specify the command pool to allocate from.
    // if the command buffer is primary, it can be directly submitted to queues.
    // A secondary buffer has to be called from some primary command buffer, and
    // cannot be directly submitted to a queue. To keep things simple, we use a
    // primary command buffer.
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount =
        1;  // allocate a single command buffer.
    VK_CHECK_RESULT(
        vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                                 &commandBuffer));  // allocate command buffer.

    /*
    Now we shall start recording commands into the newly allocated command
    buffer.
    */
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // the buffer is only
                                                      // submitted and used once
                                                      // in this application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(
        commandBuffer, &beginInfo));  // start recording commands.

    /*
    We need to bind a pipeline, AND a descriptor set before we dispatch.

    The validation layer will NOT give warnings if you forget these, so be very
    careful not to forget them.
    */
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline_layout_, 0, 1, &descriptor_set_, 0, NULL);

    MyPushConstant my_push_constant;
    my_push_constant.w     = input_img_width_;
    my_push_constant.h     = input_img_height_;
    my_push_constant.sigma = 10.0f;

    // Push Constantの値をセットするコマンドをBufferに渡す
    vkCmdPushConstants(commandBuffer, pipeline_layout_,
                       VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(MyPushConstant),
                       &my_push_constant);

    /*
    Calling vkCmdDispatch basically starts the compute pipeline, and executes
    the compute shader. The number of workgroups is specified in the arguments.
    If you are already familiar with compute shaders from OpenGL, this should be
    nothing new to you.
    */
    vkCmdDispatch(commandBuffer,
                  (uint32_t)ceil(input_img_width_ / float(WORKGROUP_SIZE)),
                  (uint32_t)ceil(input_img_height_ / float(WORKGROUP_SIZE)), 1);

    VK_CHECK_RESULT(
        vkEndCommandBuffer(commandBuffer));  // end recording commands.
  }

  void uploadSrcImgToDevice(void) {
    void* mapped_memory = nullptr;
    vkMapMemory(device, src_buffer_memory_, 0, src_buffer_size_, 0,
                &mapped_memory);
    decltype(input_img_buf_)::value_type* pmapped_memory =
        reinterpret_cast<typename decltype(input_img_buf_)::value_type*>(
            mapped_memory);

    for (size_t i = 0; i < input_img_buf_.size(); ++i) {
      pmapped_memory[i] = input_img_buf_[i];
    }
    vkUnmapMemory(device, src_buffer_memory_);
  }

  void runCommandBuffer() {
    /*
    Now we shall finally submit the recorded command buffer to a queue.
    */

    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;  // submit a single command buffer
    submitInfo.pCommandBuffers =
        &commandBuffer;  // the command buffer to submit.

    /*
      We create a fence.
    */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags             = 0;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));

    /*
    We submit the command buffer on the queue, at the same time giving a fence.
    */
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
    /*
    The command will not have finished executing until the fence is signalled.
    So we wait here.
    We will directly after this read our buffer from the GPU,
    and we will not be sure that the command has finished executing unless we
    wait for the fence. Hence, we use a fence here.
    */
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

    vkDestroyFence(device, fence, NULL);
  }

  void cleanup() {
    /*
    Clean up all Vulkan Resources.
    */

    if (enableValidationLayers) {
      // destroy callback.
      auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
          instance, "vkDestroyDebugReportCallbackEXT");
      if (func == nullptr) {
        throw std::runtime_error(
            "Could not load vkDestroyDebugReportCallbackEXT");
      }
      func(instance, debugReportCallback, NULL);
    }

    // Buffer Memory
    // // src buffer
    vkFreeMemory(device, src_buffer_memory_, nullptr);
    vkDestroyBuffer(device, src_buffer_, nullptr);
    // // dst buffer
    vkFreeMemory(device, dst_buffer_memory_, nullptr);
    vkDestroyBuffer(device, dst_buffer_, nullptr);

    // shader module
    vkDestroyShaderModule(device, compute_shader_module_, NULL);

    // descriptor
    vkDestroyDescriptorPool(device, descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout_, nullptr);

    // pipeline
    vkDestroyPipelineLayout(device, pipeline_layout_, NULL);
    vkDestroyPipeline(device, pipeline_, NULL);

    // command pool
    vkDestroyCommandPool(device, commandPool, NULL);
#if 0
    vkDestroyShaderModule(device, computeShaderModule, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout_, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);
#endif
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
  }
};

ComputeApplication::ComputeApplication(const std::string input_filepath,
                                       const std::string output_filepath)
    : input_filepath_(input_filepath), output_filepath_(output_filepath) {}

int main() {
  const std::string input_filepath  = "src.png";
  const std::string output_filepath = "dst.png";
  ComputeApplication app(input_filepath, output_filepath);

  try {
    app.run();
  } catch (const std::runtime_error& e) {
    printf("%s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
