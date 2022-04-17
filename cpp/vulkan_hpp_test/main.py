import time

import numpy as np
import vulkan_hpp_test

# instance: vulkan_hpp_test.Instance = vulkan_hpp_test.Instance()


app: vulkan_hpp_test.App = vulkan_hpp_test.App()
print(app.get_num_devices())


buffer = app.create_buffer(0, 1 << 20)
time.sleep(1)
buffer = app.create_buffer(0, 1 << 20)
time.sleep(1)
buffer = app.create_buffer(0, 1 << 20)
time.sleep(1)

cpu_buffer = app.create_cpu_buffer(4 * 25000000)
cpu_buffer.from_numpy(np.arange(25000000).astype(np.float32))
np_array = cpu_buffer.to_numpy_f32()
print(np_array)

device_buffer = cpu_buffer.to_device_buffer(0)

time.sleep(3)

return_buffer = device_buffer.to_cpu_buffer()

print(return_buffer.to_numpy_f32())
