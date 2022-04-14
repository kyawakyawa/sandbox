import time

import vulkan_hpp_test

# instance: vulkan_hpp_test.Instance = vulkan_hpp_test.Instance()


app: vulkan_hpp_test.App = vulkan_hpp_test.App()
print(app.get_num_devices())


buffer = app.create_buffer(0, 1 << 20)
time.sleep(2)
buffer = app.create_buffer(0, 1 << 20)
time.sleep(2)
buffer = app.create_buffer(0, 1 << 20)
time.sleep(2)
