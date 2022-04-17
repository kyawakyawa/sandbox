#include "app.h"
#include "buffer.h"
#include "device.h"
#include "instance.h"
// pybind11
#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(vulkan_hpp_test, m) {
  m.doc() = "vulkan_hpp_test";

  // py::class_<vulkan_hpp_test::Instance> instance(m, "Instance");

  // instance.def(py::init<>());

  py::class_<vulkan_hpp_test::App> app(m, "App");
  py::class_<vulkan_hpp_test::Buffer, std::shared_ptr<vulkan_hpp_test::Buffer>>
      buffer(m, "Buffer");
  py::class_<vulkan_hpp_test::CpuBuffer,
             std::shared_ptr<vulkan_hpp_test::CpuBuffer>>
      cpu_buffer(m, "CpuBuffer");

  using namespace pybind11::literals;
  app.def(py::init<>())
      .def("get_num_devices", &vulkan_hpp_test::App::GetNumDevices,
           "A function that get number of devices")
      .def("create_buffer", &vulkan_hpp_test::App::CreateBuffer, "device_id"_a,
           "size_byte"_a, "A function that create buffer on device")
      .def("create_cpu_buffer", &vulkan_hpp_test::App::CreateCpuBuffer,
           "size_byte"_a, "A function that create cpu buffer on device");

  buffer.def(py::init<>())
      .def("to_cpu_buffer", &vulkan_hpp_test::Buffer::ToCpuBuffer,
           "A function this copy device buffer to cpu buffer");

  cpu_buffer.def(py::init())
      .def("from_numpy", &vulkan_hpp_test::CpuBuffer::FromNumpy, "a"_a,
           "A function that receive numpy array")
      .def("to_numpy_f32", &vulkan_hpp_test::CpuBuffer::ToNumpyf32,
           "A function that create numpy array.")
      .def("to_device_buffer", &vulkan_hpp_test::CpuBuffer::ToDeviceBuffer,
           "device_id"_a, "A function that create device buffer.");
}
