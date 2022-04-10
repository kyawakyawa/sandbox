import vulkan_hpp_test

# instance: vulkan_hpp_test.Instance = vulkan_hpp_test.Instance()


app: vulkan_hpp_test.App = vulkan_hpp_test.App()
print(app.get_num_devices())
